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

//睡眠定时器
volatile int SleepTimer;

//禁用/启用所有系统外设
void SystemPeripheralCTRL(bit IsEnable)
	{
	if(IsEnable)
		{
		ADC_Init(); //初始化ADC
		PWM_Init(); //初始化PWM发生器
		LED_Init(); //初始化侧按LED
		OutputChannel_Init(); //初始化输出通道
		return;
		}
	//关闭所有外设
	SetSystemHBTimer(0); //禁用心跳定时器
	PWM_DeInit();
	ADC_DeInit(); //关闭PWM和ADC
	LocateLED_Enable(); //打开定位LED
	OutputChannel_DeInit(); //对输出通道进行复位
	}
	
//加载定时器时间
void LoadSleepTimer(void)	
	{
	//加载睡眠时间
	SleepTimer=SysMode>Operation_Locked?4800:8*SleepTimeOut; //睡眠时间延长		
	}

//检测系统是否允许进入睡眠的条件
static char QueryIsSystemNotAllowToSleep(void)
	{
	//系统处于定位指示灯选择状态，不允许睡眠
	if(LocLEDState!=LocateLED_NotEdit)return 1;
	//系统在显示电池电压不允许睡眠
	if(VshowFSMState!=BattVdis_Waiting)return 1;
	//系统开机了
	if(CurrentMode->ModeIdx!=Mode_OFF)return 1;
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
		C0CON0=0; //侧按关机后关闭比较器
		SystemPeripheralCTRL(0);//关闭所有外设
		STOP();  //令STOP=1，使单片机进入睡眠
		//系统已唤醒，立即开始检测
		delay_init();	 //延时函数初始化
		SetSystemHBTimer(1); 
		MarkAsKeyPressed(); //立即标记按键按下
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
		//系统已被唤醒，立即进入工作模式			
		SystemPeripheralCTRL(1);
		//所有外设初始化完毕，启动ADC异步处理模式并打开系统中断
		EnableADCAsync(); 
		}
	}
