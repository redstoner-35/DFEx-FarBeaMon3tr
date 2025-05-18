#include "cms8s6990.h"
#include "GPIO.h"
#include "delay.h"
#include "SideKey.h"
#include "LEDMgmt.h"
#include "ADCCfg.h"
#include "OutputChannel.h"
#include "PWMCfg.h"
#include "BattDisplay.h"
#include "ModeControl.h"
#include "TempControl.h"
#include "SOS.h"
#include "SelfTest.h"
#include "LowVoltProt.h"
#include "Beacon.h"
#include "LocateLED.h"
#include "Strobe.h"

//函数声明
void SleepMgmt(void);

//主函数
void main()
	{
	bit TaskSel=0;
	//时钟初始化	
 	delay_init();	 //延时函数初始化
	SetSystemHBTimer(1);//启用系统心跳8Hz定时器	
	//初始化外设
	OutputChannel_Init(); //启动输出通道	
	ADC_Init(); //初始化ADC
	PWM_Init(); //启动PWM定时器
	LED_Init(); //初始化侧按LED
	ModeFSMInit(); //初始化挡位状态机
  SideKeyInit(); //侧按初始化	
	OutputChannel_TestRun(); //进行输出通道试运行
	DisplayVBattAtStart(); //显示输出电压
	EnableADCAsync(); //启动ADC的异步模式提高处理速度
	//主循环	
  while(1)
		{
	  //实时处理
		SystemTelemHandler();//获取电池信息	
		SideKey_LogicHandler(); //处理侧按事务
		BatteryTelemHandler(); //处理电池遥测
		ModeSwitchFSM(); //挡位状态机
		ThermalMgmtProcess(); //温度管理函数过热保护等
		OutputChannel_Calc(); //根据电流进行输出通道控制
		PWM_OutputCtrlHandler(); //处理PWM输出事务	
		//8Hz定时处理
		if(!SysHFBitFlag)continue; //时间没到，跳过处理
		SysHFBitFlag=0;	
		//Task0，处理计算量比较大的任务
    if(!TaskSel)
			{
			RandStrobeHandler(); //处理伪随机爆闪任务
			ThermalPILoopCalc(); //积分器计算
			ModeFSMTIMHandler(); //模式状态机计时
			BeaconFSM_TIMHandler();	//信标模式计时器	
			LEDControlHandler();//侧按指示LED控制函数
			OCFSM_TIMHandler(); //输出通道状态机计时
			HoldSwitchGearCmdHandler(); //长按换挡
			SideKey_TIM_Callback();//侧按按键的监测定时器处理
			//处理结束，对任务选择进行翻转处理下一组
			TaskSel=1;
		  }			
		//Task1，处理计算量比较小的计时任务
		else
			{			
			OutputFaultDetect();//输出故障检测
			BattDisplayTIM(); //电池电量显示TIM
			SOSTIMHandler(); //SOS计时器
			BattAlertTIMHandler(); //电池警报定时处理
			DisplayErrorTIMHandler(); //故障代码显示
			SleepMgmt(); //休眠管理
			LocateLED_TIMHandler(); //定位LED计时
			//处理结束，对任务选择进行翻转处理下一组
			TaskSel=0;
			}
		}
	}
