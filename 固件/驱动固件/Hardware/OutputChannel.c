#include "cms8s6990.h"
#include "PinDefs.h"
#include "GPIO.h"
#include "PWMCfg.h"
#include "delay.h"
#include "SpecialMode.h"
#include "TempControl.h"
#include "OutputChannel.h"
#include "ADCCfg.h"
#include "SelfTest.h"

//内部SFR
sbit AUXEN=AUXENIOP^AUXENIOx; //辅助6V DCDC使能
sbit PWMDACEN=PWMDACENIOP^PWMDACENIOx; //PWMDAC使能
sbit SYSHBLED=SYSHBLEDIOP^SYSHBLEDIOx; //心跳LED
sbit BOOSTRUN=BOOSTRUNIOP^BOOSTRUNIOx; //LTC3787的EN
sbit LEDMOS=LEDNegMOSIOP^LEDNegMOSIOx; //LEDMOSFET

//外部电流配置参考
xdata volatile int Current; //目标电流(mA)
xdata int CurrentBuf; //存储当前已经上传的电流值 
bit IsCurrentRampUp;  //电流正在上升过程中的标记位（用于和MPPT试探联动）

//内部变量
static bit IsEnableSlowILEDRamp; //标志位，是否启用慢速电流斜率控制
static xdata char PreChargeFSMTimer; //预充电状态机计时器
static xdata OutChFSMStateDef OutputFSMState; //输出控制状态机
static xdata char HBTimer; //心跳计时器

/*********************************************************************************************************************
输出通道控制器所使用的内部函数，仅能在该文件内调用。
*********************************************************************************************************************/

//内部用于计算PWMDAC占空比的函数	
static float Duty_Calc(int CurrentInput)
	{
	float buf;
	//计算实际占空比
	buf=(float)CurrentInput*(float)MainChannelShuntmOhm; //输入传进来的电流(mA)并乘以检流电阻阻值(mR)得到检流电阻处的目标电压(uV)
	buf*=(float)0.0015; //uV转mV并根据1.5mA per LSB换算得到实际的电流值
	buf*=CurrentSenseOpAmpGain; //将检流电阻处的目标电压(mV)乘以检流放大器的增益得到运放端的整定值
	buf/=Data.MCUVDD*(float)1000; //计算出目标DAC输出电压和PWMDAC缓冲器供电电压(MCUVDD)之间的比值
	buf*=102; //转换为百分比(乘以102补偿掉系统的换算误差)
	//结果输出	
	return buf>100?100:buf;
	}
	
/*********************************************************************************************************************
输出通道控制器所使用的外部函数，可以在其他地方调用。
*********************************************************************************************************************/

//初始化函数
void OutputChannel_Init(void)
	{
	GPIOCfgDef OCInitCfg;
	//设置结构体
	OCInitCfg.Mode=GPIO_Out_PP;
  OCInitCfg.Slew=GPIO_Fast_Slew;		
	OCInitCfg.DRVCurrent=GPIO_High_Current; //推MOSFET,需要高上升斜率
	//初始化bit
  AUXEN=0;
	BOOSTRUN=0;
	PWMDACEN=0;
	SYSHBLED=0; //所有bit都为0
	//开始配置IO	
	GPIO_ConfigGPIOMode(PWMDACENIOG,GPIOMask(PWMDACENIOx),&OCInitCfg);	
	GPIO_ConfigGPIOMode(AUXENIOG,GPIOMask(AUXENIOx),&OCInitCfg);			
	GPIO_ConfigGPIOMode(BOOSTRUNIOG,GPIOMask(BOOSTRUNIOx),&OCInitCfg);		
	GPIO_ConfigGPIOMode(LEDNegMOSIOG,GPIOMask(LEDNegMOSIOx),&OCInitCfg);
  GPIO_ConfigGPIOMode(SYSHBLEDIOG,GPIOMask(SYSHBLEDIOx),&OCInitCfg);		
	//调用复位函数重置所有状态
  OutputChannel_DeInit();
	}

//内联函数，设置心跳LED
//void SetHBLEDState(bit State)
//	{
//	SYSHBLED=State;
//	}	
	
