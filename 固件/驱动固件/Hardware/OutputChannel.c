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

//内部状态表
static code OutChFSMStateDef NeedsOFFStateTable[]=
	{
	//该状态表记录了可以通过把目标电流设置为0实现关机的状态
	OutCH_1LumenOpenRun,	
	OutCH_ReleasePreCharge,
	OutCH_SubmitDuty,
	OutCH_OutputEnabled,
	OutCH_OutputIdle
	};

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
	return buf;
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
	//调用复位函数重置所有状态
  OutputChannel_DeInit();
	//开始配置IO	
	GPIO_ConfigGPIOMode(PWMDACENIOG,GPIOMask(PWMDACENIOx),&OCInitCfg);	
	GPIO_ConfigGPIOMode(AUXENIOG,GPIOMask(AUXENIOx),&OCInitCfg);			
	GPIO_ConfigGPIOMode(BOOSTRUNIOG,GPIOMask(BOOSTRUNIOx),&OCInitCfg);		
	GPIO_ConfigGPIOMode(LEDNegMOSIOG,GPIOMask(LEDNegMOSIOx),&OCInitCfg);
  GPIO_ConfigGPIOMode(SYSHBLEDIOG,GPIOMask(SYSHBLEDIOx),&OCInitCfg);		
	}

//预充电状态机计时器
void OCFSM_TIMHandler(void)
	{
	//心跳LED控制	
	if(CurrentMode->ModeIdx==Mode_Fault) //发生故障时HB快闪
		SYSHBLED=SYSHBLED?0:1; //翻转LED
	else if(CurrentMode->ModeIdx==Mode_1Lumen)SYSHBLED=0; //极低亮禁用心跳LED省电
	else if(GetIfOutputEnabled())SYSHBLED=1;//输出已启用，LED配置为1		
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
	/**********************************************	
	系统在开环运行、应用占空比和输出已正常启用的时
	候，返回启用状态。这里为什么不用三个if或者查表
	是利用了三种状态处于连续的位置：
	OutCH_SubmitDuty为ID5
	OutCH_OutputEnabled为ID6
	OutCH_1LumenOpenRun为ID7
	这样子比三个If或者switch要省很多空间。
	**********************************************/
	if(OutputFSMState>4&&OutputFSMState<8)return 1;
	//否则返回0
	return 0;
	}

//获取系统是否在安全关机阶段
bit GetIfSystemInPOFFSeq(void)
	{
	/************************************************
	如果系统处在软关机的等待阶段则返回1，不允许系统接
	受任何新的用户操作，直到输出已经完成放电，输出电
	容里面没有多余的电能后才允许继续接收用户操作。
	************************************************/
	if(OutputFSMState==OutCH_GracefulShut||
		 OutputFSMState==OutCH_WaitVOUTDecay)return 1;
	//系统已经关闭，返回0
	return 0;
	}	
	
//输出通道试运行
void OutputChannel_TestRun(void)
	{
	char retry=100;
	//打开辅助电源和PWMDAC
	AUXEN=1;
	PWMDACEN=1;
	PWM_ForceEnableOut(1);
	//延时40mS后检测电压，如果电压大于8V则正常启动进行检测
	delay_ms(40);
	SystemTelemHandler();
	//电池电压正常，令3787开始运行，进行输出检查
	if(Data.RawBattVolt>8)
		{
		//令3787EN=1，启动输出
	  BOOSTRUN=1; 
		//启动输出后循环读取DCDC的输出电压检查DCDC模块，预充系统是否正常
		do
			{
			SystemTelemHandler();
			//DCDC输出过压，立即关闭系统并报错
			if(Data.OutputVoltage>16.5)
				{
				ReportError(Fault_DCDCPreChargeFailed);
				break;
				}
			//DCDC输出正常建立，退出
			else if(Data.OutputVoltage>14.0)break;
			//检查失败，延时5mS后再试
			delay_ms(5);
			}
		while(--retry);		
		}
	//检查结束，关闭DCDC并复位PWMDAC
	PWM_ForceEnableOut(0);
	OutputChannel_DeInit();
	//根据超时结果判断是否异常
	if(!retry)ReportError(Fault_DCDCFailedToStart);
	}
	
