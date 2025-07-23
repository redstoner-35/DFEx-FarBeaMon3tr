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
#include "TurboICCMAX.h"
#include "SetupMenu.h"
#include "VersionCheck.h"

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
		//配置是否允许进入爆闪
		true,
		//低电量保护设置
		Mode_OFF,							 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Disable,        //低电量保护机制的类型
		//挡位切换设置
		Mode_OFF,
		Mode_OFF	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 
		//出错了
		{
		Mode_Fault,
		0,
		0,  //电流0mA
		0,
		false,
		false,
		//配置是否允许进入爆闪
		false,
		//低电量保护设置
		Mode_OFF,							 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Disable,        //低电量保护机制的类型
		//挡位切换设置
		Mode_OFF,
		Mode_OFF	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 
		//月光
		{
		Mode_Moon,
		CalcIREFValue(25),  //实际是20
		0,   //最小电流没用到，无视
		2750,  //2.75V关断
		false, //月光档有专用入口，无需带记忆
		false,
		//配置是否允许进入爆闪
		false,
		//低电量保护设置
		Mode_Moon,							 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_OFF,        //低电量保护机制的类型
		//挡位切换设置
		Mode_OFF,
		Mode_1Lumen,	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 	
		//1流明挡位
		{
		Mode_1Lumen,
		2,  //电流随便填的
		0,   //最小电流没用到，无视
		2500,  //2.5V关断（1流明没有保护）
		false, //1流明档有专用入口，无需带记忆
		false,	
		//配置是否允许进入爆闪
		false,
		//低电量保护设置
		Mode_1Lumen,							 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_OFF,        //低电量保护机制的类型
		//挡位切换设置
		Mode_Moon,
		Mode_OFF	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)			
		},
		//极低亮
		{
		Mode_ExtremelyLow,
		CalcIREFValue(200),  //200mA
		0,   //最小电流没用到，无视
		2850,  //2.85V关断
		true, //带记忆
		false,
		//配置是否允许进入爆闪
		true,
		//低电量保护设置
		Mode_ExtremelyLow,							 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_OFF,        //低电量保护机制的类型
		//挡位切换设置
		Mode_Low,
		Mode_OFF	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 	
    //低亮
		{
		Mode_Low,
		CalcIREFValue(1000),  //1000mA电流
		0,   //最小电流没用到，无视
		2950,  //2.8V关断
		true,
		false,
		//配置是否允许进入爆闪
		true,
		//低电量保护设置
		Mode_ExtremelyLow,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_Jump,        //低电量保护机制的类型
		//挡位切换设置
		Mode_Mid,
		Mode_ExtremelyLow	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		},
    //中亮
		{
		Mode_Mid,
		CalcIREFValue(2000),  //2000mA电流
		0,   //最小电流没用到，无视
		3050,  //3.0V关断
		true,
		false,
		//配置是否允许进入爆闪
		true,
		//低电量保护设置
		Mode_Low,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_Jump,        //低电量保护机制的类型
		//挡位切换设置
		Mode_MHigh,
		Mode_Low	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 	
    //中高亮
		{
		Mode_MHigh,
		CalcIREFValue(4000),  //4000mA电流
		0,   //最小电流没用到，无视
		3150,  //3.1V关断
		true,
		true,
		//配置是否允许进入爆闪
		true,
		//低电量保护设置
		Mode_Mid,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_Jump,        //低电量保护机制的类型
		//挡位切换设置
		Mode_High,
		Mode_Mid	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 	
    //高亮
		{
		Mode_High,
		CalcIREFValue(8000),  //8000mA电流
		0,   //最小电流没用到，无视
		3250,  //3.2V关断
		true,
		true,
		//配置是否允许进入爆闪
		true,
		//低电量保护设置
		Mode_MHigh,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_Jump,        //低电量保护机制的类型
		//挡位切换设置
		Mode_ExtremelyLow,
		Mode_MHigh	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 	
    //极亮
		{
		Mode_Turbo,
		CalcIREFValue(TurboICCMAX),  //30A电流	
		0,   //最小电流没用到，无视
		3400,  //3.4V关断
		false, //极亮不能带记忆
		true,
		//配置是否允许进入爆闪
		false,
		//低电量保护设置
		Mode_Turbo,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Disable,        //低电量保护机制的类型
		//挡位切换设置
		Mode_OFF,
		Mode_High	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 	
    //爆闪		
		{
		Mode_Strobe,
		CalcIREFValue(StrobeICCMAX),		
		0,   //最小电流没用到，无视
		2750,  //2.75V关断
		false, //爆闪不能带记忆
		true,
		//配置是否允许进入爆闪
		false,
		//低电量保护设置
		Mode_Strobe,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_OFF,        //低电量保护机制的类型
		//挡位切换设置
		Mode_SOS,
		Mode_Beacon	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 
	  //无极调光		
		{
		Mode_Ramp,
		CalcIREFValue(10000),  //最大 10000mA电流
		CalcIREFValue(100),   //最小 100mA电流
		3200,  //3.2V关断
		false, //不能带记忆  
		true,
		//配置是否允许进入爆闪
		true,
		//低电量保护设置
		Mode_Ramp,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Disable,        //低电量保护机制的类型
		//挡位切换设置
		Mode_OFF,
		Mode_OFF	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 
		//信标模式
		{
		Mode_Beacon,
		CalcIREFValue(BeaconICCMAX),
		0,   //最小电流没用到，无视
		2750,  //2.75V关断
		false,	//SOS不能带记忆
		true,
		//配置是否允许进入爆闪
		false,
		//低电量保护设置
		Mode_Beacon,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_OFF,        //低电量保护机制的类型
		//挡位切换设置
		Mode_Strobe,
		Mode_SOS	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 
	  //SOS
		{
		Mode_SOS,
		CalcIREFValue(14000),  //14A电流
		0,   //最小电流没用到，无视
		2750,  //2.75V关断
		false,	//SOS不能带记忆
		true,
		//配置是否允许进入爆闪
		false,
		//低电量保护设置
		Mode_SOS,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_OFF,        //低电量保护机制的类型
		//挡位切换设置
		Mode_Beacon,
		Mode_Strobe	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 
	};

//全局变量(挡位)
ModeStrDef *CurrentMode; //挡位结构体指针
xdata ModeIdxDef LastMode; //挡位记忆存储
xdata ModeIdxDef LastSpecialMode; //特殊功能挡位存储
SysConfigDef SysCfg; //系统配置	

//全局变量(状态位)
bit IsRampEnabled; //是否开启无极调光
bit IsMainMemEnabled; //是否开启主挡位记忆
bit IsSpecMemEnabled; //是否开启特殊挡位记忆
bit IsStrobePoweredFromOFF; //是否为关机模式下进入到一键爆闪
bit IsPowerModeEnabled; //0=ECO MODE 1=POWER MODE	
bit IsRampFault; //无极调光故障，当该bit置起后无极调光将会强制禁用	
	
//全局软件计时变量
xdata unsigned char HoldChangeGearTIM=0; //挡位模式下长按换挡
xdata unsigned char DisplayLockedTIM; //锁定和战术模式进入退出显示

//内部变量和标志位
static xdata unsigned char RampDIVCNT; //无极调光降低调光速度的分频计时器		
static bit IsRampKeyPressed;  //标志位，用户是否按下按键对无极调光进行调节
static bit IsNotifyMaxRampLimitReached; //标记无极调光达到最大电流	
static bit IsSlowFading; //关机渐暗特效
static bit IsSwitchingKeyStillHold; //按键是否仍然按住
	
//获取系统挡位在没有任何外部影响情况下的全部电流
int QuerySystemFullScaleCurrent(void)
	{
	//极亮且开启ECO模式，电流按照ECO模式的ICCMAX取
	if(CurrentMode->ModeIdx==Mode_Turbo&&IsPowerModeEnabled)
		return CalcIREFValue(ECOTurboICCMAX);
	//其他情况按照极亮当前电流取
  return QueryCurrentGearILED();	
	}	
	
//输入指定的Index，从index里面找到目标模式结构体并返回指针
ModeStrDef *FindTargetMode(ModeIdxDef Mode,bool *IsResultOK)
	{
	unsigned char i;
	*IsResultOK=false;
	for(i=0;i<ModeTotalDepth;i++)if(ModeSettings[i].ModeIdx==Mode)
		{
		*IsResultOK=true;
		break;
		}
	//返回对应的index
	return &ModeSettings[i];
	}
	
//初始化模式状态机
void ModeFSMInit(void)
{
	bool Result;
	//初始化无极调光
	SysCfg.RampLimitReachDisplayTIM=0;
  ReadSysConfig(); //从EEPROM内读取无极调光配置
	
	CurrentMode=FindTargetMode(Mode_Ramp,&Result);//遍历挡位设置结构体寻找无极调光的挡位并读取配置
	if(Result)
		{
		SysCfg.RampBattThres=CurrentMode->LowVoltThres; //低压检测上限恢复
		SysCfg.RampCurrentLimit=CurrentMode->Current; //找到挡位数据中无极调光的挡位，电流上限恢复
		if(SysCfg.RampCurrent<CurrentMode->MinCurrent)SysCfg.RampCurrent=CurrentMode->MinCurrent;
		if(SysCfg.RampCurrent>SysCfg.RampCurrentLimit)SysCfg.RampCurrent=SysCfg.RampCurrentLimit;		//读取数据结束后，检查读入的数据是否合法，不合法就直接修正
		//无级调光挡位正常
		IsRampFault=0;	
		}
	//无法找到无极调光数值，禁止无极调光功能
	else 
		{
		IsRampEnabled=0;
		IsRampFault=1;	
		LEDMode=LED_RedBlinkFifth; //触发LED提示
		}
	//复位变量和一部分模块
	IsSlowFading=0;
	IsRampKeyPressed=0;
	SetupFSMState=SetupMenu_InACT;
	ResetStrobeModule(); 											//复位爆闪控制器
	RampDIVCNT=RampAdjustDividingFactor; 			//复位分频计数器
	//挡位模式配置
	ResetSOSModule(); //复位SOS模块
	LastMode=Mode_ExtremelyLow;
	LastSpecialMode=Mode_Strobe;
	ErrCode=Fault_None; //没有故障
	CurrentMode=&ModeSettings[0]; //记忆重置为第一个档
}	

//挡位状态机所需的软件定时器处理
void ModeFSMTIMHandler(void)
{
	//无极调光相关的定时器
	if(IsLargerThanOneU8(SysCfg.CfgSavedTIM))SysCfg.CfgSavedTIM--;
	if(SysCfg.RampLimitReachDisplayTIM)
		{
		SysCfg.RampLimitReachDisplayTIM--;
		if(!SysCfg.RampLimitReachDisplayTIM)IsNotifyMaxRampLimitReached=0;
		}
	//锁定操作提示计时器
  if(DisplayLockedTIM)DisplayLockedTIM--;
}

//挡位跳转
void SwitchToGear(ModeIdxDef TargetMode)
	{
	bool IsLastModeNeedStepDown,Result;
	ModeStrDef *ModeBuf;
	//当前挡位已经是目标值，不执行
	if(IsRampFault&&TargetMode==Mode_Ramp)return;  //无极调光异常，禁止换到无极调光模式
	if(TargetMode==CurrentMode->ModeIdx)return;
	//记录换档前的结果	
	IsLastModeNeedStepDown=CurrentMode->IsNeedStepDown; //存下是否需要降档
	//开始寻找
	ModeBuf=FindTargetMode(TargetMode,&Result);
	if(!Result)return;                    //找不到对应的挡位，退出
	//应用挡位结果并重新计算极亮电流	
	CurrentMode=ModeBuf;	
	CalcTurboILIM();
	//复位特殊功能挡位至初始状态
	ResetSOSModule();						//复位整个SOS模块
	BeaconFSM_Reset(); 					//复位整个信标模块	
			
	//如果新老挡位都是常亮挡，则重新设置PI环避免电流过调
	if(TargetMode>1&&TargetMode<11&&IsLastModeNeedStepDown)RecalcPILoop(Current); 	
	}
	
//特殊功能挡位独立记忆的处理函数
static void SpecialModeMemoryHandler(void)	
	{
	//关闭特殊挡位记忆
	if(!IsSpecMemEnabled)return;
	//关机状态下三击爆闪，不进行记忆，立即清除flag
	if(IsStrobePoweredFromOFF)IsStrobePoweredFromOFF=0;
	//非关机状态下一键爆闪和战术模式，存储下离开或者关机之前的特殊功能挡位
	else if(!SysMode)LastSpecialMode=CurrentMode->ModeIdx;
	}

//长按关机函数	
void ReturnToOFFState(void)
	{
	switch(CurrentMode->ModeIdx)
		{
		case Mode_Fault:
		case Mode_OFF:return;  //非法状态，直接打断整个函数的执行
		case Mode_Beacon:
		case Mode_Strobe:	//特殊挡位执行记忆函数，且不启用慢速关闭
		case Mode_SOS:
			  SpecialModeMemoryHandler();
			  break; 

		//其余挡位如果非特殊模式，则执行判断
		default:
			if(SysMode)break;
			if(CurrentMode->IsNeedStepDown)Current=CurrentBuf; //如果当前挡位需要温控，则在关机的时候直接取目前已执行的电流结果
			IsSlowFading=1;	//非战术模式的常亮挡位触发渐暗特效
		}
  //执行挡位记忆并跳回到关机状态
	if(IsMainMemEnabled) //挡位记忆开启
		{
		//该挡位有记忆，存下关机前的状态
		if(CurrentMode->IsModeHasMemory)LastMode=CurrentMode->ModeIdx;
		}
	//无级调光模式下关闭挡位记忆，强制恢复到初始电流
	else if(IsRampEnabled)
		{
		LoadMinimumRampCurrentToRAM();
		SaveSysConfig(0);                //保存一遍配置，确保写入到EEPROM里面的数据一定是最低电流
		}
	//执行关机处理，强制跳回到关机挡位
	SwitchToGear(Mode_OFF); 
	}	
	
//长按换挡的间隔命令生成
void HoldSwitchGearCmdHandler(void)
	{
	char buf;
	if(SysMode||(!getSideKeyHoldEvent()&&!getSideKey1HEvent()))//按键松开或者系统处在非正常状态，计时器和Flag复位
		{
		IsSwitchingKeyStillHold=0;
		HoldChangeGearTIM=0; 
		}
	else //执行换挡程序
		{
		buf=HoldChangeGearTIM&0x1F; //取出TIM值
		if(!buf&&!(HoldChangeGearTIM&0x40))//令换挡命令位1指示换挡可以继续
			{
			IsSwitchingKeyStillHold=1;
			HoldChangeGearTIM|=getSideKey1HEvent()?0x20:0x80; 
			}
		HoldChangeGearTIM&=0xE0; //去除掉原始的TIM值
		if(buf<HoldSwitchDelay&&!(HoldChangeGearTIM&0x40))buf++;
		else buf=0;  //时间到，清零结果
		HoldChangeGearTIM|=buf; //把数值写回去
		}
	}	

//无极调光处理
static void RampAdjHandler(void)
	{	
  int Limit;
	bit IsPress;
  //计算出无极调光上限
	IsPress=getSideKey1HEvent()|getSideKeyHoldEvent();
	Limit=SysCfg.RampCurrentLimit<CurrentMode->Current?SysCfg.RampCurrentLimit:CurrentMode->Current;
	if(Limit<CurrentMode->Current&&IsPress&&SysCfg.RampCurrent>Limit)SysCfg.RampCurrent=Limit; //在电流被限制的情况下用户按下按键尝试调整电流，立即限幅
	//进行亮度调整
	if(getSideKeyHoldEvent()&&!IsRampKeyPressed) //长按增加电流
			{	
			if(RampDIVCNT)RampDIVCNT--;
			else 
				{
				//时间到，开始增加电流
				if(SysCfg.RampCurrent<Limit)SysCfg.RampCurrent++;
				else
					{
					IsNotifyMaxRampLimitReached=1; //标记已达到上限
					SysCfg.RampLimitReachDisplayTIM=4; //熄灭0.5秒指示已经到上限
					SysCfg.RampCurrent=Limit; //限制电流最大值	
					IsRampKeyPressed=1;
					}
				//计时时间到，复位变量
				RampDIVCNT=RampAdjustDividingFactor;
				}
			}	
	else if(getSideKey1HEvent()&&!IsRampKeyPressed) //单击+长按减少电流
		 {
			if(RampDIVCNT)RampDIVCNT--;
			else
				{
				if(SysCfg.RampCurrent>CurrentMode->MinCurrent)SysCfg.RampCurrent--; //减少电流	
				else
					{
					IsNotifyMaxRampLimitReached=0;
					SysCfg.RampLimitReachDisplayTIM=4; //熄灭0.5秒指示已经到下限
					SysCfg.RampCurrent=CurrentMode->MinCurrent; //限制电流最小值
					IsRampKeyPressed=1;
					}
				//计时时间到，复位变量
				RampDIVCNT=RampAdjustDividingFactor;
				}
		 }
  else if(!IsPress&&IsRampKeyPressed)
		{
	  IsRampKeyPressed=0; //用户放开按键，允许调节		
		RampDIVCNT=RampAdjustDividingFactor; //复位分频计时器
		}
	//进行数据保存的判断
	if(IsPress)SysCfg.CfgSavedTIM=32; //按键按下说明正在调整，复位计时器
	else if(SysCfg.CfgSavedTIM==1)
		{
		SysCfg.CfgSavedTIM--;
		if(IsMainMemEnabled)SaveSysConfig(0);  //一段时间内没操作说明已经调节完毕，保存数据
		}
	}
//进行关机和开机状态执行N击+长按事件处理的函数
static void ProcessNClickAndHoldHandler(void)
	{
  //正常执行处理
	switch(getSideKeyNClickAndHoldEvent())
		{
		case 1:	//单击+长按进入1流明挡位(仅系统处于关机后)
			if(CurrentMode->ModeIdx!=Mode_OFF)break;
			SwitchToGear(Mode_1Lumen);
			break; 
		case 2:TriggerVshowDisplay();break; //双击+长按查询电量
		case 3:TriggerTShowDisplay();break;//三击+长按查询温度
		//其余情况什么都不做
		default:break;			
		}
	}	
	
//进行模式状态机的表驱动模块处理	
static void ModeSwitchFSMTableDriver(char ClickCount)
	{
	if(CurrentMode->IsEnterTurboStrobe)TryEnterTurboStrobeProcess(ClickCount);//读取当前的模式结构体，执行进入极亮或者爆闪的检测	
  if(IsLargerThanOneU8(CurrentMode->ModeIdx)) //大于1的比较									
		{
		//系统在开机状态，且标志位无效之后则执行电量显示启动检测
		ProcessNClickAndHoldHandler();
		//侧按单击或者在战术模式下松开按钮时关机	
		if(!SysMode)
				{
				//非战术模式单击关闭
				if(ClickCount==1)ReturnToOFFState();
				}
		else if(!getSideKeyHoldEvent())ReturnToOFFState();
		}			
	
 	if(HoldChangeGearTIM&0x80)	 
		{
		//当挡位数据库内的状态表使能长按换挡功能且条件满足时，执行顺向换挡
		HoldChangeGearTIM&=0x7F; 
		if(CurrentMode->ModeTargetWhenH!=Mode_OFF)SwitchToGear(CurrentMode->ModeTargetWhenH); 		
		}
	
	if(HoldChangeGearTIM&0x20)  
		{
		//当挡位数据库内的状态表使能单击+长按换挡功能且条件满足时，执行逆向换挡
		HoldChangeGearTIM&=0xDF; 
		if(CurrentMode->ModeTargetWhen1H!=Mode_OFF)SwitchToGear(CurrentMode->ModeTargetWhen1H); 
		}
	
	if(CurrentMode->LVConfig)BatteryLowAlertProcess(CurrentMode->LVConfig&0x02,CurrentMode->ModeWhenLVAutoFall); //执行低电量处理
	}	
	
//特殊挡位电流处理
static void SpecialModeCurrentFetch(void)
	{
	switch(BattState)//取出挡位电流
			{
			case Battery_Plenty:
			case Battery_Mid:Current=QueryCurrentGearILED();break;
      case Battery_Low:Current=CalcIREFValue(10000);break;
			case Battery_VeryLow:Current=CalcIREFValue(4000);break;
			}
	}
	
//挡位状态机
void ModeSwitchFSM(void)
	{
	char ClickCount;
	ModeIdxDef ModeBeforeFSMSwitch;
	//获取按键状态
	if(GetIfSystemInPOFFSeq())return; //系统处于关机过程中，不执行按键处理
	ClickCount=getSideKeyShortPressCount();	//读取按键处理函数传过来的参数
		
	//挡位记忆参数检查
	if(IsRampFault)IsRampEnabled=0;  //无极调光数据故障，禁止无极调光功能
	if(LastSpecialMode<11||LastSpecialMode>13)LastSpecialMode=Mode_Strobe;        //特殊功能
	if(LastMode<2||LastMode>13)LastMode=Mode_ExtremelyLow;									//全局常规记忆
		
	//处理FSM的特殊逻辑部分		
  ModeBeforeFSMSwitch=CurrentMode->ModeIdx;		 //存下进入之前的挡位
	IsHalfBrightness=0; //按键灯默认全亮
	if(VChkFSMState==VersionCheck_InAct)switch(ModeBeforeFSMSwitch)	
		{
		//关机状态
		case Mode_OFF:		  
			//处理特殊功能和定位LED和其他设置的变更(在变更和特殊模式下拒绝执行其他内容)
		  if(SetupFSMState!=SetupMenu_InACT||LocateLED_Edit())break;
			else if(SpecialModeOperation(ClickCount)!=Operation_Normal)break;
		  
		  //非特殊模式正常单击开关机，执行一键极亮，爆闪和转换无极调光
			switch(ClickCount)
				{
				case 1:
					//侧按单击开机，进入循环挡位上一次关闭的模式（仅在开启了记忆的条件下）
					PowerToNormalMode(!IsMainMemEnabled?Mode_ExtremelyLow:LastMode);
					break; 	
				case 7:
					//7击进入设置菜单
					TriggerSetupMenuDisplay();
				  break;
				case 8:
					//8击触发版本查询
					VersionCheck_Trigger();
				  break;
				//其余情况什么都不做
        default:break;				
				}
			//长按开机进入月光挡位	
      if(getSideKeyLongPressEvent())EnterMoonProcess();				
		  //N击+长按查询电压，温度和进入1流明挡位
			ProcessNClickAndHoldHandler();
  		break;
		//出现错误	
		case Mode_Fault:
      SysMode=Operation_Normal; //故障后自动回到普通模式	
		  if(IsErrorFatal())
				{
				//电池已经耗尽，强制关闭并且禁止开机
				if(IsBatteryFault)IsDisplayLocked=0;
				//在NTC故障状态下可以应急开机使用，但是锁50mA，不准换挡允许用户应急使用
				else if(ErrCode==Fault_NTCFailed&&ClickCount)IsDisplayLocked=~IsDisplayLocked;
				}				 
			//非致命错误状态用户按下按钮清除错误，清除后特殊功能模块会让主灯熄灭
			else if(getSideKeyLongPressEvent())ClearError();
		  break;
		 //月光状态
		 case Mode_Moon:
			 //电池电压充足，长按进入低亮挡位
		   if(!IsSwitchingKeyStillHold&&getSideKeyLongPressEvent())  
					{
					PowerToNormalMode(Mode_ExtremelyLow); //开机到极低亮模式
					if(CurrentMode->ModeIdx==Mode_Moon)break;//换挡之后无法成功离开月光模式，不进行下面的复位操作
					if(IsRampEnabled)LoadMinimumRampCurrentToRAM(); //如果是无极调光则恢复到最低电流
					HoldChangeGearTIM|=0x40; //禁止换挡系统工作
					}		    	
		//1流明挡位						
	  case Mode_1Lumen:
			 /***********************************************
		   月光和1LM模式按键灯亮度减半
		   （这里利用了switch语句的shoot through特性，执行
			 正常月光之后没有break所以会往下走跳到1LM的位置
			 执行设置按键灯亮度一半的处理）
		   ***********************************************/
			 IsHalfBrightness=1; 
		   if(Battery<2.4)ReturnToOFFState();   //单节电池电压小于2.4之后DCDC可能工作异常，强制断电
			 break;				
    //无极调光状态				
    case Mode_Ramp:
		    RampLowVoltHandler(); 				//低电压保护
        RampAdjHandler();					    //无极调光处理
		    break;
		//极亮状态
    case Mode_Turbo:
				TurboLVILIMProcess(); //执行极亮低电流检测
			  if(ClickCount==2||IsForceLeaveTurbo)PowerToNormalMode(Mode_Low); //双击或者温度达到上限值，强制返回到低亮
				if(ClickCount==3)SwitchToGear(LastSpecialMode); //侧按3击进入上次关闭的特殊功能组
		    break;	
		//特殊功能挡位（爆闪、SOS、信标）执行退出检测
    case Mode_Strobe:		
		case Mode_SOS:
		case Mode_Beacon:
			  //开机状态下二击或者三击执行退出挡位记忆判断
			  if(IsLargerThanOneU8(ClickCount)&&ClickCount<4)SpecialModeMemoryHandler();
		    //执行实际的特殊模式退出操作
				if(ClickCount==3)PowerToNormalMode(LastMode); //三击调用退回函数，退回到普通模式
				else TryEnterTurboStrobeProcess(ClickCount); //其他按键次数，直接call尝试极亮函数让他自己判断去
		    break;				
		}
		
	//处理FSM中的表驱动部分
	if(ModeBeforeFSMSwitch==CurrentMode->ModeIdx&&VChkFSMState==VersionCheck_InAct)
		{
		//如果状态机FSM内有操作或者当前处于版本检查状态则跳过表驱动，否则执行表驱动
		ModeSwitchFSMTableDriver(ClickCount); 
		}
	ClearShortPressEvent(); //表驱动事项响应完毕，清除按键状态

  //应用输出电流
	if(VChkFSMState!=VersionCheck_InAct)Current=VersionCheckFSM()?CalcIREFValue(50):-1;
	else if(DisplayLockedTIM||IsDisplayLocked)Current=CalcIREFValue(50); //用户进入或者退出锁定，用50mA短暂点亮提示一下
	else switch(CurrentMode->ModeIdx)	
		{ 
		//极亮模式
    case Mode_Turbo:
		 Current=QuerySystemFullScaleCurrent();  //ECO模式开启时使用ECO电流，否则使用极亮电流
     if(TurboILIM<Current)Current=TurboILIM; //应用限流设置
		 break;
		//信标模式
		case Mode_Beacon:
			switch(BeaconFSM())
				 {
				 case 0:Current=-1;break; //0表示让电流关闭
				 case 2:Current=CalcIREFValue(200);break; //用200mA低亮提示告知用户已进入信标模式
				 default:SpecialModeCurrentFetch(); //其他值调用系统默认电流
				 } 			
			 break;
		//SOS模式	
		case Mode_SOS: 
			 if(!SOSFSM())Current=-1;
			 else SpecialModeCurrentFetch();
			 break;
	  //爆闪模式
		case Mode_Strobe:     
       if(!StrobeOutputHandler())Current=-1; 
		   else SpecialModeCurrentFetch();
		   break;
	  //关机状态下电流缓降
		case Mode_OFF:	
       //关机函数没有使能该功能或者拖尾被用户禁用，电流直接到0			
       if(!SysCfg.FadingCfg||!IsSlowFading)Current=0; 
		   //逐渐变暗特效开启，缓慢减少电流
			 else if(Current>CalcIREFValue(20))Current-=1+(Current/(((int)SysCfg.FadingCfg)*430));
 		   else IsSlowFading=0;		//特效结束，清除flag并使得电流置零
			 break;
		//其余模式，电流取正常值
		default:
		  if(LowPowerStrobe())Current=-1; //触发低压报警，闪烁
			else if(CurrentMode->ModeIdx==Mode_Ramp)
				{
				//无极调光模式取结构体内数据
				if(SysCfg.RampCurrent>SysCfg.RampCurrentLimit)Current=SysCfg.RampCurrentLimit;
				else Current=SysCfg.RampCurrent;
				}
		  else Current=QueryCurrentGearILED();//其他挡位使用设置值作为目标电流
		}				
	//无极调光模式指示(无极调光模式在抵达上下限后短暂熄灭或者调到25%)
	if(SysCfg.RampLimitReachDisplayTIM)Current=IsNotifyMaxRampLimitReached?Current>>2:-1;
	}
