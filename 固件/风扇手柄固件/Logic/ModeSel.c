#include "ModeSel.h"
#include "OutputChannel.h"
#include "BattDisplay.h"
#include "SideKey.h"
#include "LEDMgmt.h"
#include "FastOp.h"
#include "PWMCfg.h"
#include "LowVoltProt.h"

code ModeStrDef ModeSettings[ModeTotalDepth]=
	{
		{
		//关机状态
		Mode_OFF,
		0, //低电压检测电压(mV)
		0, //风扇速度百分比(%)
		0, //目标的输出电压(仅CV模式有效)		
		//低电量保护设置
		Mode_OFF,		//低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Disable,        //低电量保护机制的类型
		//挡位切换设置
		Mode_OFF,
		Mode_OFF,	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位
		},
		{
		//超低挡位
		Mode_UltraLow,
		2850, //低电压检测电压(mV)
		12, //风扇速度百分比(%)
		5.55, //目标的输出电压(仅CV模式有效)		
		//低电量保护设置
		Mode_OFF,		//低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_OFF,        //低电量保护机制的类型
		//挡位切换设置
		Mode_Low,
		Mode_OFF,	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位
		},	
		{
		//低挡位
		Mode_Low,
		2950, //低电压检测电压(mV)
		25, //风扇速度百分比(%)
		6.80, //目标的输出电压(仅CV模式有效)		
		//低电量保护设置
		Mode_UltraLow,		//低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_Jump,        //低电量保护机制的类型
		//挡位切换设置
		Mode_Mid,
		Mode_UltraLow,	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位
		},	
		{
		//中挡位
		Mode_Mid,
		3150, //低电压检测电压(mV)
		45, //风扇速度百分比(%)
		8.5, //目标的输出电压(仅CV模式有效)		
		//低电量保护设置
		Mode_Low,		//低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_Jump,        //低电量保护机制的类型
		//挡位切换设置
		Mode_High,
		Mode_Low,	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位
		},	
		{
		//高挡位
		Mode_High,
		3250, //低电压检测电压(mV)
		70, //风扇速度百分比(%)
		9.7, //目标的输出电压(仅CV模式有效)		
		//低电量保护设置
		Mode_Mid,		//低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_Jump,        //低电量保护机制的类型
		//挡位切换设置
		Mode_UltraLow,
		Mode_Mid,	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位
		},
		{
		//脑瓜子嗡嗡挡位
		Mode_Turbo,
		3400, //低电压检测电压(mV)
		100, //风扇速度百分比(%)
		12.0, //目标的输出电压(仅CV模式有效)		
		//低电量保护设置
		Mode_High,		//低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_Jump,        //低电量保护机制的类型
		//挡位切换设置
		Mode_OFF,
		Mode_OFF,	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位
		},
		{
		//无级调节
		Mode_Ramp,
		3000, //低电压检测电压(mV)
		70, //风扇速度百分比(%)
		10, //目标的输出电压(仅CV模式有效)		
		//低电量保护设置
		Mode_Low,		//低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_Jump,        //低电量保护机制的类型
		//挡位切换设置
		Mode_OFF,
		Mode_OFF,	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位
		},		
	};
	
//全局变量(挡位)
ModeStrDef *CurrentMode; //挡位结构体指针
xdata ModeIdxDef LastMode; //挡位记忆存储
xdata ModeIdxDef LastModeBeforeturbo; //记忆进入极速之前的挡位	
	
//全局软件计时变量
xdata unsigned char HoldChangeGearTIM; //挡位模式下长按换挡

//内部变量
static xdata TurboTimedStepDownDef TurboCfg; //极速配置
static bit IsSystemLocked;        //系统是否已经锁定
static xdata char DisplayUnlockTIM; //指示系统解锁的计时器	
static xdata float RampVoltage; //无极调速目标电压
static xdata float RampDuty; //无极调速目标占空比
static xdata int RampDivCNT; //无极调速分频计时器	
static bit IsRampKeyStillHold; //无极调速是否一直按住	
	
