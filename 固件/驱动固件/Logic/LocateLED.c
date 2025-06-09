#include "delay.h"
#include "LEDMgmt.h"
#include "GPIO.h"
#include "LocateLED.h"
#include "SideKey.h"
#include "SysConfig.h"
#include "ModeControl.h"
#include "PinDefs.h"
#include "cms8s6990.h"

//全局变量
xdata LocLEDEditDef LocLEDState=LocateLED_NotEdit;
static xdata char LocLEDTIM;
static xdata u8 LocSetTimeOutTIM;

//定位LED设置最大超时时间
#define LocateLEDTimeOut 30

//定位LED显示计时器
void LocateLED_TIMHandler(void)
	{
	if(LocLEDTIM)LocLEDTIM--;
	if(LocSetTimeOutTIM)LocSetTimeOutTIM--;
	}

//显示当前系统配置的定位LED类型
LEDStateDef LocateLED_ShowType(void)
	{
	IsHalfBrightness=SysCfg.LocatorCfg?1:0;
	//设置状态	
	switch(SysCfg.LocatorCfg)
		{
	  case Locator_OFF:	//红色每隔一段时间快闪表示关闭
			if(!LocLEDTIM)
				{
        MakeFastStrobe(LED_Red);
				LocLEDTIM=6;
				}
			break;
		case Locator_Green:return LED_Green; //绿灯
		case Locator_Red:return LED_Red; //红灯
		case Locator_Amber:return LED_Amber; //黄灯		
		}
	//其余状态返回OFF
	return LED_OFF;
	}

//使能定位LED
void LocateLED_Enable(void)
	{
	GPIOCfgDef LEDInitCfg;
	//设置结构体
	LEDInitCfg.Mode=GPIO_IPU;
  LEDInitCfg.Slew=GPIO_Slow_Slew;		
	LEDInitCfg.DRVCurrent=GPIO_High_Current; //配置为上拉输入
	//配置绿灯GPIO	
	if(SysCfg.LocatorCfg&0x01)GPIO_ConfigGPIOMode(GreenLEDIOG,GPIOMask(GreenLEDIOx),&LEDInitCfg);
	GPIO_SetMUXMode(GreenLEDIOG,GreenLEDIOx,GPIO_AF_GPIO);	
	//配置红灯GPIO
	if(SysCfg.LocatorCfg&0x02)GPIO_ConfigGPIOMode(RedLEDIOG,GPIOMask(RedLEDIOx),&LEDInitCfg);	
	GPIO_SetMUXMode(RedLEDIOG,RedLEDIOx,GPIO_AF_GPIO);	
	}

//定位LED状态编辑
char LocateLED_Edit(char ClickCount)
	{
	switch(LocLEDState)
		{
		//默认状态
		case LocateLED_NotEdit:
			//关机状态7击按键进入编辑
			if(ClickCount!=7)return 0;			
			LocLEDTIM=0;
			LocSetTimeOutTIM=8*LocateLEDTimeOut;
			LocLEDState=LocateLED_Sel;
		  break;
		//编辑过程
		case LocateLED_Sel:
			if(ClickCount)SysCfg.LocatorCfg=SysCfg.LocatorCfg<3?SysCfg.LocatorCfg+1:0; //反复切换index
		  if(!LocSetTimeOutTIM)
				{
				//设置菜单超时，不保存并退出
				LocLEDState=LocateLED_NotEdit;
				LEDMode=LED_RedBlinkFifth;      //红色LED闪五次表示编辑异常结束
				}
			if(!getSideKeyHoldEvent())break; //如果检测到长按则保存并退出		  
			DisplayLockedTIM=4; //锁定指示闪一下
			LocLEDState=LocateLED_WaitKeyRelease;
			SaveSysConfig(0);
		  break;
		//等待按键放开
		case LocateLED_WaitKeyRelease:
			if(getSideKeyHoldEvent())break;  //等待按键放开
		  getSideKeyLongPressEvent();  
			LocLEDState=LocateLED_NotEdit; //获取一遍长按事件避免长按退出保存的时候开机到月光 
      break;
		}		
  //其余事件，返回1
	return 1;	
	}