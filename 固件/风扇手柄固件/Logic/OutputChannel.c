#include "cms8s6990.h"
#include "PinDefs.h"
#include "PWMCfg.h"
#include "delay.h"
#include "GPIO.h"
#include "ADCCfg.h"
#include "OutputChannel.h"
#include "ModeSel.h"

//内部sbit
sbit PINStrap=PinStrapIOP^PinStrapIOx;
sbit DCDCEN=DCDCENIOP^DCDCENIOx;
sbit FANPWREN=FANPWRENIOP^FANPWRENIOx;

//内部define
#define UsingFanIntSoftStart //使用风扇内置软起动（如果发现风扇启动异常则使能此define）

//内部变量
bit IsEnablePWMFan; //内部标志位，是否开启PWM风扇模式
static OCFSMStateDef OCState; //输出通道状态
static xdata unsigned char OCFSMWait=0; //内部等待函数
static xdata float RaiseVoltProc; //电压上升的函数

//外部调整变量
xdata MinMaxDutyVOutDef VMinMaxCfg;  //存储系统最小电压电流和无极调速上限配置
xdata float TargetVoltage;          //目标风扇电压(仅电压模式生效)
xdata float TargetfanSpeed; //目标风扇速度
bit IsUpdateFanSpeed; //更新风扇速度

//根据传入速度计算风扇PWM数值的函数
static int SetFanPWMProcess(void)
	{
	float buf;
	//取数值
	if(!IsEnablePWMFan)buf=100; //电压模式PWM全高
	else if(TargetfanSpeed>100)buf=100;
	else buf=TargetfanSpeed;            //限制数值
	//乘以PWM Step然后除以100得到PWM取值
	buf*=FanPWMStepConstant;
	buf/=(float)100;
	//限制参数取值为0-FanPWMStepConstant
	if(buf<(float)FanPWMStepConstant)return (int)buf;
	return FanPWMStepConstant;
	}

//根据传入电压计算CV DAC PWM占空比的函数
static void SetCVDACProcess(float VIN)
	{	
	float buf;
	//输入参数限幅
	if(VIN>11.98)buf=11.98;
	else if(VIN<5.1)buf=5.1;
	else buf=VIN;
	//开始调用魔法公式计算
  buf=0.4314*(buf-(float)1);
	buf=4.7288-buf; //计算得出运放输出的目标电压
	buf=buf/Data.MCUVDD; 
	buf*=100; //和当前MCU的VIN电压相除得到PWMDAC的占空比
	if(buf<0)buf=0;
	if(buf>100)buf=100; //占空比数值只能是0和100
  //计算完毕，写CVDAC的PWM
	CVDACTargetDuty=buf;	
	}

//获取风扇输出是否已经开启
bit GetIfFanOutputEnabled(void)
	{
	switch(OCState)
		{
		case OCFSM_RaiseVOUT:
		case OCFSM_PWMRampingUp: 
	  case OCFSM_NormalOperation:return 1;
		}
	//其余情况返回0
	return 0;
	}	
	