//预充电状态机计时器
void OCFSM_TIMHandler(void)
	{
	//心跳LED控制	
	if(CurrentMode->ModeIdx==Mode_Fault) //发生故障时HB快闪
		SYSHBLED=SYSHBLED?0:1; //翻转LED
	else if(GetIfOutputEnabled())//输出已启用，LED配置为1
		{
		//输出系统处于待机状态，输出LED慢闪
		if(OutputFSMState==OutCH_OutputIdle)
			{
			SYSHBLED=HBTimer==3?1:0; //待机状态下每隔半秒快闪一次
			if(HBTimer<4)HBTimer++;
			else HBTimer=0;
			}
		//其余状态下LED常亮
		else SYSHBLED=1;
		}			
	else //待机状态下慢闪
		{
	  if(HBTimer<4)HBTimer++;
	  else
			{
			SYSHBLED=SYSHBLED?0:1; //翻转LED
			HBTimer=0;
			}
		}
	//状态机计时器
	if(PreChargeFSMTimer>0)PreChargeFSMTimer--;
	}	
	
//输出通道复位
void OutputChannel_DeInit(void)
	{
	BOOSTRUN=0;
	AUXEN=0;
	LEDMOS=0;
	PWMDACEN=0;
	SYSHBLED=0; //所有bit都为0
	//系统上电时电流配置为0
	Current=0;
	CurrentBuf=0;
	IsCurrentRampUp=0;
	IsEnableSlowILEDRamp=0;
	//复位状态机
	HBTimer=0;
	OutputFSMState=OutCH_Standby;
	}	

//外部获取输出是否正常启用的函数
bit GetIfOutputEnabled(void)	
	{
	if(OutputFSMState==OutCH_OutputEnabled)return 1;
	if(OutputFSMState==OutCH_SubmitDuty)return 1;
	//否则返回0
	return 0;
	}

//获取系统是否在安全关机阶段
bit GetIfSystemInPOFFSeq(void)
	{
	//如果系统处在软关机的等待阶段则不允许关闭	
	if(OutputFSMState==OutCH_WaitVOUTDecay)return 1;
  if(OutputFSMState==OutCH_GracefulShut)return 1;		
	//系统已经关闭，返回0
	return 0;
	}	
	
//输出通道试运行
void OutputChannel_TestRun(void)
	{
	char retry=100;
	bit IsDCDCOV=0;
	//打开辅助电源和PWMDAC
	AUXEN=1;
	PWMDACEN=1;
	PWM_ForceEnableOut(1);
	//延时40mS后检测电压，如果电压大于8V则正常启动进行检测
	delay_ms(40);
	SystemTelemHandler();
	if(Data.RawBattVolt>8)BOOSTRUN=1; //令3787EN=1，启动输出
	else 
		{
		PWM_ForceEnableOut(0);
		OutputChannel_DeInit(); //关闭PWM输出并复位输出通道
		return;
		}
	//检查输出状态
	do
		{
		SystemTelemHandler();
		if(Data.OutputVoltage>16.5)IsDCDCOV=1; //标记出现过压
		else if(Data.OutputVoltage>14.0)
			{
			//检查通过，关闭DCDC
			BOOSTRUN=0;
			delay_ms(5);
			//复位PWMDAC
			PWMDACEN=0;
			PWM_ForceEnableOut(0);
			//关闭辅助电源
			AUXEN=0;	
			return;
			}
	  //检查失败
		delay_ms(5);
		retry--;
		}
	while(retry>0);
	//DCDC检查超时，报告错误
	ReportError(IsDCDCOV?Fault_DCDCPreChargeFailed:Fault_DCDCFailedToStart);
	}