//初始化模式状态机
void ModeFSMInit(void)
	{
	unsigned char i;
	unsigned long buf;
	//进行变量初始化
	CurrentMode=&ModeSettings[0];
	LastMode=Mode_UltraLow;
	IsSystemLocked=0;
	DisplayUnlockTIM=0;
	RampDivCNT=10;
	IsRampKeyStillHold=0;
	TurboCfg.TurboRefreshCount=MaxTurboRefreshCount;
	TurboCfg.TurboRefreshTIM=8*TurboRefreshCountCD;
	//寻找指定挡位数据载入结果
	for(i=0;i<ModeTotalDepth;i++)
		{
		//超低挡位填充系统下限数据
		if(ModeSettings[i].ModeIdx==Mode_UltraLow)
			{
			VMinMaxCfg.SysMinSpeed=ModeSettings[i].Speed;
			VMinMaxCfg.SysMinVolt=ModeSettings[i].TargetVOUT;
			}
		//极亮挡位填充极亮配置结构体
		if(ModeSettings[i].ModeIdx==Mode_Turbo)
			{
			TurboCfg.TurboCurrentVoltage=ModeSettings[i].TargetVOUT;
			TurboCfg.TurboCurrentDuty=(float)ModeSettings[i].Speed;
			}
		//高亮挡位填充最低占空比配置
		if(ModeSettings[i].ModeIdx==Mode_High)
			{
			TurboCfg.TurboMinimumVoltage=ModeSettings[i].TargetVOUT;
			TurboCfg.TurboMinimumDuty=(float)ModeSettings[i].Speed;			
			}
		}	
	RampVoltage=VMinMaxCfg.SysMinVolt;
	RampDuty=(float)VMinMaxCfg.SysMinSpeed;
	//计算系统启动时的占空比结果
	buf=(unsigned long)VMinMaxCfg.SysMinSpeed;
	buf*=FanPWMStepConstant;
	buf/=100;
	VMinMaxCfg.SysStartUpDuty=(int)(buf&0x7FFF);
	}	
	
//加载极速模式配置（同时令系统进入极速模式）
static void LoadTurboConfig(void)
	{
	//进行进入前记忆判断
	if(CurrentMode->ModeIdx!=Mode_OFF)LastModeBeforeturbo=CurrentMode->ModeIdx; //记下满功率模式进入之前的挡位
	else LastModeBeforeturbo=Mode_UltraLow;
	//加载时控降档跳回挡位的阈值
	if(CurrentMode->ModeIdx==Mode_Ramp)
		{
		//进入极速之前的挡位=无极调速，加载无极调速配置作为最低电流
		TurboCfg.TurboMinimumVoltage=RampVoltage>8.5?RampVoltage:8.6;
		TurboCfg.TurboMinimumDuty=RampDuty>50?RampDuty:50;
		}
	else
		{
		//阶梯挡位模式，临时切换到高档位读取高档位的目标VOUT作为最低电流设置值
		SwitchToGear(Mode_High);
		TurboCfg.TurboMinimumVoltage=CurrentMode->TargetVOUT;
		TurboCfg.TurboMinimumDuty=CurrentMode->Speed;
		}
	//切换到极速档，加载极速挡位的数据
	SwitchToGear(Mode_Turbo);
	TurboCfg.TurboCurrentVoltage=CurrentMode->TargetVOUT;	
	TurboCfg.TurboCurrentDuty=CurrentMode->Speed;
	TurboCfg.FullSpeedTime=TurboMaintainTime*8;
	}
	
//睡眠过程中定时唤醒补充极亮强制刷新次数
void AddTurboRefreshCountWhenSleep(void)
	{
	//如果次数没有达到最大值则增加次数
	if(TurboCfg.TurboRefreshCount<MaxTurboRefreshCount)TurboCfg.TurboRefreshCount++;
	}
	
