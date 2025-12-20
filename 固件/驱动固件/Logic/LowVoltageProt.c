/****************************************************************************/
/** \file LowVltageProt.c
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个文件为顶层应用层逻辑文件。该文件负责实现系统常规、无极调光
以及特殊功能挡位的低电量保护逻辑。并且实现驱动在极亮挡位下的自适应输入MPPT功能以
及供LED管理器查询系统当前降档状态的查询功能。

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "BattDisplay.h"
#include "ModeControl.h"
#include "LowVoltProt.h"
#include "OutputChannel.h"
#include "SideKey.h"
#include "ADCCfg.h"
#include "SelfTest.h"
#include "TurboICCMAX.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros - for Parameter Definition
****************************************************************************/

//配置极亮的输入MPPT开始启动电池放电不足警告的阈值，单位是1%
#define TurboMPPTAlertRatio 2 	

//极亮启动过程中，电池最大允许的和运行前的压差(V)
#define BatteryMaximumTurboVdroop 1.4  

#define BatteryAlertDelay 10 		//电池警报延迟	
#define BatteryFaultDelay 2 		//电池故障强制跳档/关机的延迟
#define TurboILIMTryCDTime 4 		//每次极亮尝试下调电流的冷却时间（单位是1/8秒）

/****************************************************************************/
/*	Local pre-processor symbols/macros - for Parameter Parsing
****************************************************************************/

//POWER模式下输入MPPT的电流告警负偏移量（使用整数计算方式实现极亮电流的指定百分比）
#define InputMPPTAlmNegOffset ((TurboICCMAX/100)*TurboMPPTAlertRatio)  
#define InputMPPTRawCurrentVal (TurboICCMAX-InputMPPTAlmNegOffset)
#define InputMPPTAlertThershold CalcIREFValue(InputMPPTRawCurrentVal)  

//ECO模式下输入MPPT的电流告警负偏移量（使用整数计算方式实现极亮电流的指定百分比）
#define InuputMPPTECOAlmNegOffset ((ECOTurboICCMAX/100)*TurboMPPTAlertRatio)
#define InputMPPTRawECOCurrentVal (ECOTurboICCMAX-InuputMPPTECOAlmNegOffset)
#define InputMPPTAlertThersholdECO CalcIREFValue(InputMPPTRawECOCurrentVal)   

#if (InputMPPTRawECOCurrentVal >= ECOTurboICCMAX)
	//检测到异常数值时阻止编译通过
	#error "Error 016:Negative Current Offset Of Input MPPT(ECO Mode) is out of range."
#endif

#if (InputMPPTRawCurrentVal >= TurboICCMAX)
	//检测到异常数值时阻止编译通过
	#error "Error 015:Negative Current Offset Of Input MPPT(Power Mode) is out of range."
#endif

/****************************************************************************/
/*	Local variable and Flag definitions('static')
****************************************************************************/
static xdata unsigned char BattAlertTimer; //电池低电压告警处理
static xdata unsigned char RampCurrentRiseAttmTIM; //无极调光恢复电流的计时器	
static unsigned char MPPTStepdownWaitTimer; //MPPT下调极亮等待的计时器

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/
xdata int TurboILIM; 						//极亮电流限制
xdata float BeforeRawBattVolt; 	//开启极亮前的电池电压

/****************************************************************************/
/*	Function implementation - local('static')
****************************************************************************/	

//低电量保护函数
static void StartBattAlertTimer(void)
	{
	//启动定时器
	if(!BattAlertTimer)BattAlertTimer=1;
	}	

/****************************************************************************/
/* Global	Function implementation - Low Voltage Protect Logic handler and 
	 timing handler for Other Mode
****************************************************************************/		
	
//电池低电量报警处理函数
void BattAlertTIMHandler(void)
	{
	//MPPT下调判断
	if(MPPTStepdownWaitTimer)MPPTStepdownWaitTimer--;
	//无极调光警报定时
	if(RampCurrentRiseAttmTIM&&RampCurrentRiseAttmTIM<9)RampCurrentRiseAttmTIM++;
	//电量警报
	if(BattAlertTimer&&BattAlertTimer<(BatteryAlertDelay+1))BattAlertTimer++;
	}	
	
