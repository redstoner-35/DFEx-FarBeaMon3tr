#include "SpecialMode.h"
#include "LEDMgmt.h"
#include "SideKey.h"
#include "LocateLED.h"
#include "BattDisplay.h"
#include "OutputChannel.h"
#include "SysConfig.h"
#include "ADCCfg.h"
#include "TempControl.h"
#include "LowVoltProt.h"
#include "SelfTest.h"
#include "SOS.h"
#include "Beacon.h"
#include "Strobe.h"

//极亮和爆闪电流选择
//#define TurboCurrent32A  //注释掉开启36A极亮，否则极亮电流为31.75A（适配FV7212D）
#define FullPowerStrobe //保留则开启全功率爆闪
#define FullPowerBeacon //全功率信标

//挡位结构体
code ModeStrDef ModeSettings[ModeTotalDepth]=
	{
		//关机状态
    {
		Mode_OFF,
		0,
		0,  //电流0mA
		0,  //关机状态阈值为0强制解除警报
		true,
		false,
		}, 
		//出错了
		{
		Mode_Fault,
		0,
		0,  //电流0mA
		0,
		false,
		false,
		}, 
		//月光
		{
		Mode_Moon,
		CalcIREFValue(20),  //实际是20
		0,   //最小电流没用到，无视
		2500,  //2.5V关断
		false, //月光档有专用入口，无需带记忆
		false,
		}, 	
		//极低亮
		{
		Mode_ExtremelyLow,
		CalcIREFValue(200),  //200mA
		0,   //最小电流没用到，无视
		2600,  //2.6V关断
		true, //带记忆
		false,
		}, 	
    //低亮
		{
		Mode_Low,
		CalcIREFValue(1000),  //1000mA电流
		0,   //最小电流没用到，无视
		2800,  //2.8V关断
		true,
		false,
		},
    //中亮
		{
		Mode_Mid,
		CalcIREFValue(2000),  //2000mA电流
		0,   //最小电流没用到，无视
		2900,  //2.9V关断
		true,
		false,
		}, 	
    //中高亮
		{
		Mode_MHigh,
		CalcIREFValue(4000),  //4000mA电流
		0,   //最小电流没用到，无视
		3000,  //3V关断
		true,
		true,
		}, 	
    //高亮
		{
		Mode_High,
		CalcIREFValue(8000),  //8000mA电流
		0,   //最小电流没用到，无视
		3100,  //3.1V关断
		true,
		true,
		}, 	
    //极亮
		{
		Mode_Turbo,
		#ifdef TurboCurrent32A
		CalcIREFValue(31750),  //31.75A电流
		#else
    CalcIREFValue(36000),  //36A电流(针对7175)
	  #endif		
		0,   //最小电流没用到，无视
		3400,  //3.4V关断
		false, //极亮不能带记忆
		true,
		}, 	
    //爆闪		
		{
		Mode_Strobe,
		#ifndef FullPowerStrobe		
		CalcIREFValue(22000),  //22A电流
		#else
			//全功率爆闪激活
			#ifdef TurboCurrent31A
			CalcIREFValue(31750),  //31.75A电流
			#else
			CalcIREFValue(36000),  //36A电流(针对7175)
			#endif		
    #endif			
		0,   //最小电流没用到，无视
		2500,  //2.5V关断(实际上2.7就会拉闸，这里调成2.5是为了避免低电压处理反复触发导致爆闪工作异常)
		false, //爆闪不能带记忆
		true,
		}, 
	  //无极调光		
		{
		Mode_Ramp,
		CalcIREFValue(10000),  //最大 10000mA电流
		CalcIREFValue(100),   //最小 100mA电流
		3200,  //3.2V关断
		false, //不能带记忆  
		true,
		}, 
		//信标模式
		{
		Mode_Beacon,
		#ifdef FullPowerBeacon
			//全功率信标模式激活
			#ifdef TurboCurrent31A
			CalcIREFValue(31750),  //31.75A电流
			#else
			CalcIREFValue(36000),  //36A电流(针对7175)
			#endif	
    #else	
			CalcIREFValue(22000),  //22A电流
		#endif
		0,   //最小电流没用到，无视
		2500,  //2.5V关断(实际上2.7就会拉闸，实际上这里调成2.5是为了避免低电压处理反复触发重置SOS状态机导致SOS工作异常)
		false,	//SOS不能带记忆
		true,
		}, 
	  //SOS
		{
		Mode_SOS,
		CalcIREFValue(14000),  //14A电流
		0,   //最小电流没用到，无视
		2500,  //2.5V关断(实际上2.7就会拉闸，实际上这里调成2.5是为了避免低电压处理反复触发重置SOS状态机导致SOS工作异常)
		false,	//SOS不能带记忆
		true,
		}, 
	};