//极速挡位时控降档处理
void TurboTimedStepDownPROC(void)
	{
	extern bit IsEnablePWMFan;	
	//非极速挡位停止计时并执行刷新程序
	if(CurrentMode->ModeIdx!=Mode_Turbo)
		{
		if(TurboCfg.TurboRefreshCount==MaxTurboRefreshCount)return;
    if(TurboCfg.TurboRefreshTIM)TurboCfg.TurboRefreshTIM--;
		else
			{
			//冷却时间到，极速强制刷新次数+1
			TurboCfg.TurboRefreshTIM=8*TurboRefreshCountCD;
			TurboCfg.TurboRefreshCount++;
			}
	  return;
		}
	//计时器还在进行计时中
	if(TurboCfg.FullSpeedTime)TurboCfg.FullSpeedTime--;
	//电压模式，开始线性降档
	else if(!IsEnablePWMFan)
		{
		if(TurboCfg.TurboCurrentVoltage>TurboCfg.TurboMinimumVoltage)TurboCfg.TurboCurrentVoltage-=0.01;
		else if(LastModeBeforeturbo==Mode_Ramp)
			{
			//挡位减到最低了且极速是从无极调速状态启动的
			TurboCfg.TurboCurrentVoltage=TurboCfg.TurboMinimumVoltage;
			if(RampVoltage<TurboCfg.TurboCurrentVoltage)RampVoltage=TurboCfg.TurboCurrentVoltage; 
			SwitchToGear(Mode_Ramp);
			}
		//从非高亮模式启动的直接跳到高亮
		else SwitchToGear(Mode_High);
		}
	//PWM模式，开始线性减少占空比
	else
		{
		if(TurboCfg.TurboCurrentDuty>TurboCfg.TurboMinimumDuty)TurboCfg.TurboCurrentDuty-=0.075;
		else if(LastModeBeforeturbo==Mode_Ramp)
			{
			TurboCfg.TurboCurrentDuty=TurboCfg.TurboMinimumDuty;
			if(RampDuty<TurboCfg.TurboCurrentDuty)RampDuty=TurboCfg.TurboCurrentDuty;
			SwitchToGear(Mode_Ramp);
			}
		//从非高亮模式启动的直接跳到高亮
		else SwitchToGear(Mode_High);		
		}
	}

//换挡函数
void SwitchToGear(ModeIdxDef TargetMode)
	{
	unsigned char i;
	//要换的挡位等于当前值不执行查找
	if(CurrentMode->ModeIdx==TargetMode)return;
	//进行换挡循环	
	for(i=0;i<ModeTotalDepth;i++)if(ModeSettings[i].ModeIdx==TargetMode)
		{
		CurrentMode=&ModeSettings[i];
		break;
		}
	}	

//长按换挡的间隔命令生成
void HoldSwitchGearCmdHandler(void)
	{
	char buf;
	char SwitchDelay;
	extern bit IsEnablePWMFan;
	//无极调速模式，禁止长按换挡
	if(CurrentMode->ModeIdx==Mode_Ramp)HoldChangeGearTIM=0;
	//按键松开或者系统处在非正常状态，计时器和Flag复位
	else if(!getSideKeyHoldEvent()&&!getSideKey1HEvent())HoldChangeGearTIM=0;
	else //执行换挡程序
		{
		//动态计算换挡延时	
		SwitchDelay=IsEnablePWMFan?PWMHoldSwitchDelay:HoldSwitchDelay;
		//进行换挡处理	
		buf=HoldChangeGearTIM&0x1F; //取出TIM值
		if(!buf&&!(HoldChangeGearTIM&0x40))HoldChangeGearTIM|=getSideKey1HEvent()?0x20:0x80;//令换挡命令位1指示换挡可以继续
		HoldChangeGearTIM&=0xE0; //去除掉原始的TIM值
		if(buf<SwitchDelay&&!(HoldChangeGearTIM&0x40))buf++;
		else buf=0;  //时间到，清零结果
		HoldChangeGearTIM|=buf; //把数值写回去
		}
	//执行定时器处理
	if(DisplayUnlockTIM)DisplayUnlockTIM--;
	}	
	
	
		
