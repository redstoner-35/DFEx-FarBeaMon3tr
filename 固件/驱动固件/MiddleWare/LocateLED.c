/****************************************************************************/
/** \file LocateLED.c
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个文件为中层设备驱动文件，负责实现驱动在待机模式下点亮侧按指示
灯发出微光指示手电开关位置的功能。同时该驱动文件实现了设置菜单子系统中针对待机指示
的颜色设置和电流拖尾特效的配置功能。

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "delay.h"
#include "LEDMgmt.h"
#include "GPIO.h"
#include "LocateLED.h"
#include "SideKey.h"
#include "SysConfig.h"
#include "ModeControl.h"
#include "PinDefs.h"
#include "SideKey.h"
#include "cms8s6990.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros for Parameter definition ('#define')
****************************************************************************/ 
#define LocateLEDTimeOut 30		//定位LED设置最大超时时间(秒)

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/
xdata LocLEDEditDef LocLEDState=LocateLED_NotEdit;
extern xdata unsigned char CommonSysFSMTIM;

/****************************************************************************/
/*	Local variable definitions('static')
****************************************************************************/	
static xdata u8 LocSetTimeOutTIM;

/****************************************************************************/
/*	Local variable definitions('static')
****************************************************************************/	

//将电流拖尾设置转换为定位灯对应index的LUT
static code LocatorLEDDef Fading2LocLEDLUT[4]=
	{
	//项0，对应Fading_OFF	
	Locator_OFF,
	//项1，对应Fading_Enable_Fast
	Locator_Green,
	//项2，对应Fading_Enable_Mid,
  Locator_Amber,
	//项3，对应Fading_Enable_Slow
	Locator_Red		
	};

/****************************************************************************/
/*	Function implementation - local('static')
****************************************************************************/	
	
//内部函数，负责执行对应index的循环自增操作
static void LoopAddSysIndex(void)
	{
	char buf;
	//取出数据
	if(LocLEDState==LocateLED_SelFading)buf=SysCfg.FadingCfg;
	else buf=SysCfg.LocatorCfg;
	//循环自增(0-3)
	if(buf&0x04)buf=0;
	else buf++;
	//写回去
	if(LocLEDState==LocateLED_SelFading)SysCfg.FadingCfg=(ShutdownFadingDef)buf;
	else SysCfg.LocatorCfg=(LocLEDEditDef)buf;	
	}	

/****************************************************************************/
/*	Global Function implementation - Logic Handler
****************************************************************************/

//定位LED显示的计时处理函数
void LocateLED_TIMHandler(void)
	{
	if(LocSetTimeOutTIM)LocSetTimeOutTIM--;
	}
	
//定位LED和系统电流拖尾状态编辑的逻辑处理函数
char LocateLED_Edit(void)
	{
	switch(LocLEDState)
		{
		//默认状态，返回0正常执行下面的逻辑
		case LocateLED_NotEdit:return 0;			
		//编辑过程
		case LocateLED_SelFading:
		case LocateLED_Sel:
			if(getSideKeyShortPressCount())LoopAddSysIndex(); //反复切换index
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

/****************************************************************************/
/*	Global Function implementation - Hardware Configuration for Locate LED
****************************************************************************/	
	
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
	
/****************************************************************************/
/*	Global Function implementation - Display Handler and Initialization
****************************************************************************/
	
//显示当前系统配置的定位LED以及电流拖尾速度类型的处理函数（被LED管理器调用）
LEDStateDef LocateLED_ShowType(void)
	{
	LocatorLEDDef Data;
	//读取设置index
  if(LocLEDState==LocateLED_SelFading)
		{		
		//读取配置（通过LUT转换一下）
		Data=Fading2LocLEDLUT[SysCfg.FadingCfg&0x03];		
		//在开启拖尾的时候，通过指定间隔的熄灭提示用户当前选择的拖尾速度（速度越慢，闪烁间隔越长）
		if(Data)
			{
			if(!CommonSysFSMTIM)CommonSysFSMTIM=SysCfg.FadingCfg*3;
			else if(CommonSysFSMTIM==1)return LED_OFF;
			}
		}
  else Data=SysCfg.LocatorCfg;
	//设置状态
	IsHalfBrightness=(Data==Locator_OFF)?1:0;	 //关闭功能的话按键灯设置为低亮
	switch(Data)
		{
	  case Locator_OFF:	//红色每隔一段时间快闪表示关闭
			if(!CommonSysFSMTIM)
				{
        MakeFastStrobe(LED_Red);
				CommonSysFSMTIM=5;
				}
			break;
		case Locator_Green:return LED_Green; //绿灯
		case Locator_Red:return LED_Red; //红灯
		case Locator_Amber:return LED_Amber; //黄灯		
		}
	//其余状态返回OFF
	return LED_OFF;
	}

//初始化定位LED编辑模块并进入编辑模式
void InitLocateLEDEditSys(void)	
	{
	CommonSysFSMTIM=0;
	LocSetTimeOutTIM=8*LocateLEDTimeOut;
	}
/*********************************  End Of File  ************************************/