//全局变量(挡位)
ModeStrDef *CurrentMode; //挡位结构体指针
xdata ModeIdxDef LastMode; //挡位记忆存储
SysConfigDef SysCfg; //系统配置	

//全局变量(状态位)
bit IsRampEnabled; //是否开启无极调光
static bit IsNotifyMaxRampLimitReached=0; //标记无极调光达到最大电流	
	
//软件计时变量
xdata char HoldChangeGearTIM; //挡位模式下长按换挡
xdata char DisplayLockedTIM; //锁定和战术模式进入退出显示	
static xdata char RampDIVCNT; //分频计时器	
	
//获取极亮电流
static int QueryTurboCurrent(void)	
	{
	//极亮MPPT限流启动，从动态极亮限流函数取电流限制
	if(TurboILIM<QueryCurrentGearILED())return TurboILIM;
	//其余情况，从当前挡位拿电流	
	return QueryCurrentGearILED();  
	}

//初始化模式状态机
void ModeFSMInit(void)
{
	char i;
	//初始化无极调光
	SysCfg.RampLimitReachDisplayTIM=0;
  ReadSysConfig(); //从EEPROM内读取无极调光配置
  for(i=0;i<ModeTotalDepth;i++)if(ModeSettings[i].ModeIdx==Mode_Ramp) //遍历挡位设置结构体寻找无极调光的挡位并读取配置
		{
		SysCfg.RampBattThres=ModeSettings[i].LowVoltThres; //低压检测上限恢复
		SysCfg.RampCurrentLimit=ModeSettings[i].Current; //找到挡位数据中无极调光的挡位，电流上限恢复
		//读取数据结束后，检查读入的数据是否合法，不合法就直接修正
		if(SysCfg.RampCurrent<ModeSettings[i].MinCurrent)SysCfg.RampCurrent=ModeSettings[i].MinCurrent;
		if(SysCfg.RampCurrent>SysCfg.RampCurrentLimit)SysCfg.RampCurrent=SysCfg.RampCurrentLimit;
		//读取结束，跳出循环
		break;
		}
	//复位变量
	RampDIVCNT=3; 	
	//挡位模式配置
	ResetSOSModule(); //复位SOS模块
	LastMode=Mode_Low;
	ErrCode=Fault_None; //没有故障
	CurrentMode=&ModeSettings[0]; //记忆重置为第一个档
}	

//挡位状态机所需的软件定时器处理
void ModeFSMTIMHandler(void)
{
	//无极调光相关的定时器
	if(SysCfg.CfgSavedTIM<32)SysCfg.CfgSavedTIM++;
	if(SysCfg.RampLimitReachDisplayTIM>0)
		{
		SysCfg.RampLimitReachDisplayTIM--;
		if(!SysCfg.RampLimitReachDisplayTIM)IsNotifyMaxRampLimitReached=0; //当无极调光显示计时器变为0之后，复位标志位
		}
	//锁定操作提示计时器
  if(DisplayLockedTIM>0)DisplayLockedTIM--;
}