//长按关机函数	
void ReturnToOFFState(void)
	{
	//已经关机了
  if(CurrentMode->ModeIdx==Mode_OFF)return;
	//非极速挡位进行记忆处理	
	if(CurrentMode->ModeIdx!=Mode_Turbo&&CurrentMode->ModeIdx!=Mode_Ramp)LastMode=CurrentMode->ModeIdx;
	//执行关机处理，强制跳回到关机挡位
	TargetVoltage=0;
	TargetfanSpeed=0;
	SwitchToGear(Mode_OFF); 
	}	

//查询挡位的目标电池电压
static int QueryModeRequiredBattVolt(ModeIdxDef TargetMode)	
	{
		unsigned char i;
	for(i=0;i<ModeTotalDepth;i++)if(ModeSettings[i].ModeIdx==TargetMode)
		{
		//找到目标挡位了，返回保护电压值
		return ModeSettings[i].LowVoltThres;
		}
	//整个挡位存储区域找遍了都没有，返回0
	return 0;
	}

//进行关机和开机状态执行N击+长按事件处理的函数
static void ProcessNClickAndHoldHandler(void)
	{
  //正常执行处理
	switch(getSideKeyNClickAndHoldEvent())
		{
		case 1:	//单击+长按进入超低速挡位(仅系统处于且电量充足时)
			if(CurrentMode->ModeIdx!=Mode_OFF)break;
			if(CellVoltage>2850)SwitchToGear(Mode_UltraLow);
		  else LEDMode=LED_RedBlinkThird;                  //闪三下表示电量过低无法开机               
			break; 
		case 2:TriggerVshowDisplay();break; //双击+长按查询电量
		case 3:
			//系统已经开启，三击复位结果
			if(CurrentMode->ModeIdx==Mode_Ramp)
				{
				if(IsRampKeyStillHold||CurrentMode->ModeIdx!=Mode_Ramp)break; //非无级调速模式，或者用户还在按着按键，禁止执行重置速度功能
				if(RampDuty==(float)VMinMaxCfg.SysMinSpeed&&RampVoltage==VMinMaxCfg.SysMinVolt)break; //当前速度已被重置，禁止执行重置功能
				IsRampKeyStillHold=1; //标记已被重置
				RampVoltage=VMinMaxCfg.SysMinVolt;
				RampDuty=(float)VMinMaxCfg.SysMinSpeed;      //强制系统回到最低速度
				break;
				}
			//三击+长按进入无级调速且强制进入最低速度（安全开机）
			if(CellVoltage<3050)LEDMode=LED_RedBlinkFifth;   //电池电压不足时禁止进入无级调速
			else 
				{
				//电压正常，进入无级调速并强制以最低速度开机
				SwitchToGear(Mode_Ramp);
				IsRampKeyStillHold=1;
				RampVoltage=VMinMaxCfg.SysMinVolt;
				RampDuty=(float)VMinMaxCfg.SysMinSpeed;      //强制系统回到最低速度
				}
			break;
		//其余情况什么都不做
		default:break;			
		}
	}	

//正向换挡的处理函数
static void PositiveDirChangeGearHandler(void)	
	{
	//当前挡位不支持长按换挡操作
	if(CurrentMode->ModeTargetWhenH==Mode_OFF)return;
	//当前系统的电池电压低于目标要换过去的电压，直接跳到最低构成循环
	if(CellVoltage<QueryModeRequiredBattVolt(CurrentMode->ModeTargetWhenH)+50)
		{
		if(CurrentMode->ModeIdx==Mode_UltraLow)return; //当前已经是最低了还跑到这里，直接退出
		else SwitchToGear(Mode_UltraLow);
		}
	//正常换挡过去
	else SwitchToGear(CurrentMode->ModeTargetWhenH);
	}

