#include "ModeControl.h"
#include "LEDMgmt.h"
#include "SideKey.h"
#include "BattDisplay.h"
#include "TempControl.h"
#include "SpecialMode.h"
#include "SysConfig.h"
#include "LowVoltProt.h"

//全局变量
static xdata unsigned char ShowTacModeTIM;
bit IsDisplayLocked;
SpecialOperationDef SysMode; //系统模式

//进入退出锁定切换
static void EnterExitLock(void)
	{
	DisplayLockedTIM=8; //指示锁定状态切换
	SysMode=!SysMode?Operation_Locked:Operation_Normal;
	SaveSysConfig(0);
	}
	
//进入退出战术切换
static void EnterExitTac(void)
	{
	DisplayLockedTIM=2; //指示战术切换
	SysMode=!SysMode?Operation_TacTurbo:Operation_Normal;
	}	

//进入月光处理
void EnterMoonProcess(void)
	{
	extern bit TemporaryDisableVoltageQuery;
	//电池电压足够的时候进入月光
	if(Battery>2.8)SwitchToGear(Mode_Moon);
	//高于2.4V每节则进入月光
	else if(Battery>2.4)
		{		
		TemporaryDisableVoltageQuery=1;
		SwitchToGear(Mode_1Lumen);
		}
	//电量已经低于DCDC可工作的水平，系统禁止开机并红色闪5次
	else LEDMode=LED_RedBlinkFifth; 
	}	

//开启到普通模式
void PowerToNormalMode(ModeIdxDef Mode)
	{
	if(Battery>3.0)SwitchToGear(IsRampEnabled?Mode_Ramp:Mode); //正常开启
	else if(Battery>2.65)EnterMoonProcess();  //电池电压大于2.7，执行进入月光判断    		
	else if(CurrentMode->ModeIdx==Mode_OFF)LEDMode=LED_RedBlinkFifth;	//手电处于关机状态下且电池电量不足，闪烁五次提示进不去	
	else ReturnToOFFState();	 //电池电量严重不足，且手电开着，直接关机
	//如果成功进入了无级模式，则进行复位处理
	if(CurrentMode->ModeIdx==Mode_Ramp)RampRestoreLVProtToMax();
	}
	
//进入极亮和爆闪的判断
void EnterTurboStrobe(char ClickCount)	
	{
	//双击极亮
	if(ClickCount==2)
		{
		//电池电量充足且没有触发关闭极亮的保护，正常开启
		if(Battery>3.45&&!IsDisableTurbo)SwitchToGear(Mode_Turbo); 
		//电池电池电量不足或者极亮被锁定尝试开到高亮去
		else PowerToNormalMode(Mode_High);	
		}
	//三击爆闪
	else if(ClickCount==3)
		{
		if(Battery>2.7)SwitchToGear(Mode_Strobe);   //进入爆闪
		else LEDMode=LED_RedBlinkFifth; //电量不足五次闪烁提示
		if(CurrentMode->ModeIdx!=Mode_OFF)LastMode=CurrentMode->ModeIdx; //在开机状态下三击爆闪，记忆进入前的挡位
		}
	}
	
//特殊模式下回到特殊功能里面的切换
void LeaveSpecialMode(char ClickCount)	
	{
	if(ClickCount==3)PowerToNormalMode(LastMode); //三击调用退回函数，退回到普通模式
  else EnterTurboStrobe(ClickCount); //其他按键次数，直接call尝试极亮函数让他自己判断去
	}	

//显示战术模式启用
bit DisplayTacModeEnabled(void)
	{
	//计时器控制
	if(CurrentMode->ModeIdx!=Mode_OFF||SysMode<Operation_TacTurbo)ShowTacModeTIM=0;
	else //进行累加
		{
		ShowTacModeTIM++; //进行增加
		if(ShowTacModeTIM==14&&SysMode==Operation_TacStrobe)return 1; //战术爆闪模式激活，频闪2次
		else if(ShowTacModeTIM==16)
			{
			ShowTacModeTIM=0;
			return 1; //返回1打开显示
			}		
		}
	//其余状态返回0
	return 0;
	}	
	
//特殊功能切换	
void SpecialModeOperation(char Click)
	{
		//复位flag
	  IsDisplayLocked=0;
		//特殊操作模式切换
			switch(SysMode)
				{
				//普通模式
				case Operation_Normal:
					if(Click==5)EnterExitLock(); //进入锁定模式
				  if(Click==6)EnterExitTac(); //进入战术模式
					break;
				//锁定模式
				case Operation_Locked:
				   if(Click==5)EnterExitLock();
				   else if(getSideKeyHoldEvent())IsDisplayLocked=1;
				   else if(IsKeyEventOccurred())LEDMode=LED_RedBlinkFifth; //指示手电已被锁定
				   break;
				//战术模式
				case Operation_TacTurbo:
				case Operation_TacStrobe:
				  if(Click==6)EnterExitTac();
					if(Click==2) //切换模式
						{
						if(SysMode==Operation_TacTurbo)
							{
							SysMode=Operation_TacStrobe;
							LEDMode=LED_GreenBlinkThird; //开启爆闪战术
							}
						else
							{
							SysMode=Operation_TacTurbo;
							LEDMode=LED_RedBlinkThird;  //关闭爆闪战术
							}
						}
					if(getSideKeyHoldEvent())EnterTurboStrobe(SysMode==Operation_TacStrobe?3:2); //调用进入函数尝试进极亮
			  break;
				}
	}	