//输出通道状态机运算
void OutputChannel_Calc(void)
	{
	//状态机处理
	switch(OCState)
		{
		//风扇处于关闭状态
		case OCFSM_Idle: 
			 //输出通道未启动，停止运行
			 if(TargetVoltage==0&&!TargetfanSpeed)break;
		   CVDACTargetDuty=0;
		   OCFSMWait=0; //不需要等待
		   FanPWMDuty=VMinMaxCfg.SysStartUpDuty;
		   IsNeedToUploadPWM=1; //默认风扇自身PWM输出1
			 OCState=IsEnablePWMFan?OCFSM_Start_DCDC:OCFSM_Voltage_PreBIAS;
			 break;
		
		//系统工作在电压模式，预先送PWMDAC基准
		case OCFSM_Voltage_PreBIAS: 
			 if(IsNeedToUploadPWM)break; //占空比应用中，退出
		   RaiseVoltProc=VMinMaxCfg.SysMinVolt;
		   SetCVDACProcess(VMinMaxCfg.SysMinVolt); //送低电压让风扇先启动
		   IsNeedToUploadPWM=1;
		   OCFSMWait=0xFF; //等待255个PWM周期后再启动DCDC
		   OCState=OCFSM_Start_DCDC;
		   break;
		case OCFSM_Start_DCDC: //等待DCDC启动
			 if(IsNeedToUploadPWM)break;
		   //等待计时结束，结束后启动DCDC
		   if(OCFSMWait)OCFSMWait--;
		   else
				 {
				 FANPWREN=1;
				 delay_ms(5);
				 DCDCEN=1;
				 if(IsEnablePWMFan)OCState=OCFSM_PWMRampingUp;
				 else OCState=OCFSM_RaiseVOUT;
				 }
		   break;
		//风扇已经通电，开始逐步抬高电压提升转速避免DCDC过载(电压模式)
    case OCFSM_RaiseVOUT:		
      if(TargetVoltage==0||!TargetfanSpeed)OCState=OCFSM_ShutOFF; //输出电压设置为0或者风扇速度=0，关闭风扇			
			if(IsNeedToUploadPWM)break;
      //电压抬到顶了
		  if(RaiseVoltProc>=TargetVoltage)
				{
				SetCVDACProcess(TargetVoltage);
				OCState=OCFSM_NormalOperation;
				IsNeedToUploadPWM=1;
				}
			else if(OCFSMWait)OCFSMWait--;
			else
				{
				RaiseVoltProc+=0.1;
				SetCVDACProcess(RaiseVoltProc); //电压增加0.01V，应用电压结果
				OCFSMWait=5;
				IsNeedToUploadPWM=1;
				}
			break;
		//风扇已经通电，开始逐步抬升PWM占空比让风扇受控加速(PWM模式)
		case OCFSM_PWMRampingUp: 
			if(TargetVoltage==0||!TargetfanSpeed)OCState=OCFSM_ShutOFF; //输出电压设置为0或者风扇速度=0，关闭风扇		
		  #ifndef UsingFanIntSoftStart
			if(IsNeedToUploadPWM)break;		  
	    //PWM已经加满了
		  if(FanPWMDuty>=SetFanPWMProcess())
				{
				FanPWMDuty=SetFanPWMProcess();
				OCState=OCFSM_NormalOperation;
				IsNeedToUploadPWM=1;
				}
			//时间还没到，等待
			else if(OCFSMWait)OCFSMWait--;
			else 
				{
				//开始逐步提升风扇占空比进行加速处理
				FanPWMDuty++;
				OCFSMWait=2;
				if(FanPWMDuty>FanPWMStepConstant)FanPWMDuty=FanPWMStepConstant; //数值限幅
				IsNeedToUploadPWM=1;
				}
			#else
			//使用风扇内置软起，直接一步到位加到目标值，然后开始运行
			FanPWMDuty=SetFanPWMProcess();
			OCState=OCFSM_NormalOperation;
			IsNeedToUploadPWM=1;	
			#endif
			break;
	  //风扇启动完毕，进入正常输出
		case OCFSM_NormalOperation:
			if(TargetVoltage==0||!TargetfanSpeed)OCState=OCFSM_ShutOFF; //输出电压设置为0或者风扇速度=0，关闭风扇		
		  //触发风扇速度更新
			if(!IsUpdateFanSpeed||IsNeedToUploadPWM)break;
			//非极速模式，直接应用占空比
		  if(CurrentMode->ModeIdx!=Mode_Turbo)
				{
				FanPWMDuty=SetFanPWMProcess();
				if(!IsEnablePWMFan)SetCVDACProcess(TargetVoltage);
				else CVDACTargetDuty=0;
				RaiseVoltProc=TargetVoltage; 
				IsNeedToUploadPWM=1;
				}
			//极速模式，缓慢增加占空比
			else
				{
				//计时器仍在计时，等待
				if(OCFSMWait)
					{
					OCFSMWait--;
					break;
					}
				else if(!IsEnablePWMFan)
					{
					RaiseVoltProc+=0.1;			
					if(RaiseVoltProc>TargetVoltage)RaiseVoltProc=TargetVoltage;  //进行电压钳位
					SetCVDACProcess(RaiseVoltProc); //电压增加0.01V，应用电压结果
					OCFSMWait=5;
					IsNeedToUploadPWM=1;
					if(RaiseVoltProc<TargetVoltage)break;
					}
				else
				  {
					//使用系统自带软起动
					#ifndef UsingFanIntSoftStart
					FanPWMDuty++;
					if(FanPWMDuty>SetFanPWMProcess())FanPWMDuty=SetFanPWMProcess(); //进行占空比钳位
					OCFSMWait=2;
					IsNeedToUploadPWM=1;
					if(FanPWMDuty<SetFanPWMProcess())break;
					#else
					//使用风扇自带软起功能
					FanPWMDuty=SetFanPWMProcess();	
					IsNeedToUploadPWM=1;
					#endif
					}
				}
		  //风扇速度更新完毕，清除标志位
		  IsUpdateFanSpeed=0;
			break;
				 
	  case OCFSM_ShutOFF: //输出状态机关闭风扇的流程
		  //断开DCDC，延迟10mS后关闭风扇电源
		  DCDCEN=0;
			delay_ms(10);
			FANPWREN=0;
			//令PWM输出=0
		  FanPWMDuty=0;
			CVDACTargetDuty=0;
			IsNeedToUploadPWM=1;
			//复位电压记录
		  RaiseVoltProc=0;
			//返回到待机状态
	    IsUpdateFanSpeed=0; //复位标记位
			OCState=OCFSM_Idle;
		  break;
		}
	//取消内部宏的定义
	#undef FanStartPWMParam
	}