//进行模式状态机的表驱动模块处理	
static void ModeSwitchFSMTableDriver(char ClickCount)
	{
	unsigned char buf;
	//非极速挡位进入极速
	if(ClickCount==2&&CurrentMode->ModeIdx!=Mode_Turbo)
		{
		//如果当前挡位电压大于3.4V则进入满功率模式
		if(CellVoltage>3400)LoadTurboConfig();
		//电压大于最低挡位的则进入对应挡位
		else if(CellVoltage>2850)for(buf=Mode_High;buf>Mode_UltraLow;buf--)
			{
			//当前电池电压无法满足对应挡位的要求，继续往下找
      if(CellVoltage<ModeSettings[buf].LowVoltThres)continue;
			SwitchToGear(ModeSettings[buf].ModeIdx);
			break; //找到了，换过去
			}
		//电池电压过低，如果是关机状态则提示电量严重不足禁止开机
		else if(CurrentMode->ModeIdx==Mode_OFF)TriggerBattStatDisplay();
		}
	
	//开机状态进行处理
  if(CurrentMode->ModeIdx!=Mode_OFF)								
		{
		//系统在开机状态，且标志位无效之后则执行电量显示启动检测
		ProcessNClickAndHoldHandler();
		//侧按单击时关机
		if(ClickCount==1)ReturnToOFFState();	
		}			
	
 	if(HoldChangeGearTIM&0x80)	 
		{
		//当挡位数据库内的状态表使能长按换挡功能且条件满足时，执行顺向换挡
		HoldChangeGearTIM&=0x7F; 
		PositiveDirChangeGearHandler();	
		}
	
	if(HoldChangeGearTIM&0x20)  
		{
		//当挡位数据库内的状态表使能单击+长按换挡功能且条件满足时，执行逆向换挡
		HoldChangeGearTIM&=0xDF; 
		if(CurrentMode->ModeTargetWhen1H!=Mode_OFF)SwitchToGear(CurrentMode->ModeTargetWhen1H); 
		}
	
	if(CurrentMode->LVConfig)BatteryLowAlertProcess(CurrentMode->LVConfig&0x02,CurrentMode->ModeWhenLVAutoFall); //执行低电量处理
	}	
	
//无极调速处理流程
static bool RampFSMPROC(void)
	{
	bit IsRampADJ=0;
	extern bit IsEnablePWMFan;
	//用户刚长按进入无极调速，等待按键放开
	if(IsRampKeyStillHold)
		{
		//按键放开后复位flag使无极调速可以正常运行
		if(!getSideKeyHoldEvent()&&!getSideKeyNClickAndHoldEvent())IsRampKeyStillHold=0;
		return false;
		}
	//电池电量低于3.25V，逐步降速到40%
	else if(CellVoltage<3250)
		{
		//电压模式，降低输出电压
		if(!IsEnablePWMFan&&RampVoltage>8.50)
			{
			RampVoltage-=0.05;
			if(RampVoltage<8.50)RampVoltage=8.50; //数值限制
			IsRampADJ=1;
			}
		//PWM模式，降低占空比
		else if(IsEnablePWMFan&&RampDuty>40)
			{
			RampDuty-=0.01;
			if(RampDuty<40)RampDuty=40; //数值限制
			IsRampADJ=1;
			}
		}
	//正向调节
   if(getSideKeyHoldEvent())
		{
		if(RampDivCNT)RampDivCNT--;
		else
			{
			RampDivCNT=IsEnablePWMFan?5:55;
			//电压模式，抬升电压
			if(!IsEnablePWMFan&&RampVoltage<CurrentMode->TargetVOUT)
				{
				RampVoltage+=0.05;
				if(RampVoltage>CurrentMode->TargetVOUT)RampVoltage=CurrentMode->TargetVOUT;
				IsRampADJ=1;
				}
			//PWM模式，抬升占空比
			else if(IsEnablePWMFan&&RampDuty<CurrentMode->Speed)
				{
				RampDuty+=0.010f;
				if(RampDuty>(float)CurrentMode->Speed)RampDuty=(float)CurrentMode->Speed; //数值限制
				IsRampADJ=1;
				}
			}
		}
	//反向调节
	else if(getSideKey1HEvent())
		{
			if(RampDivCNT)RampDivCNT--;
		  else
			{
			RampDivCNT=IsEnablePWMFan?5:55;
			//电压模式，抬升电压
			if(!IsEnablePWMFan&&RampVoltage>VMinMaxCfg.SysMinVolt)
				{
				RampVoltage-=0.05;
				if(RampVoltage<VMinMaxCfg.SysMinVolt)RampVoltage=VMinMaxCfg.SysMinVolt; //数值限制
				IsRampADJ=1;
				}
			//PWM模式，抬升占空比
			else if(IsEnablePWMFan&&RampDuty>VMinMaxCfg.SysMinSpeed)
				{
				RampDuty-=0.010f;
				if(RampDuty<(float)VMinMaxCfg.SysMinSpeed)RampDuty=(float)VMinMaxCfg.SysMinSpeed; //数值限制
				IsRampADJ=1;
				}
			}
		}
	//按键放开，重置计数器
	else RampDivCNT=0;	
	//返回结果
	return IsRampADJ?true:false;
	}
	
