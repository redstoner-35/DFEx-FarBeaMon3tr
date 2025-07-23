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
#include "LEDMgmt.h"
#include "LocateLED.h"
#include "SetupMenu.h"
#include "Strobe.h"
#include "VersionCheck.h"

//睡眠定时器
volatile unsigned int SleepTimer;

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

//加载定时器时间
void LoadSleepTimer(void)	
	{
	//加载睡眠时间
	if(SysMode>Operation_Locked)SleepTimer=4800;	//开启战术模式，睡眠时间延长
	else if(CurrentMode->ModeIdx==Mode_Fault)SleepTimer=240; //故障报错模式，系统睡眠时间变为240S
	else SleepTimer=8*SleepTimeOut; 		
	}

//检测系统是否允许进入睡眠的条件
static char QueryIsSystemNotAllowToSleep(void)
	{
	//系统处于定位指示灯选择或者设置菜单状态，不允许睡眠
	if(LocLEDState||SetupFSMState)return 1;
	//系统在显示电池电压和版本号，不允许睡眠
	if(VshowFSMState!=BattVdis_Waiting||VChkFSMState!=VersionCheck_InAct)return 1;
	//系统开机了
	if(IsLargerThanOneU8(CurrentMode->ModeIdx))return 1;
	//允许睡眠
	return 0;
	}	
	
//睡眠管理函数
void SleepMgmt(void)
	{
	bit sleepsel;
	//非关机且仍然在显示电池电压的时候定时器复位禁止睡眠
	if(QueryIsSystemNotAllowToSleep())LoadSleepTimer();
	//允许睡眠开始倒计时
	if(SleepTimer>0)SleepTimer--;
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