//输出通道计算
void OutputChannel_Calc(void)
	{
	int TargetCurrent,ILIM;
	extern bit IsBatteryAlert;
	//读取目标电流
	TargetCurrent=Current;
	if(Current>0) //电流大于0说明是有效输出执行温控计算
		{
		//温控计算
		ILIM=ThermalILIMCalc();
		if(TargetCurrent>ILIM)TargetCurrent=ILIM; //温控反馈的运行电流超过目标值，进行限流
		}
	//进行输出通道状态机管理
	switch(OutputFSMState)
		{
		//输出通道故障	
		case OutCH_PreChargeFailed:
			 OutputChannel_DeInit(); //执行输出复位
		   break; 
		//输出通道待机状态
    case OutCH_Standby:
       //复位DCDC控制
		   BOOSTRUN=0;
			 AUXEN=0;
			 LEDMOS=0;
			 PWMDACEN=0;
	     //复位标记位
	     IsCurrentRampUp=0;
			 //复位PWMDAC
		   if(PreChargeDACDuty||PWMDuty>0)
				{
				PreChargeDACDuty=0;
				PWMDuty=0;
				IsNeedToUploadPWM=1;
				}
			 //如果电流发生变更则进入启动状态
			 if(TargetCurrent>0)OutputFSMState=OutCH_PWMDACPreCharge;
			 break;
		//启动步骤1，送出PWMDAC
		case OutCH_PWMDACPreCharge:
       //启动电流整定DAC
		   PWMDACEN=1;
		   //配置PWMDAC占空比
			 CurrentBuf=TargetCurrent>CalcIREFValue(1500)?CalcIREFValue(1500):TargetCurrent;
			 PWMDuty=Duty_Calc(CurrentBuf);  //配置流程是如果当前电流大于1.5A，则钳位到1.5A，然后用这个初值配置PWMDAC
       //启动CV限压环DAC
       PreChargeDACDuty=0x82A; //0x82A=87.128%=11.29-0.4815*14.4->(0.87128*5)
       //上传占空比并跳转到下一步(启动主DCDC辅助PSU)
			 IsNeedToUploadPWM=1;
		   OutputFSMState=OutCH_StartAUXPSU;
		   break;
		//启动步骤2，启动辅助PSU（给LTC3787供电的电源）
		case OutCH_StartAUXPSU:
			 //等待PWM输出
		   if(IsNeedToUploadPWM)break;
		   delay_ms(20); //延时20mS
		   //启动辅助电源并跳转到下个状态
			 AUXEN=1;
		   PreChargeFSMTimer=16; //设置计时器最多等待2秒
		   OutputFSMState=OutCH_EnableBoost;
		   break;
		//启动步骤3，启动主DCDC并检查输出是否正常
		case OutCH_EnableBoost:
			//令3787EN=1，主Boost开始输出然后检测电压状态
			BOOSTRUN=1;
		  if(Data.OutputVoltage>14.0)OutputFSMState=OutCH_ReleasePreCharge; //电压起来了，继续启动流程
		  //等待超时后报错
		  if(PreChargeFSMTimer>0)break;
		  ReportError(Fault_DCDCFailedToStart);
		  OutputFSMState=OutCH_PreChargeFailed;
			break;
		//启动步骤4，逐步下调预充PWMDAC抬升输出电压
		case OutCH_ReleasePreCharge:
			//接通LED负极FET，LED开始发光
			LEDMOS=1;
		  //如果电流为0则开始进入放电阶段
			if(TargetCurrent==0)OutputFSMState=OutCH_GracefulShut;
			//如果占空比调整到0了则进入提升输出电流到目标值的程序
		  if(!PreChargeDACDuty)
				{
				//如果预充完成之后，已应用的电流和目标值同步则直接跳转到正常输出状态
				OutputFSMState=(CurrentBuf==TargetCurrent)?OutCH_OutputEnabled:OutCH_SubmitDuty;
				break;
				}
		  //开始逐步下调预充占空比把输出电压调到额定值
		  if(IsNeedToUploadPWM)break; //上次调整还未完毕
			ILIM=TargetCurrent/25;
			if(ILIM>200)ILIM=200; //计算出每次PWMDAC递减的值
			PreChargeDACDuty-=ILIM+1;
		  if(PreChargeDACDuty<0)PreChargeDACDuty=0; //禁止占空比为负数
			IsNeedToUploadPWM=1;
		  break;
		//应用占空比
		case OutCH_SubmitDuty:
			if(IsNeedToUploadPWM)break; //PWM正在应用中，等待
		  if(TargetCurrent==0)OutputFSMState=OutCH_GracefulShut; //系统电流配置为0，说明需要结束运行，跳转到待机
			//保护LED的电流斜率限制器
			if(TargetCurrent-CurrentBuf>CalcIREFValue(6000))IsEnableSlowILEDRamp=1; //监测到非常大的电流瞬态，避免冲爆灯珠采用软起
			if(!SysMode&&IsEnableSlowILEDRamp)
				{
				switch(CurrentMode->ModeIdx)
					{
					case Mode_Turbo:CurrentBuf+=IsInputLimited?0:10;break;  //极亮MPPT系统，配合输入告警监测使用
					case Mode_Beacon:CurrentBuf+=5000;break;
					case Mode_Strobe:CurrentBuf+=1500;break;
					case Mode_SOS:CurrentBuf+=500;break;
					default:CurrentBuf+=15;
					}
				if(CurrentBuf>=TargetCurrent)
					{
					IsEnableSlowILEDRamp=0;
					CurrentBuf=TargetCurrent; //限幅，不允许目标电流大于允许值
					}
				}
			else CurrentBuf=TargetCurrent; //直接同步		
		  //更新占空比
			IsNeedToUploadPWM=1;
			PWMDuty=Duty_Calc(CurrentBuf);
			//占空比已同步，跳转到正常运行阶段
			if(TargetCurrent==CurrentBuf)
				{
				IsCurrentRampUp=1; //标记电流爬升结束
				OutputFSMState=OutCH_OutputEnabled;
				}
	    break;
		//输出通道正常运行阶段
		case OutCH_OutputEnabled:
			if(!TargetCurrent)OutputFSMState=OutCH_GracefulShut;  //系统电流配置为0，进入软关机阶段开始下调输出电压
			else if(TargetCurrent==-1)OutputFSMState=OutCH_EnterIdle;	//系统电流配置为-1，说明需要暂停LED电流，跳转到暂停流程
			else if(TargetCurrent!=CurrentBuf)OutputFSMState=OutCH_SubmitDuty; //占空比发生变更，开始进行处理
			break;
		//输出通道软关机控制
		case OutCH_GracefulShut:
			//打开LEDMOS并关闭DCDC
			LEDMOS=1;
		  BOOSTRUN=0;
			//复位PWMDAC
		  PWMDACEN=0;
		  PreChargeDACDuty=2399;
			PWMDuty=0;
		  IsNeedToUploadPWM=1;
		  //跳转到等待输出电压衰减的过程
		  PreChargeFSMTimer=24; //等待输出电压衰减的过程最多等待3秒
		  OutputFSMState=OutCH_WaitVOUTDecay;
		  break;
		//DCDC关闭，等待输出电压衰减
		case OutCH_WaitVOUTDecay:
		  //等待输出电压衰减
		  if(Data.OutputVoltage>15.6&&PreChargeFSMTimer)break;
			//关闭预充PWMDAC
		  PreChargeDACDuty=0;
		  IsNeedToUploadPWM=1;
			//输出电压衰减结束，关闭LEDMOS和辅助电源并回到待机状态
		  LEDMOS=0;
			AUXEN=0;
		  PreChargeFSMTimer=0; //复位计时器
	    OutputFSMState=OutCH_Standby;
			break;
		//需要暂时关闭LED，在进入idle之前的准备
		case OutCH_EnterIdle:
			//立即让预充PWMDAC把电压钳住
		  PreChargeDACDuty=0x82A; //0x82A=87.128%=11.29-0.4815*14.4->(0.87128*5)
		  IsNeedToUploadPWM=1;
			//等待输出电压下降
		  if(CurrentMode->ModeIdx!=Mode_Beacon&&Data.OutputVoltage>17.3)break;
		  LEDMOS=0; //断开LEDMOS切断电流
		  OutputFSMState=OutCH_OutputIdle; //进入idle状态
		  break;
		//暂时关闭LED的等待
		case OutCH_OutputIdle:
			if(TargetCurrent==0)OutputFSMState=OutCH_GracefulShut;  //系统电流配置为0，进入软关机阶段
			if(TargetCurrent>0) //LED电流重新打开，需要启动输出
				{
				LEDMOS=1; //打开LEDMOS，接通电流
				PreChargeDACDuty=0;
				IsNeedToUploadPWM=1; //令PWMDAC开始SysDown，LED发光
			  OutputFSMState=OutCH_SubmitDuty; //应用最新的占空比
				}
			break;
		//卡出来的非法状态回到默认待机
		default:OutputFSMState=OutCH_PreChargeFailed;
		}
	}
