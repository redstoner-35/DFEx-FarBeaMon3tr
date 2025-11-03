#include "ModeSel.h"
#include "cms8s6990.h"
#include "OutputChannel.h"
#include "BattDisplay.h"
#include "SideKey.h"
#include "LEDMgmt.h"
#include "ADCCfg.h"
#include "PWMCfg.h"
#include "delay.h"
#include "LVDCtrl.h"

//睡眠定时器
volatile unsigned int SleepTimer;

//函数声明
void MaskUnusedIO(void);

//禁止所有系统外设
static void DisableSysPeripheral(void)
	{
	MaskUnusedIO();    //关闭所有无用的IO
	DisableSysHBTIM(); 
	PWM_DeInit();
	ADC_DeInit(); //关闭PWM和ADC
	OutputChannel_DeInit(); //对输出通道进行复位
	LED_DeInit(); //关闭LED控制器
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
	LVD_Disable(); //关闭LVD
	}

//加载定时器时间
void LoadSleepTimer(void)	
	{
	//加载睡眠时间
	SleepTimer=8*SleepTimeOut; 		
	}
	
//检测系统是否允许进入睡眠的条件
static char QueryIsSystemNotAllowToSleep(void)
	{
	//系统在显示电池电压和版本号，不允许睡眠
	if(VshowFSMState!=BattVdis_Waiting)return 1;
	//系统开机了
	if(CurrentMode->ModeIdx!=Mode_OFF)return 1;
	//允许睡眠
	return 0;
	}	

//实际的系统休眠处理流程
static void SleepProcHandler(void)
	{
	bit sleepsel;
	unsigned char ADCSampleCounter;
	//关闭外设并初始化欠压定时器
	ADCSampleCounter=12;
	DisableSysPeripheral();
	//开始睡眠处理
	do
		{		
		STOP();  //令STOP=1，使单片机进入睡眠
		//唤醒之后需要跟6条NOP
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		//系统已唤醒，立即开始检测
		if(GetIfSideKeyTriggerInt()) 
			{
			//检测到系统并非由LVD唤醒，立即完成初始化判断按键状态
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
			return;
			}
		//欠压自杀计时器计时中
		else if(ADCSampleCounter)ADCSampleCounter--;
		//系统由WUT唤醒，启动ADC，检测电池电压后立即睡眠
		else
			{
			//初始化ADC	
			ADCSampleCounter=11;    //欠压自杀不需要特别频繁的采样
			ADC_Init(); 						//初始化ADC
			SystemTelemHandler(); 
			CellVoltage=(int)(Data.BatteryVoltage*1000); //启动一次ADC，获取并更新电池电压
			if(CellVoltage<2900)
				{
				LVD_Disable(); //关闭LVD
			  LED_Init(); //电池欠压，初始化侧按LED关闭上拉
				}
			//检测完毕，关闭ADC并补充极亮刷新次数
			ADC_DeInit(); 
			AddTurboRefreshCountWhenSleep();
			//令睡眠定时器=0，系统立即进入睡眠
			SleepTimer=0;
			}
		}
	while(!SleepTimer);
	}	
	
//睡眠管理函数
void SleepMgmt(void)
	{
	
	//非关机且仍然在显示电池电压的时候定时器复位禁止睡眠
	if(QueryIsSystemNotAllowToSleep())LoadSleepTimer();
	//允许睡眠开始倒计时
	if(SleepTimer>0)SleepTimer--;
	//立即进入睡眠阶段
	else SleepProcHandler();
	}
	
