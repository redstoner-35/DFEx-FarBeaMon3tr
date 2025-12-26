/****************************************************************************/
/** \file SetupMenu.c
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个文件为顶层应用层逻辑文件。该文件实现了系统中设置固件各个属性
偏好的设置菜单功能。

**	History:
				2025年12月26日 10:05 1.修改系统包含的设置项至9个以容纳新增的四击功能选择
				                     2.在系统设置的handler里面注册相关的功能选择bit
														 
				2025年12月20日 Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "SideKey.h"
#include "ModeControl.h"
#include "delay.h"
#include "Strobe.h"
#include "LEDMgmt.h"
#include "SetupMenu.h"
#include "LocateLED.h"
#include "FastOp.h"
#include "SysConfig.h"
#include "BattDisplay.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros for Parameter definition ('#define')
****************************************************************************/
#define TotalSetupNum 9	 //系统总共包含的设置项数量（不要随便改！会爆的！）


/****************************************************************************/
/*	Local variable and Flag definitions('static')
*****************************************************************************/
static xdata unsigned char SetupMenuIdx;
static xdata unsigned char SetupTimedOutTIM;
static bit BitBuf;

/****************************************************************************/
/*	Global & Extern variable and Flag definitions('extern')
*****************************************************************************/

//通用软件计时变量
extern xdata unsigned char CommonSysFSMTIM;

//设置菜单FSM存储
xdata SetupMenuFSMDef SetupFSMState;

/****************************************************************************/
/*	Function implementation - local('static')
****************************************************************************/	

//进行按键响应的函数
static void KeyAddFluxProcess(void)
	{
	//按键按下之后就进行处理
	if(!getSideKeyShortPressCount())return;
	SetupTimedOutTIM=50;
	//进行数值翻转
	if(SetupFSMState==SetupMenu_BitFieldEdit)BitBuf=~BitBuf;
	else 
		{
		//Index进行自增，自增后立即使定时器变为0开始下一轮显示,显示最新的数值
		SetupMenuIdx++;
		SetupMenuIdx%=TotalSetupNum;
		CommonSysFSMTIM=0;
		SetupFSMState=SetupMenu_WaitShowContent;
 		}
	ClearShortPressEvent();
	}

//设置菜单的index自动增加
static void SetupMenuIdxAutoADD(void)
	{
	//单击+长按进入编辑,进入后绿色闪3下
	if(getSideKeyNClickAndHoldEvent()==1)
		{
		SetupFSMState=SetupMenu_SubmitContent;
		CommonSysFSMTIM=13;
		}
	//非正常退出处理
  else if(getSideKeyLongPressEvent())SetupTimedOutTIM=0;
		
	//进行菜单增减
  KeyAddFluxProcess();
	}
	
/****************************************************************************/
/*	Function implementation - Global(decleared in header files with 'extern')
*****************************************************************************/	
	
//触发设置菜单选项
void TriggerSetupMenuDisplay(void)
	{
	if(SetupFSMState!=SetupMenu_InACT)return;
	SetupFSMState=SetupMenu_Prepare;
	CommonSysFSMTIM=15;
	}	

