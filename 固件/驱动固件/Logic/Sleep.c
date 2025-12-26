/****************************************************************************/
/** \file Sleep.c
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个文件为顶层应用层逻辑文件。负责实现系统在长时间未使用时，自
		动进入低功耗待机模式以节省电力的相关逻辑的处理。

**	History: 
				2025年12月26日 10:05 1.重命名加载睡眠超时的函数至ResetSleepTimer()
				                     2.修改自动define系统在睡眠时间错误时的报告entry的
															 ID错误重复的问题。
														 3.针对新增的可自行运行的紧急月光模式增加了阻止系统
														   休眠的对应entry。
														 
				2025年12月20日 Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "cms8s6990.h"
#include "delay.h"
#include "SideKey.h"
#include "PWMCfg.h"
#include "PinDefs.h"
#include "ModeControl.h"
#include "OutputChannel.h"
#include "SpecialMode.h"
#include "BattDisplay.h"
#include "ADCCfg.h"
#include "FastOP.h"
#include "LEDMgmt.h"
#include "LocateLED.h"
#include "SetupMenu.h"
#include "Strobe.h"
#include "VersionCheck.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros - for Parameter Definition
****************************************************************************/
#define DefaultSleepTimeOut 5 		 //默认情况下的睡眠进入延时，单位(秒)
#define SleepTimeOutWhenFault 30   //系统故障情况下的睡眠进入延时，单位(秒)
#define SleepTimeOutForTac 10      //系统开启战术模式后的睡眠进入延时，单位(分)

/****************************************************************************/
/*	Local pre-processor symbols/macros - for Parameter Parsing
****************************************************************************/

//默认睡眠时间的计数器值计算
#define DefaultSleepCNTVAL 8*DefaultSleepTimeOut
#if (DefaultSleepCNTVAL > 0xFFFE | DefaultSleepCNTVAL < 8)
	#error "Error 013:Invalid Sleep timeout for default mode!"
#endif

//系统出现故障时睡眠时间的计数器值计算
#define SleepCNTVALWhenFault 8*SleepTimeOutWhenFault
#if (SleepCNTVALWhenFault > 0xFFFE | SleepCNTVALWhenFault < 120)
	#error "Error 014:Invalid Sleep timeout when fault occurred!"
#endif

#define SleepCNTVALWhenTac 480*SleepTimeOutForTac
#if (SleepCNTVALWhenTac > 0xFFFE | SleepCNTVALWhenTac < 480)
	#error "Error 015:Invalid Sleep timeout for tacital mode!"
#endif

/****************************************************************************/
/*	Local variable and Flag definitions('static')
****************************************************************************/
static unsigned int SleepTimer;

/****************************************************************************/
/*	Local Function implementation - Peripheral Management
****************************************************************************/
//禁止所有系统外设
static void DisableSysPeripheral(void)
	{
	DisableSysHBTIM(); 
	PWM_DeInit();
	ADC_DeInit(); //关闭PWM和ADC
	LocateLED_Enable(); //打开定位LED
	OutputChannel_DeInit(); //对输出通道进行复位
	}

//启动所有系统外设
static void EnableSysPeripheral(void)
	{
	ADC_Init(); //初始化ADC
	PWM_Init(); //初始化PWM发生器
	LED_Init(); //初始化侧按LED
	OutputChannel_Init(); //初始化输出通道
	SystemTelemHandler(); //启动一次ADC，进行初始测量
	DisplayVBattAtStart(0); //执行一遍电池初始化函数	
	EnableADCAsync(); 			//所有外设初始化完毕，启动ADC异步处理模式
	}

/****************************************************************************/
/*	Local Function implementation - Sleep Condition Check
****************************************************************************/

//检测系统是否允许进入睡眠的条件
static char QueryIsSystemNotAllowToSleep(void)
	{
	//系统处于定位指示灯选择或者设置菜单状态以及开启了应急月光，不允许睡眠
	if(LocLEDState||SetupFSMState||IsDisplayLocked)return 1;
	//系统在显示电池电压和版本号，不允许睡眠
	if(VshowFSMState!=BattVdis_Waiting||VChkFSMState!=VersionCheck_InAct)return 1;
	//系统开机了
	if(IsLargerThanOneU8(CurrentMode->ModeIdx))return 1;
	//允许睡眠
	return 0;
	}		

/****************************************************************************/
/*	Function implementation - Global(decleared in header files with 'extern')
*****************************************************************************/	
	
//复位进入睡眠的倒计时定时器
void ResetSleepTimer(void)	
	{
	//加载睡眠时间
	if(SysMode>Operation_Locked)SleepTimer=SleepCNTVALWhenTac;	//开启战术模式，睡眠时间延长
	else if(CurrentMode->ModeIdx==Mode_Fault)SleepTimer=SleepCNTVALWhenFault; //故障报错模式，系统睡眠时间变为240S
	else SleepTimer=DefaultSleepCNTVAL; 		
	}
	
//睡眠管理函数
void SleepMgmt(void)
	{
	bit sleepsel;
	//非关机且仍然在显示电池电压的时候定时器复位禁止睡眠
	if(QueryIsSystemNotAllowToSleep())ResetSleepTimer();
	//允许睡眠开始倒计时
	if(SleepTimer)SleepTimer--;
	//立即进入睡眠阶段
	else
		{		
		if(SysMode>Operation_Locked)SysMode=Operation_Normal; //强制退出战术模式
		DisableSysPeripheral();//关闭所有外设
		STOP();  //令STOP=1，使单片机进入睡眠
		//系统已唤醒，立即开始检测
		StartSystemTimeBase(); //启动系统定时器提供系统定时和延时函数
		MarkAsKeyPressed(); //立即标记按键按下
		SideKey_SetIntOFF(); //关闭侧按中断
		do	
			{
			delay_ms(1);
			SideKey_LogicHandler(); //处理侧按事务
			//侧按按键的监测定时器处理(使用62.5mS心跳时钟,通过2分频)
			if(!SysHFBitFlag)continue; 
			SysHFBitFlag=0;
			sleepsel=~sleepsel;
			if(sleepsel)SideKey_TIM_Callback();
			}
		while(!IsKeyEventOccurred()); //等待按键唤醒
		//系统已完成按键事件检测，初始化其余外设		
		EnableSysPeripheral();
		//每次上电复位爆闪控制器	
		ResetStrobeModule(); 			
		}
	}
/*********************************  End Of File  ************************************/