//电池低电量保护函数
void BatteryLowAlertProcess(bool IsNeedToShutOff,ModeIdxDef ModeJump)
	{
	unsigned char Thr=BatteryFaultDelay;
	bit IsChangingGear;
	//获取手电按键的状态
	if(getSideKey1HEvent())IsChangingGear=1;
	else IsChangingGear=getSideKeyHoldEvent();
	//控制计时器启停
	if(!IsBatteryFault) //电池没有发生低压故障
		{
		Thr=BatteryAlertDelay; //没有故障可以慢一点降档
		//当前在换挡阶段或者没有告警，停止计时器,否则启动
		if(!IsBatteryAlert||IsChangingGear)BattAlertTimer=0;
		else StartBattAlertTimer();
		}
  else StartBattAlertTimer();//发生低压告警立即启动定时器
	//定时器计时已满，执行对应的动作
	if(BattAlertTimer>Thr)
		{
		//当前挡位处于需要在触发低电量保护时主动关机的状态	
		if(IsNeedToShutOff)ReturnToOFFState();
		//当前处于换挡模式不允许执行降档但是需要判断电池是否过低然后强制关闭
		else if(IsChangingGear&&IsBatteryFault)ReturnToOFFState();
		//不需要关机，触发换挡动作
		else
			{
			BattAlertTimer=0;//重置定时器至初始值
			SwitchToGear(ModeJump); //复位到指定挡位
			}
		}
	}			
	
/****************************************************************************/
/* Global	Function implementation - Low Voltage Protect Logic handler and 
	 Input Auto MPPT Tracking handler for Turbo Mode
****************************************************************************/		
	
//计算极亮挡位电流的限制值
void CalcTurboILIM(void)
	{
	TurboILIM=QueryCurrentGearILED(); //默认上限按照目标电流去取
	if(CurrentMode->ModeIdx!=Mode_Turbo)return; //非极亮挡位不重置MPPT
	
	//ECO模式开启时使用低电流作为极亮MAX
	if(!IsPowerModeEnabled)TurboILIM=CalcIREFValue(ECOTurboICCMAX); 
	
	IsCurrentRampUp=0; //复位标志位重置MPPT系统
	
	//切换到极亮之前取样电池实时电压并减去允许压差作为实时采样值
	BeforeRawBattVolt=Data.RawBattVolt-BatteryMaximumTurboVdroop; 
	}	
	
//极亮挡位进行MPPT输入监测和低电量保护的处理
void TurboLVILIMProcess(void)	
	{
	//电池电压低且MPPT协商已结束,执行正常低电量判断
	if(IsBatteryAlert&&IsCurrentRampUp)	
		{
		//启动定时器并开始计时
		StartBattAlertTimer();
		if(BattAlertTimer<BatteryAlertDelay)return;
		//时间到，立即换挡
		BattAlertTimer=0;	
		SwitchToGear(IsRampEnabled?Mode_Ramp:Mode_High);
		}
	//触发输入限流,立即停止MPPT协商
	else if(IsInputLimited)
		{
		//MPPT协商已停止，进行输入限流下调判断
		if(IsCurrentRampUp)
			{
			//刚完成一次调整，需要等待ADC采样新的输入结果之后输入限流bit才会刷新，所以要倒计时
			if(MPPTStepdownWaitTimer)return;
			//计时结束，开始下调
			TurboILIM-=CalcIREFValue(50);
			MPPTStepdownWaitTimer=4; //每次下调减少50mA，等待0.5秒
			//判断电流是否仍在极亮区间内
			if(TurboILIM>CalcIREFValue(13000))return;
			//尝试到13A仍然无法满足极亮，退出极亮
			TurboILIM=CalcIREFValue(13000);
			SwitchToGear(IsRampEnabled?Mode_Ramp:Mode_High);
			}
		//在电流RampUp的过程中如果触发输入限流则立即将当前电流值设置为极亮限流
		else if(CurrentBuf<QueryCurrentGearILED())
			{
			MPPTStepdownWaitTimer=8; //MPPT协商停止，等待1秒的消隐间隔之后再进行输入限流判断
 			TurboILIM=CurrentBuf; //使用当前应用的电流作为极亮电流限制
			IsCurrentRampUp=1; //强制set标记位，标记MPPT试探停止
			}
		}
	//没有告警，复位定时器
	else BattAlertTimer=0;
	}

