#include "ModeControl.h"
#include "LEDMgmt.h"
#include "SideKey.h"
#include "BattDisplay.h"
#include "TempControl.h"
#include "SpecialMode.h"
#include "SysConfig.h"
#include "SideKey.h"
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
	//电池电压足够的时候进入月光
	if(Battery>2.8)SwitchToGear(Mode_Moon);
	//高于2.4V每节则进入月光
	else if(Battery>2.4)SwitchToGear(Mode_1Lumen);
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
	
//尝试进入极亮和爆闪的处理
void TryEnterTurboStrobeProcess(char Count)	
	{
  switch(Count)
		{
		//双击极亮
		case 2:	
			//电池电量充足且没有触发关闭极亮的保护，正常开启
			if(Battery>3.45&&!IsDisableTurbo)SwitchToGear(Mode_Turbo); 
			//电池电池电量不足或者极亮被锁定尝试开到高亮去
			else PowerToNormalMode(Mode_High);	
		  break;
	//三击爆闪
		case 3:
			//尝试进入爆闪（开机状态下进入上次记忆的特殊功能），如果电池电量不足则进入失败,电量指示五次闪烁
			if(Battery>2.7)
				{			
				//在开机状态下三击，记忆进入前的挡位并进入到上次退出之前的状态
				if(CurrentMode->ModeIdx!=Mode_OFF)
					{
				  LastMode=CurrentMode->ModeIdx; 
					SwitchToGear(!IsSpecMemEnabled?Mode_Strobe:LastSpecialMode);
					}
				//关机状态下三击，一键爆闪
				else
					{
					IsStrobePoweredFromOFF=true;
					SwitchToGear(Mode_Strobe);
					}
				}
			//爆闪进入失败，LED闪五次提示
			else LEDMode=LED_RedBlinkFifth; 
		  break;
		}
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
	
//特殊功能切换处理（返回当前非0数值）
SpecialOperationDef SpecialModeOperation(char Click)
	{
		//复位flag
	  IsDisplayLocked=0;
		//特殊操作模式切换
		switch(SysMode)
			{
			//普通模式
			case Operation_Normal:
					if(Click==5)EnterExitLock(); //进入锁定模式
				  if(Click==4)EnterExitTac(); //四击进入战术模式
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
				  if(Click==4)EnterExitTac();
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
					if(getSideKeyHoldEvent())TryEnterTurboStrobeProcess(SysMode==Operation_TacStrobe?3:2); //调用进入函数尝试进极亮
				break;
			}
	//所有运算完毕，返回系统状态（直接返回enum就行，因为非0值的话系统就是特殊模式，此时可以让条件成立）
	return SysMode;
	}	