//挡位跳转
void SwitchToGear(ModeIdxDef TargetMode)
	{
	char i;
  int LastICC;
	bool IsLastModeNeedStepDown;
	//记录换档前的结果
	ModeIdxDef BeforeMode=CurrentMode->ModeIdx; 			
	IsLastModeNeedStepDown=CurrentMode->IsNeedStepDown; //存下是否需要降档
	if(CurrentMode->ModeIdx==Mode_Turbo)LastICC=QueryTurboCurrent(); //如果是极亮挡位则需要取当前的电流限制作为最终电流
	else LastICC=CurrentMode->Current; //存储换挡之前的挡位和电流值
	//开始寻找
	for(i=0;i<ModeTotalDepth;i++)if(ModeSettings[i].ModeIdx==TargetMode)
		{
		//复位特殊功能挡位至初始状态
    ResetSOSModule();		//复位整个SOS模块
		BeaconFSM_Reset(); //复位整个信标模块
		ResetStrobeModule(); //复位爆闪控制
		
		//找到匹配index，将对应的结构体基地址赋值给指针
		CurrentMode=&ModeSettings[i]; 
		//进行换挡之后的温控交接和重新计算极亮电流限制
		if(BeforeMode!=Mode_Turbo&&TargetMode==Mode_Turbo)CalcTurboILIM(); //从非极亮挡位进入极亮重新计算电流值并重置MPPT系统
		if(IsLastModeNeedStepDown)RecalcPILoop(LastICC); //重新设置PI环避免电流过调
		//已找到目标挡位，退出循环
		break;
		}
	}
	
//长按关机函数	
void ReturnToOFFState(void)
	{
	if(CurrentMode->ModeIdx==Mode_OFF)return; //关机状态不执行		
	if(CurrentMode->IsModeHasMemory)LastMode=CurrentMode->ModeIdx; //存储关机前的挡位
	SwitchToGear(Mode_OFF); //强制跳回到关机挡位
	}	

//长按换挡的间隔命令生成
void HoldSwitchGearCmdHandler(void)
	{
	char buf;
	if(SysMode||CurrentMode->ModeIdx==Mode_Ramp)HoldChangeGearTIM=0; //战术模式或者进入锁定，以及位于无极调光模式下，禁止换挡系统运行
	else if(!getSideKeyHoldEvent()&&!getSideKey1HEvent())HoldChangeGearTIM=0; //按键松开，计时器复位
	else //执行换挡程序
		{
		buf=HoldChangeGearTIM&0x1F; //取出TIM值
		if(buf==0&&!(HoldChangeGearTIM&0x40))HoldChangeGearTIM|=getSideKey1HEvent()?0x20:0x80; //令换挡命令位1指示换挡可以继续
		HoldChangeGearTIM&=0xE0; //去除掉原始的TIM值
		if(buf<HoldSwitchDelay&&!(HoldChangeGearTIM&0x40))buf++;
		else buf=0;  //时间到，清零结果
		HoldChangeGearTIM|=buf; //把数值写回去
		}
	}	

//侧按长按换挡操作执行
static void SideKeySwitchGearHandler(ModeIdxDef TargetMode)	
	{
	if(!(HoldChangeGearTIM&0x80))return;
	HoldChangeGearTIM&=0x7F; //清除标记位标记本次换挡完成
  SwitchToGear(TargetMode); //换到目标挡位
	}
	
//侧按单击+长按换挡回退操作执行
static void SideKey1HRevGearHandler(ModeIdxDef TargetMode)
	{
	if(!(HoldChangeGearTIM&0x20))return;
	HoldChangeGearTIM&=0xDF; //清除标记位标记本次换挡完成
	SwitchToGear(TargetMode); //换到目标挡位
	}	
	