//状态机处理
LEDStateDef SetupMenuFSM(void)
	{
	//正常状态机处理	
	switch(SetupFSMState)
		{
		//非激活状态
		case SetupMenu_InACT:return LED_OFF;
		//准备显示
		case SetupMenu_Prepare:		    
			  if(!CommonSysFSMTIM)
					{
					//等待计时时间到
					SetupMenuIdx=0;
					SetupTimedOutTIM=100;
					SetupFSMState=SetupMenu_WaitShowContent; //启动菜单显示
					}
			  //显示先导提示
				return VshowEnter_ShowIndex();
		    break;
		//开始显示内容顺便接收按键操作
		case SetupMenu_ShowContent:
        SetupMenuIdxAutoADD(); //执行自动增加
			  if(!CommonSysFSMTIM) 
					{
					if(SetupTimedOutTIM)SetupTimedOutTIM--;
					CommonSysFSMTIM=10;
					SetupFSMState=SetupMenu_WaitShowContent; //等待一会
					}
				//制造绿色闪烁(最后一项是黄色)提示当前选的菜单项
				else if((CommonSysFSMTIM%4)&0x7E)
					{
					if(SetupMenuIdx==(TotalSetupNum-1))return LED_Amber;
					else return LED_Green;
					}
				break;
		//进行等待显示内容的处理
		case SetupMenu_WaitShowContent:
			  SetupMenuIdxAutoADD(); //执行自动增加
			  if(CommonSysFSMTIM)break;
		    CommonSysFSMTIM=(4*(SetupMenuIdx+1))-1;
		    SetupFSMState=SetupMenu_ShowContent;
		    break;
	  //选择对应的菜单项
	  case SetupMenu_SubmitContent:
			  //等待用户松开按键
				if(CommonSysFSMTIM&0xF8)return LED_GreenBlinkThird;
				if(CommonSysFSMTIM||(SetupMenuIdx!=(TotalSetupNum-1)&&getSideKeyNClickAndHoldEvent()==1))break;
        //非有源夜光模式，启动对应的Edit
			  if(IsLargerThanOneU8(SetupMenuIdx))
					{
					switch(SetupMenuIdx)
						{
						case 2:
							//菜单项3：是否启用无极调光	
							BitBuf=IsRampEnabled;
							break;						
						case 3:
							//菜单项4：是否开启主挡位记忆
							BitBuf=IsMainMemEnabled; 
							break;
						case 4:
							//菜单项5：是否开启特殊挡位记忆
							BitBuf=IsSpecMemEnabled; 
							break;
						case 5:
							//菜单项6：是否开启特殊挡位记忆
							BitBuf=IsPowerModeEnabled;
							break;			
						case 6:
							//菜单项7：是否开启随机变频爆闪
						  BitBuf=EnableRandomStrobe;
						  break;
						case 7:
							//菜单项8：设置系统四击的逻辑
						  BitBuf=QuadClickSel;
						  break;
						//菜单项9：恢复出厂设置
						case 8:ResetSysConfigToDefault();	
						}
					CommonSysFSMTIM=20;
					SetupTimedOutTIM=50;
					SetupFSMState=SetupMenu_BitFieldEdit;
					}
				//执行有源夜光处理
				else
					{
					//index正好是等于实际数值+2
					LocLEDState=(LocLEDEditDef)SetupMenuIdx+2;
					InitLocateLEDEditSys();
					//菜单项已选择交给后续的东西处理，退出
					SetupFSMState=SetupMenu_InACT;
					}
				break;
		case SetupMenu_BitFieldEdit:	
				//按键按下切换index
				KeyAddFluxProcess();		
				//长按保存
		    if(getSideKeyLongPressEvent())
					{
					//回写编辑值
					switch(SetupMenuIdx)
						{
						case 2:IsRampEnabled=BitBuf;break;				//菜单项2：是否启用无极调光
						case 3:IsMainMemEnabled=BitBuf;break; 		//菜单项3：是否开启主挡位记忆
						case 4:IsSpecMemEnabled=BitBuf;break;			//菜单项4：是否开启特殊挡位记忆
						case 5:IsPowerModeEnabled=BitBuf;break;		//菜单项5：POWER-ECO模式
						case 6:EnableRandomStrobe=BitBuf;break;		//菜单项6：开启随机变频爆闪
						case 7:QuadClickSel=BitBuf;break;         //菜单项7：设置系统四击的逻辑
						}
					//保存数据并提示
					DisplayLockedTIM=4; //锁定指示闪一下
					SaveSysConfig(0);
					CommonSysFSMTIM=0;
					SetupFSMState=SetupMenu_InactiveExitWait; //跳到等待用户放开按键
					break;
					}						

			 //返回bit位结果
			 if(IsLargerThanOneU8(CommonSysFSMTIM))return BitBuf?LED_Green:LED_Red; //计时器倒计时ing，显示状态
			 else if(!CommonSysFSMTIM)
				 {
				 //一轮显示时间到，复位超时计时器并重置结果
				 CommonSysFSMTIM=20;
				 if(SetupTimedOutTIM)SetupTimedOutTIM--; //反复累减超时计时器
				 }
			 break;
				 
		case SetupMenu_InactiveExitWait:
			 //等待用户放开按键
	     if(CommonSysFSMTIM)return LED_RedBlinkFifth; //红色闪五次表示异常退出
       if(getSideKeyHoldEvent())break;
			 //用户已经松开按键，返回到未设置状态
		   SetupFSMState=SetupMenu_InACT; 
		   break;
		}
	//超时，异常退出
  if(!SetupTimedOutTIM&&IsLargerThanOneU8(SetupFSMState))
		{
		CommonSysFSMTIM=6;
		SetupFSMState=SetupMenu_InactiveExitWait;		
		}
	//默认情况下返回
	return LED_OFF;
	}
/*********************************  End Of File  ************************************/