//挡位逻辑的实际处理
static bool ModeFSMDrvPROC(char ClickCount)
	{
	ModeIdxDef ModeBeforeFSMSwitch,ModeBuf;
  bool Rampresult=false;
	//处理FSM的特殊逻辑部分
  ModeBeforeFSMSwitch=CurrentMode->ModeIdx;		 //存下进入之前的挡位
	ModeBuf=ModeBeforeFSMSwitch;                 //存下模式组配置记录下FSM操作之前的动作
		
	switch(ModeBeforeFSMSwitch)
		{
		//关机状态的逻辑
		case Mode_OFF:
			//N击+长按查询电压和默认进入最低速度挡位
			ProcessNClickAndHoldHandler();
			//五击锁定
			if(ClickCount==5&&!IsSystemLocked)
				{
				LEDMode=LED_RedBlinkThird; 
				IsSystemLocked=1;
				}
			//长按进入无级调速
			if(getSideKeyLongPressEvent())
				{
				if(IsBatteryFault||CellVoltage<3050)TriggerBattStatDisplay();   //电池电压不足时禁止进入无级调速
				else SwitchToGear(Mode_Ramp);
				IsRampKeyStillHold=1;
				}
			//单击开机回到上一个挡位
			if(ClickCount==1)
				{
			  if(!IsBatteryFault)SwitchToGear(LastMode);
			  else TriggerBattStatDisplay();  //电量不足之后禁止开机，显示异常
				}
			break;
		//无级调速的逻辑处理
		case Mode_Ramp:
		  Rampresult=RampFSMPROC();
		  break;
		//极速挡位的逻辑处理	
		case Mode_Turbo:
			//在极速挡位内再次双击刷新计时器
			if(ClickCount==2&&TurboCfg.TurboRefreshCount&&!TurboCfg.FullSpeedTime)
				{
				TurboCfg.TurboCurrentVoltage=CurrentMode->TargetVOUT;	
				TurboCfg.TurboCurrentDuty=CurrentMode->Speed;
				TurboCfg.FullSpeedTime=TurboMaintainTime*8;
				TurboCfg.TurboRefreshCount--;               //刷新次数-1
				}
			//长按返回极速模式进入前的挡位
			if(HoldChangeGearTIM&0x80)
				{
				SwitchToGear(LastModeBeforeturbo);
				HoldChangeGearTIM&=0x7F;
				HoldChangeGearTIM|=0x40;   //令Stop_GearChange位=1，确保换挡系统只会触发一次
				}
			//单击+长按回退到高速或者无级调速的最高速度
		  if(!(HoldChangeGearTIM&0x20))break;
		  HoldChangeGearTIM&=0xDF; 
		  if(LastModeBeforeturbo==Mode_Ramp)
				{
				//用户从极速模式模式单击+长按返回无级调速
				IsUpdateFanSpeed=1;
				TargetVoltage=9.7;
				TargetfanSpeed=70;  
				IsRampKeyStillHold=1; //标记用户回到无级调速，进入最高挡位且禁止调速功能，直到用户放开按键
				SwitchToGear(Mode_Ramp);
				}
		  else 
				{
				//正常返回处理：回到高速挡位，并且给换挡系统mark一下禁止继续响应，直到用户放开按键
				HoldChangeGearTIM|=0x40;
				SwitchToGear(Mode_High);
				}
		  break;
		}		
	//处理FSM中的表驱动部分
	if(ModeBeforeFSMSwitch==CurrentMode->ModeIdx)
		{
		//如果状态机FSM内有操作或者当前处于版本检查状态则跳过表驱动，否则执行表驱动
		ModeSwitchFSMTableDriver(ClickCount); 
		}
	//判定换挡结果
	ClearShortPressEvent(); 	//表驱动事项响应完毕，清除按键状态
		
	//返回挡位数据是否更新的结果
	if(Rampresult)return true;	//无级调节模式下如果调节发生，则更新风扇速度	
	if(ModeBuf!=CurrentMode->ModeIdx)return true; //挡位发生变更时刷新风扇状态
	return false;	
	}