//无极调光处理
static void RampAdjHandler(void)
	{
	static bit IsKeyPressed=0;	
  int Limit;
	bit IsPress;
  //计算出无极调光上限
	IsPress=(getSideKey1HEvent()||getSideKeyHoldEvent())?1:0;
	Limit=SysCfg.RampCurrentLimit<CurrentMode->Current?SysCfg.RampCurrentLimit:CurrentMode->Current;
	if(Limit<CurrentMode->Current&&IsPress&&SysCfg.RampCurrent>Limit)SysCfg.RampCurrent=Limit; //在电流被限制的情况下用户按下按键尝试调整电流，立即限幅
	//进行亮度调整
	if(getSideKeyHoldEvent()&&!IsKeyPressed) //长按增加电流
			{	
			if(RampDIVCNT>0)RampDIVCNT--;
			else 
				{
				//时间到，开始增加电流
				if(SysCfg.RampCurrent<Limit)SysCfg.RampCurrent++;
				else
					{
					IsNotifyMaxRampLimitReached=1; //标记已达到上限
					SysCfg.RampLimitReachDisplayTIM=4; //熄灭0.5秒指示已经到上限
					SysCfg.RampCurrent=Limit; //限制电流最大值	
					IsKeyPressed=1;
					}
				//计时时间到，复位变量
				RampDIVCNT=3;
				}
			}	
	else if(getSideKey1HEvent()&&!IsKeyPressed) //单击+长按减少电流
		 {
			if(RampDIVCNT>0)RampDIVCNT--;
			else
				{
				if(SysCfg.RampCurrent>CurrentMode->MinCurrent)SysCfg.RampCurrent--; //减少电流	
				else
					{
					IsNotifyMaxRampLimitReached=0;
					SysCfg.RampLimitReachDisplayTIM=4; //熄灭0.5秒指示已经到下限
					SysCfg.RampCurrent=CurrentMode->MinCurrent; //限制电流最小值
					IsKeyPressed=1;
					}
				//计时时间到，复位变量
				RampDIVCNT=3;
				}
		 }
  else if(!IsPress&&IsKeyPressed)
		{
	  IsKeyPressed=0; //用户放开按键，允许调节		
		RampDIVCNT=3; //复位分频计时器
		}
	//进行数据保存的判断
	if(IsPress)SysCfg.CfgSavedTIM=0; //按键按下说明正在调整，复位计时器
	else if(SysCfg.CfgSavedTIM==32)
			{
			SysCfg.CfgSavedTIM++;
			SaveSysConfig(0);  //一段时间内没操作说明已经调节完毕，保存数据
			}
	}

//检测是否需要关机
static void DetectIfNeedsOFF(int ClickCount)
	{
	if(getSideKeyNClickAndHoldEvent()==2)TriggerVshowDisplay();
	if(!SysMode&&ClickCount!=1)return;
	if(SysMode&&getSideKeyHoldEvent())return;
	ReturnToOFFState();//侧按单击或者在战术模式下松开按钮时关机
	}	

