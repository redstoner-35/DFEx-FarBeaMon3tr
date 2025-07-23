#include "SideKey.h"
#include "ModeControl.h"
#include "delay.h"
#include "Strobe.h"
#include "LEDMgmt.h"
#include "SetupMenu.h"
#include "LocateLED.h"
#include "SysConfig.h"

//外部变量和函数
LEDStateDef VshowEnter_ShowIndex(void);
extern xdata unsigned char CommonSysFSMTIM;

//内部变量
xdata SetupMenuFSMDef SetupFSMState;
static xdata unsigned char SetupMenuIdx;
static xdata unsigned char SetupTimedOutTIM;
static bit BitBuf;

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
		if(IsRampFault&&SetupMenuIdx==1)SetupMenuIdx++; 										//无极调光故障，设置菜单在设置项1的时候额外加1调光无极调光设置
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
  if(getSideKeyLongPressEvent())SetupTimedOutTIM=0;
		
	//进行菜单增减
  KeyAddFluxProcess();
	}
	
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
						//菜单项8：恢复出厂设置
						case 7:ResetSysConfigToDefault();	
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
						}
					//保存数据并提示
					DisplayLockedTIM=4; //锁定指示闪一下
					SaveSysConfig(0);
					CommonSysFSMTIM=0;
					SetupFSMState=SetupMenu_InactiveExitWait; //跳到等待用户放开按键
					break;
					}						

			 //返回bit位结果
			 if(!CommonSysFSMTIM)
				 {
			   CommonSysFSMTIM=20;
				 if(SetupTimedOutTIM)SetupTimedOutTIM--; //反复累减超时计时器
				 }
			 else return BitBuf?LED_Green:LED_Red;
				 
		case SetupMenu_InactiveExitWait:
			 //等待用户放开按键
	     if(CommonSysFSMTIM)return LED_RedBlinkFifth; //红色闪五次表示异常退出
       if(getSideKeyHoldEvent())break;
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