//挡位状态机
void ModeSwitchFSM(void)
	{
	char ClickCount;
	//获取按键状态
	ClickCount=getSideKeyShortPressCount();	//读取按键处理函数传过来的参数
	//挡位记忆参数检查
	if(LastMode==Mode_OFF||LastMode>Mode_High)LastMode=Mode_UltraLow;									//全局常规记忆
	if(LastModeBeforeturbo==Mode_OFF||LastModeBeforeturbo==Mode_Turbo)LastModeBeforeturbo=Mode_UltraLow; //进入特殊功能挡位前的记忆
		
	//系统已锁定，此时进行锁定判断	
	if(IsSystemLocked)
		{
		//五击解锁，绿灯闪三次
		if(ClickCount==5)
			{
			LEDMode=LED_GreenBlinkThird; 
			IsSystemLocked=0;
			if(CellVoltage>2850)DisplayUnlockTIM=12;  //电池电压足够时令风扇低速旋转1.5秒
			}
		//其余按键事件，红色闪五次提示已锁定
		else if(IsKeyEventOccurred())LEDMode=LED_RedBlinkFifth;
		//清除按键状态后跳过下面所有内容
		ClearShortPressEvent();
		}
	//解锁提示解开，风扇强制低速转一下
	else if(DisplayUnlockTIM)
		{
		IsUpdateFanSpeed=1;
		TargetVoltage=VMinMaxCfg.SysMinVolt;
		TargetfanSpeed=(float)VMinMaxCfg.SysMinSpeed;
		}
	//正常模式，应用风扇速度配置
	else
		{
		if(ModeFSMDrvPROC(ClickCount))IsUpdateFanSpeed=1; //挡位发生变更，需要重新计算风扇参数
		switch(CurrentMode->ModeIdx)
			{
			//无极调速模式使用无极调速的数据
			case Mode_Ramp:
				TargetVoltage=RampVoltage;
				TargetfanSpeed=RampDuty;
				break;
			//极速模式使用极速降档配置的数据
			case Mode_Turbo:
				TargetfanSpeed=TurboCfg.TurboCurrentDuty;
			  TargetVoltage=TurboCfg.TurboCurrentVoltage;
		    break;
			//其余挡位使用模式结构体内的数据
			default:
				TargetVoltage=CurrentMode->TargetVOUT;
				TargetfanSpeed=CurrentMode->Speed;
			  break;
			}
		}
	}