//挡位状态机
void ModeSwitchFSM(void)
	{
	char ClickCount;
	//获取按键状态
	if(GetIfSystemInPOFFSeq())return; //系统处于关机过程中，不执行按键处理
	ClickCount=getSideKeyShortPressCount(0);	//读取按键处理函数传过来的参数
	//挡位记忆参数检查和EEPROM记忆
	if(LastMode==Mode_OFF||LastMode>=ModeTotalDepth)LastMode=Mode_Low;
	//状态机
	IsHalfBrightness=0; //按键灯默认全亮
	switch(CurrentMode->ModeIdx)	
		{
		//出现错误	
		case Mode_Fault:
      SysMode=Operation_Normal; //故障后自动回到普通模式			
			if(!getSideKeyLongPressEvent()||IsErrorFatal())break; //用户没有按下按钮或者是致命的错误状态不允许重置
			ClearError(); //消除掉当前错误
		  break;
		//关机状态
		case Mode_OFF:		  
			//处理特殊功能
		  if(LocLEDState==LocateLED_NotEdit)
				{
				SpecialModeOperation(ClickCount);  //只有在退出了定位LED编辑模式之后才能执行
				if(SysMode)break;
				}
		  //处理定位LED变更
			if(LocateLED_Edit(ClickCount))break;
		  //非特殊模式正常单击开关机的事项
			if(ClickCount==1)PowerToNormalMode(LastMode); //侧按单击开机进入循环	
			//进入极亮和爆闪
			else EnterTurboStrobe(ClickCount);		
      if(getSideKeyLongPressEvent())SwitchToGear(Mode_Moon); //长按开机直接进月光					
			if(ClickCount==4) //四击切换挡位模式和无极调光
					{	
					IsRampEnabled=~IsRampEnabled; //转换无极调光状态	
					LEDMode=IsRampEnabled?LED_GreenBlinkThird:LED_RedBlinkThird; //显示是否开启
					SaveSysConfig(0); //保存配置到ROM内
					}
		  //查询电压
			if(getSideKeyNClickAndHoldEvent())TriggerVshowDisplay();
  		break;
		//月光状态
		 case Mode_Moon:
			 IsHalfBrightness=1; //月光模式按键灯亮度减半
			 BatteryLowAlertProcess(true,Mode_Moon);
		   DetectIfNeedsOFF(ClickCount); //执行关机动作检测	
			 //电池电压充足，长按进入低亮挡位
		   if(getSideKeyLongPressEvent())  
					{
					PowerToNormalMode(Mode_ExtremelyLow); //开机到极低亮模式
					if(CurrentMode->ModeIdx==Mode_Moon)break;//换挡之后无法成功离开月光模式，不进行下面的复位操作
					if(IsRampEnabled)RestoreToMinimumSysCurrent(); //如果是无极调光则恢复到最低电流
					HoldChangeGearTIM|=0x40; //禁止换挡系统工作
					}		    
		    break;			
    //无极调光状态				
    case Mode_Ramp:
			  DetectIfNeedsOFF(ClickCount); //检测是否需要关机
				EnterTurboStrobe(ClickCount); //进入极亮或者爆闪的检测
		    //无极调光处理
		    RampLowVoltHandler(); //低电压保护
        RampAdjHandler();			
		    break;
		//极低亮
    case Mode_ExtremelyLow:					
				BatteryLowAlertProcess(true,Mode_ExtremelyLow);
				DetectIfNeedsOFF(ClickCount); //执行关机动作检测
		    EnterTurboStrobe(ClickCount); //进入极亮或者爆闪的检测
				SideKeySwitchGearHandler(Mode_Low); //换到低档
		    break;	    		
    //低亮状态		
    case Mode_Low:
			  BatteryLowAlertProcess(false,Mode_ExtremelyLow);
		    DetectIfNeedsOFF(ClickCount); //执行关机动作检测
				EnterTurboStrobe(ClickCount); //进入极亮或者爆闪的检测
		    //长按换挡处理
				SideKey1HRevGearHandler(Mode_ExtremelyLow); //单击+长按回退挡位到极低档
		    SideKeySwitchGearHandler(Mode_Mid); //换到中档
		    break;	    		
    //中亮状态		
    case Mode_Mid:
			  BatteryLowAlertProcess(false,Mode_Low);
		    DetectIfNeedsOFF(ClickCount); //执行关机动作检测
				EnterTurboStrobe(ClickCount); //进入极亮或者爆闪的检测
		    //长按换挡处理
		    SideKeySwitchGearHandler(Mode_MHigh); //换到中高档
		    SideKey1HRevGearHandler(Mode_Low); //单击+长按回退挡位到低档
		    break;	
	  //中高亮状态
    case Mode_MHigh:
			  BatteryLowAlertProcess(false,Mode_Mid);
		    DetectIfNeedsOFF(ClickCount); //执行关机动作检测
				EnterTurboStrobe(ClickCount); //进入极亮或者爆闪的检测
		    //长按换挡处理
		    SideKeySwitchGearHandler(Mode_High); //换到高档
		    SideKey1HRevGearHandler(Mode_Mid); //单击+长按回退挡位到中档
		    break;	
	  //高亮状态
    case Mode_High:
			  BatteryLowAlertProcess(false,Mode_MHigh);
		    DetectIfNeedsOFF(ClickCount); //执行关机动作检测
				EnterTurboStrobe(ClickCount); //进入极亮或者爆闪的检测
		    //长按换挡处理
		    SideKeySwitchGearHandler(Mode_ExtremelyLow); //换到极低档位构成循环  
		    SideKey1HRevGearHandler(Mode_MHigh); //单击+长按回退挡位到中高档
		    break;
		//极亮状态
    case Mode_Turbo:
				TurboLVILIMProcess(); //执行极亮低电流检测
		    DetectIfNeedsOFF(ClickCount); //执行关机动作检测
		    SideKeySwitchGearHandler(Mode_High); //长按退回高档 
			  if(ClickCount==2||IsForceLeaveTurbo)SwitchToGear(IsRampEnabled?Mode_Ramp:Mode_Low); //双击或者温度达到上限值，强制返回到低亮
				if(ClickCount==3)SwitchToGear(Mode_Strobe); //侧按3击进入爆闪
		    break;	
		//爆闪状态
    case Mode_Strobe:
			  BatteryLowAlertProcess(true,Mode_Strobe);
		    DetectIfNeedsOFF(ClickCount); //执行关机动作检测
		    LeaveSpecialMode(ClickCount); //退出特殊模式回到其他地方的入口
		    //长按换挡处理
		    SideKeySwitchGearHandler(Mode_SOS); //长按切换到SOS
		    break;	
    //SOS求救挡位		
		case Mode_SOS:
			  BatteryLowAlertProcess(true,Mode_SOS);
		    DetectIfNeedsOFF(ClickCount); //执行关机动作检测
			  LeaveSpecialMode(ClickCount); //退出特殊模式回到其他地方的入口
		    //长按换挡处理
		    SideKeySwitchGearHandler(Mode_Beacon); //长按切换到信标
		    break;	
		//信标挡位
		case Mode_Beacon:
			  BatteryLowAlertProcess(true,Mode_Beacon);
		    DetectIfNeedsOFF(ClickCount); //执行关机动作检测
			  LeaveSpecialMode(ClickCount); //退出特殊模式回到其他地方的入口
		    //长按换挡处理
		    SideKeySwitchGearHandler(Mode_Strobe); //长按切换到爆闪
		    break;				
		}
  //应用输出电流
	if(DisplayLockedTIM||IsDisplayLocked)Current=CalcIREFValue(50); //用户进入或者退出锁定，用50mA短暂点亮提示一下
	else switch(CurrentMode->ModeIdx)	
		{
		case Mode_Turbo:Current=QueryTurboCurrent();break; //极亮模式
		case Mode_Beacon: //信标模式			
		case Mode_SOS: 
		case Mode_Strobe://爆闪模式和SOS模式	     
	     switch(BattState)//取出挡位电流
				 {
				 case Battery_Plenty:Current=QueryCurrentGearILED();break;
			   case Battery_Mid:Current=CalcIREFValue(10000);break;
         case Battery_Low:Current=CalcIREFValue(8000);break;
				 case Battery_VeryLow:Current=CalcIREFValue(2000);break;
				 }
			 //根据状态控制电流
			 if(CurrentMode->ModeIdx==Mode_Strobe&&!StrobeOutputHandler())Current=-1; 
			 if(CurrentMode->ModeIdx==Mode_SOS&&!SOSFSM())Current=-1;
			 if(CurrentMode->ModeIdx==Mode_Beacon)switch(BeaconFSM())
				 {
				 case 0:Current=-1;break; //0表示让电流关闭
				 case 2:Current=CalcIREFValue(200);break; //用200mA低亮提示告知用户已进入信标模式
				 case 1:break; //电流1不进行任何处理
				 }
		   break; 
		//其余模式，电流取正常值
		default:
		  if(LowPowerStrobe())Current=-1; //触发低压报警，闪烁
			else if(CurrentMode->ModeIdx==Mode_Ramp)Current=SysCfg.RampCurrentLimit<SysCfg.RampCurrent?SysCfg.RampCurrentLimit:SysCfg.RampCurrent; //无极调光模式取结构体内数据
		  else Current=QueryCurrentGearILED();//其他挡位使用设置值作为目标电流
		}
  //无极调光模式指示(无极调光模式在抵达上下限后短暂熄灭或者调到33%)
	if(SysCfg.RampLimitReachDisplayTIM)Current=IsNotifyMaxRampLimitReached?Current/3:-1;
	//清除按键处理
	getSideKeyShortPressCount(1); 
	}