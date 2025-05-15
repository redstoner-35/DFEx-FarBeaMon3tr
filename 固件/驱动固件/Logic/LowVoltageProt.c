#include "BattDisplay.h"
#include "ModeControl.h"
#include "LowVoltProt.h"
#include "OutputChannel.h"
#include "SideKey.h"

//内部变量
static xdata char BattAlertTimer=0; //电池低电压告警处理
static xdata char RampCurrentRiseAttmTIM=0; //无极调光恢复电流的计时器	
static xdata char TryTurboILIMTimer=0; //尝试下调极亮的冷却计时

//全局参考
xdata int TurboILIM; //极亮电流限制


//低电量保护函数
static void StartBattAlertTimer(void)
	{
	//启动定时器
	if(BattAlertTimer)return;
	BattAlertTimer=1;
	}	

//电池低电量报警处理函数
void BattAlertTIMHandler(void)
	{
	//极亮下调电流计时	
	if(TryTurboILIMTimer>0)TryTurboILIMTimer--;
	//无极调光警报定时
	if(RampCurrentRiseAttmTIM>0&&RampCurrentRiseAttmTIM<9)RampCurrentRiseAttmTIM++;
	//电量警报
	if(BattAlertTimer>0&&BattAlertTimer<(BatteryAlertDelay+1))BattAlertTimer++;
	}	
	
//计算极亮挡位电流的限制值
void CalcTurboILIM(void)
	{
	if(Battery>3.6)TurboILIM=QueryCurrentGearILED(); //电池电压大于3.6时按照目标电流去取
	else TurboILIM=CalcIREFValue(25000); //电池电压低，极亮锁25A输出
	}	
	
//极亮挡位时动态尝试极亮运行值的功能
void TurboLVILIMProcess(void)	
	{
	//电池电压过低，立即退出极亮
	if(IsBatteryFault)
		{
		StartBattAlertTimer();
		if(BattAlertTimer<BatteryFaultDelay)return;
		//时间到，立即换挡
		BattAlertTimer=0;	
		SwitchToGear(IsRampEnabled?Mode_Ramp:Mode_High);
		}
	//电池电压低或者触发输入限流，下调极亮
	else if(IsBatteryAlert)
		{
		//在电流RampUp的过程中如果触发输入限流则立即将当前电流值设置为极亮限流
		if(IsCurrentRampUp&&CurrentBuf<QueryCurrentGearILED())
			{
		  CurrentBuf=TurboILIM;
			IsCurrentRampUp=1; //强制set标记位确保极亮限流只执行一次
			return;
			}
		//手电已经进入极亮，正常执行限流处理
		if(TryTurboILIMTimer)return;
		TurboILIM-=25;
		TryTurboILIMTimer=TurboILIMTryCDTime; //应用定时，降低电流后等待一会再判断
		//判断电流是否仍在极亮区间内
		if(TurboILIM>CalcIREFValue(13000))return;
		//尝试到13A仍然无法满足极亮，退出极亮
		TurboILIM=CalcIREFValue(13000);
		SwitchToGear(IsRampEnabled?Mode_Ramp:Mode_High);
		}
	}

//电池低电量保护函数
void BatteryLowAlertProcess(bool IsNeedToShutOff,ModeIdxDef ModeJump)
	{
	char Thr;
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
  else //发生低压告警立即启动定时器
		{
	  Thr=BatteryFaultDelay;
		StartBattAlertTimer(); 
		}
	//当前模式需要关机
	if(IsNeedToShutOff||IsChangingGear)
		 {
		 //电池电压低于关机阈值足够时间，立即关闭
		 if(IsBatteryFault&&BattAlertTimer>Thr)ReturnToOFFState(); 
		 }
	//不需要关机，触发换挡动作
	else if(BattAlertTimer>Thr)
		 {
	   BattAlertTimer=0;//重置定时器至初始值
	   SwitchToGear(ModeJump); //复位到指定挡位
		 }
	}		

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