/****************************************************************************/
/* Global	Function implementation - Low Voltage Protect Logic handler for
	 Ramp(Stepless adjustment) mode
****************************************************************************/			
	
//无极调光开机时恢复低压保护限流的处理	
void RampRestoreLVProtToMax(void)
	{
	if(IsBatteryAlert||IsBatteryFault)return;
	if(BattState==Battery_Plenty)SysCfg.RampCurrentLimit=CurrentMode->Current; //电池电量回升到充足状态，复位电流限制
	}
	
//无极调光的低电压保护
void RampLowVoltHandler(void)
	{
	if(!IsBatteryAlert&&!IsBatteryFault)//没有告警
		{
		BattAlertTimer=0;
		if(BattState==Battery_Plenty) //电池电量回升到充足状态，缓慢增加电流限制
			{
	    if(SysCfg.RampCurrentLimit<CurrentMode->Current)
				 {
			   if(!RampCurrentRiseAttmTIM)RampCurrentRiseAttmTIM=1; //启动定时器开始计时
				 else if(RampCurrentRiseAttmTIM<9)return; //时间未到
         RampCurrentRiseAttmTIM=1;
				 if(SysCfg.RampBattThres>CurrentMode->LowVoltThres)SysCfg.RampBattThres=CurrentMode->LowVoltThres; //电压检测达到上限，禁止继续增加
				 else SysCfg.RampBattThres+=50; //电压检测上调50mV
         if(SysCfg.RampCurrentLimit>CurrentMode->Current)SysCfg.RampCurrentLimit=CurrentMode->Current;//增加电流之后检测电流值是否超出允许值
				 else SysCfg.RampCurrentLimit+=250;	//电流上调250mA		 
				 }
			else RampCurrentRiseAttmTIM=0; //已达到电流上限禁止继续增加
			}
		return;
		}
	else RampCurrentRiseAttmTIM=0; //触发警报，复位尝试增加电流的定时器
	//低压告警发生，启动定时器
	StartBattAlertTimer(); //发生命令启动定时器
	if(IsBatteryFault&&BattAlertTimer>4)ReturnToOFFState(); //电池电压低于关机阈值大于0.5秒，立即关闭
	else if(BattAlertTimer>BatteryAlertDelay) //电池挡位触发
		{
		if(SysCfg.RampCurrentLimit>CalcIREFValue(500))SysCfg.RampCurrentLimit-=250; //电流下调250mA
		if(SysCfg.RampBattThres>2750)SysCfg.RampBattThres-=25; //减少25mV
    BattAlertTimer=1;//重置定时器
		}
	}	
	
/****************************************************************************/
/* Global	Function implementation - Stepdown reason Query
****************************************************************************/		
	
//获取系统在极亮模式开启时的功率限制状态
StepDownReasonDef QuerySystemTurboILIMState(void)
	{
	//极亮没有开启，返回OFF State
	if(CurrentMode->ModeIdx!=Mode_Turbo)return StepDown_OFF;
	//触发输入限流保护，指示输入限流激活	
	if(TurboILIM<(IsPowerModeEnabled?InputMPPTAlertThershold:InputMPPTAlertThersholdECO))return StepDown_BattAlert;
	//开启ECO模式，指示ECO模式激活
	if(!IsPowerModeEnabled)return StepDown_ECOModeEnabled;
  //其余情况没有发生电流被限制的情况，返回OFF
	return StepDown_OFF;
	}	
/*********************************  End Of File  ************************************/