//输出状态机复位
void OutputChannel_DeInit(void)
	{
	DCDCEN=0;	
	FANPWREN=0;
	PINStrap=0;
	//复位变量
	IsUpdateFanSpeed=0;
	RaiseVoltProc=0;
	TargetVoltage=0;
	TargetfanSpeed=0;
  OCState=OCFSM_Idle;
	}	
	
//初始化输出通道状态机
void OutputChannel_Init(void)
	{
	GPIOCfgDef LEDInitCfg;
	//设置结构体
	LEDInitCfg.Mode=GPIO_Out_PP;
  LEDInitCfg.Slew=GPIO_Slow_Slew;		
	LEDInitCfg.DRVCurrent=GPIO_High_Current; //配置为低斜率大电流的推挽输出
	//初始化bit		
	DCDCEN=0;	
	FANPWREN=0;
  //初始化DCDC-EN和风扇电源使能IO为推挽输出
  GPIO_ConfigGPIOMode(DCDCENIOG,GPIOMask(DCDCENIOx),&LEDInitCfg); 
	GPIO_ConfigGPIOMode(FANPWRENIOG,GPIOMask(FANPWRENIOx),&LEDInitCfg); 		
	//开始准备读取strap
	LEDInitCfg.Mode=GPIO_IPU;
	GPIO_ConfigGPIOMode(PinStrapIOG,GPIOMask(PinStrapIOx),&LEDInitCfg);  //配置为输入上拉
	delay_ms(40);
	if(PINStrap)IsEnablePWMFan=0;   //R17=DNP 风扇配置为电压调速模式
  else IsEnablePWMFan=1;          //R17=0R 风扇配置为四线模式		
	//读取完毕，令Strap输出=0		
	LEDInitCfg.Mode=GPIO_Out_PP;
	GPIO_ConfigGPIOMode(PinStrapIOG,GPIOMask(PinStrapIOx),&LEDInitCfg);  //配置为推挽输出
	PINStrap=0;
	//复位变量
	IsUpdateFanSpeed=0;
	RaiseVoltProc=0;
	TargetVoltage=0;
	TargetfanSpeed=0;
  OCState=OCFSM_Idle;
	}