//输出通道计算
void OutputChannel_Calc(void)
	{
	int TargetCurrent;
	char i;
	//读取目标电流并应用温控加权数据
	if(Current>0)
		{
		//取出温控限流数据
		TargetCurrent=ThermalILIMCalc();
		//如果目标电流小于当前挡位的温控限制值，则应用当前设置的电流值
		if(Current<TargetCurrent)TargetCurrent=Current;
		}
	//电流值为0或者-1，直接读取目标电流值
	else TargetCurrent=Current;
	//检测系统是否需要关机，进入关机状态	
	i=sizeof(NeedsOFFStateTable);
	do
	  {
		//如果输出状态机当前的状态为表内对应值，且系统目标电流为0，则进入关机处理
		if(OutputFSMState==NeedsOFFStateTable[i-1]&&!TargetCurrent)
			{	
			OutputFSMState=OutCH_GracefulShut;
			//打断循环阻止继续查找
			break;
			}
		}
	while(--i);
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
		   if(CurrentMode->ModeIdx==Mode_1Lumen)CurrentBuf=CalcIREFValue(25); //1LM挡位下为了避免运放同相输入接地导致CC环拉死控制器，所以随便给一个初值
			 else CurrentBuf=TargetCurrent>CalcIREFValue(1500)?CalcIREFValue(1500):TargetCurrent;
			 PWMDuty=Duty_Calc(CurrentBuf);  //配置流程是如果当前电流大于1.5A，则钳位到1.5A，然后用这个初值配置PWMDAC
       //启动CV限压环DAC
       PreChargeDACDuty=CVPreStartDACVal;
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
			if(Data.OutputVoltage>14.0)OutputFSMState=CurrentMode->ModeIdx==Mode_1Lumen?OutCH_1LumenOpenRun:OutCH_ReleasePreCharge; //电压起来了，继续启动流程
		  //等待超时后报错
		  if(PreChargeFSMTimer>0)break;
		  ReportError(Fault_DCDCFailedToStart);
		  OutputFSMState=OutCH_PreChargeFailed;
			break;
		//启动步骤4，逐步下调预充PWMDAC抬升输出电压
		case OutCH_ReleasePreCharge:
			//接通LED负极FET，LED开始发光
			LEDMOS=1;
			//开始逐步下调预充占空比把输出电压调到额定值
		  if(IsNeedToUploadPWM)break; //上次调整还未完毕
		  if(PreChargeDACDuty>0)
				{
				//继续进行调整，下调占空比
				TargetCurrent=1+(TargetCurrent/25);
				if(TargetCurrent>200)TargetCurrent=200; //计算出每次PWMDAC递减的值
				PreChargeDACDuty-=TargetCurrent;
				if(PreChargeDACDuty<0)PreChargeDACDuty=0; //禁止占空比为负数
				//预充状态机计时为20，需要20个主循环确保最新值已被应用才继续
				PreChargeFSMTimer=20;
				//标记占空比已更新，需要上传最新值	
				IsNeedToUploadPWM=1;
				}
			//预充PWMDAC输出=0，说明预充完成。系统开始进行预充状态机倒计时
			else if(PreChargeFSMTimer>0)PreChargeFSMTimer--;	
			//倒计时结束，系统已经应用了输出电流，此时按照输出电流是否匹配跳转到目标状态
			else OutputFSMState=(CurrentBuf==TargetCurrent)?OutCH_OutputEnabled:OutCH_SubmitDuty;
		  break;
		//启动步骤5：应用整定PWMDAC占空比抬升输出电流到目标值
		case OutCH_SubmitDuty:
			if(IsNeedToUploadPWM)break; //PWM正在应用中，等待
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
		//正常运行，1流明开环运行挡位
		case OutCH_1LumenOpenRun:
			//接通LED负极FET，LED开始发光
			LEDMOS=1;
		  //设置限压环DAC到其终值
	    IsNeedToUploadPWM=1;
		  PreChargeDACDuty=OneLMDACVal;
			//系统尝试进入普通月光，返回到下调占空比模式
		  if(TargetCurrent>2)OutputFSMState=OutCH_ReleasePreCharge;
		  break;
		//输出通道正常运行阶段
		case OutCH_OutputEnabled:
			if(TargetCurrent==2)OutputFSMState=OutCH_1LumenOpenRun; //进入1流明开环运行模式
			if(TargetCurrent==-1)OutputFSMState=OutCH_EnterIdle;	//系统电流配置为-1，说明需要暂停LED电流，跳转到暂停流程
			if(TargetCurrent!=CurrentBuf)OutputFSMState=OutCH_SubmitDuty; //占空比发生变更，开始进行处理
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
		  PreChargeDACDuty=CVPreStartDACVal;
		  IsNeedToUploadPWM=1;
			//等待输出电压下降
		  if(CurrentMode->ModeIdx!=Mode_Beacon&&Data.OutputVoltage>17.3)break;
		  LEDMOS=0; //断开LEDMOS切断电流
		  OutputFSMState=OutCH_OutputIdle; //进入idle状态
		  break;
		//暂时关闭LED的等待
		case OutCH_OutputIdle:
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
