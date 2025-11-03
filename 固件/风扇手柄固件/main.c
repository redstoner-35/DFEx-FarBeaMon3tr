#include "cms8s6990.h"
#include "GPIO.h"
#include "delay.h"
#include "SideKey.h"
#include "LEDMgmt.h"
#include "ADCCfg.h"
#include "PWMCfg.h"
#include "BattDisplay.h"
#include "OutputChannel.h"
#include "ModeSel.h"
#include "LowVoltProt.h"
#include "SysReset.h"

//函数声明
void SleepMgmt(void);
void MaskUnusedIO(void);

//主函数
void main()
	{
	bit TaskSel=0;
	//启动系统定时器提供系统定时和延时函数
  StartSystemTimeBase(); 
	ClearSoftwareResetFlag();
	//初始化外设
  LED_Init(); //初始化侧按LED		
	ADC_Init(); //初始化ADC
	WaitBatteryVoltageOK(); //等待电池电压稳定
	SideKeyInit(); //初始化按键控制器
	PWM_Init(); //启动PWM定时器
	OutputChannel_Init(); //初始化输出通道
	ModeFSMInit(); //初始化模式状态机
	BattCellCountConfig(); //电池节数识别处理
	MaskUnusedIO(); //屏蔽掉不用的IO
	DisplayVBattAtStart(1); //上电时显示电池电压	
  //主循环
	while(1)
		{
		//实时处理	
		SystemTelemHandler();//对ADC执行采集获取系统状态
		SideKey_LogicHandler(); //处理侧按事务
		BatteryTelemHandler(); //处理电池遥测	
		ModeSwitchFSM(); //挡位变换状态机
		OutputChannel_Calc(); //输出通道运算
		PWM_OutputCtrlHandler(); //处理PWM输出事务		
		//8Hz定时处理
		if(!SysHFBitFlag)continue; //时间没到，跳过处理
    if(!TaskSel)
			{
			//Task0，处理计算量比较大的任务
	    LEDControlHandler();//侧按指示LED控制函数
			SideKey_TIM_Callback();//侧按按键的监测定时器处理
			TurboTimedStepDownPROC(); //极速定时降档处理
			SleepMgmt(); //睡眠处理
			
			//任务处理完毕，处理task 1
			TaskSel=1;
			}
		else
			{		
			//Task1，处理计算量比较小的计时任务
			BattAlertTIMHandler(); //电池警报处理
			HoldSwitchGearCmdHandler(); //长按换挡命令处理
			BattDisplayTIM(); //电池电量显示TIM
			RampConfigAutoSaveHandler(); //无级调速自动保存处理
				
			//任务处理完毕，处理task 0
			TaskSel=0;
			}
	  //处理完毕，令flag清零
		SysHFBitFlag=0;	
		}
	}
