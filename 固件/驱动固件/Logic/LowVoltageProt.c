#include "BattDisplay.h"
#include "ModeControl.h"
#include "LowVoltProt.h"
#include "OutputChannel.h"
#include "SideKey.h"
#include "SelfTest.h"

//内部变量
static xdata char BattAlertTimer=0; //电池低电压告警处理
static xdata char RampCurrentRiseAttmTIM=0; //无极调光恢复电流的计时器	
static char MPPTStepdownWaitTimer; //MPPT下调极亮等待的计时器

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
	//MPPT下调判断
	if(MPPTStepdownWaitTimer>0)MPPTStepdownWaitTimer--;
	//无极调光警报定时
	if(RampCurrentRiseAttmTIM>0&&RampCurrentRiseAttmTIM<9)RampCurrentRiseAttmTIM++;
	//电量警报
	if(BattAlertTimer>0&&BattAlertTimer<(BatteryAlertDelay+1))BattAlertTimer++;
	}	
	
//计算极亮挡位电流的限制值
void CalcTurboILIM(void)
	{
	IsCurrentRampUp=0; //复位标志位重置MPPT系统
	if(Battery>3.6)TurboILIM=QueryCurrentGearILED(); //电池电压大于3.6时按照目标电流去取
	else TurboILIM=CalcIREFValue(25000); //电池电压低，极亮锁25A输出
	}	
	
//极亮挡位进行计时，降档至高亮的处理
static void TurboStepWaitTimerHandler(bit IsFault)
	{
	StartBattAlertTimer();
	if(BattAlertTimer<(IsFault?BatteryFaultDelay:BatteryAlertDelay))return;
	//时间到，立即换挡
	BattAlertTimer=0;	
	SwitchToGear(IsRampEnabled?Mode_Ramp:Mode_High);
	}	
	
//极亮挡位进行MPPT输入监测和低电量保护的处理
void TurboLVILIMProcess(void)	
	{
	//电池电压严重过低启动计时，如果持续过久则立即退出极亮
	if(IsBatteryFault)TurboStepWaitTimerHandler(1);
	//电池电压低且MPPT协商已结束,执行正常低电量判断
	else if(IsBatteryAlert&&IsCurrentRampUp)	
		{
		//进行计时，时间到则执行跳档
		TurboStepWaitTimerHandler(0);
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

//电池低电量保护函数
void BatteryLowAlertProcess(bool IsNeedToShutOff,ModeIdxDef ModeJump)
	{
	char Thr=BatteryFaultDelay;
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
