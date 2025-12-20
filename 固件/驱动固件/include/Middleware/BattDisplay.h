/************************************************************************************/
/** \file BattDisplay.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个头文件为系统的电量测量和显示模块的外部声明文件。该文件声明了触发电量
报告、执行运行时精确电池电压和系统温度以及大概的电量报告的所需的处理函数并且为上层逻辑提供
电池信息的接口。

**	History: Initial Release
**	
/*************************************************************************************/
#ifndef _BattDisplay_
#define _BattDisplay_
/*************************************************************************************/
/*	Include Files
**************************************************************************************/
#include "LEDMgmt.h"
/*************************************************************************************/
/*	Global type definitions('typedef')
**************************************************************************************/
typedef enum
	{
	Battery_Plenty, //电池电量充足
	Battery_Mid, //电池电量较为充足
	Battery_Low, //电池电量不足
	Battery_VeryLow //电池电量严重不足
	}BattStatusDef;

typedef enum
	{
  BattVdis_Waiting, //等待显示阶段
	BattVdis_PrepareDis, //准备显示
	BattVdis_DelayBeforeDisplay, //延迟一段时间
	BattVdis_Show10V, //显示十位
	BattVdis_Gap10to1V, //十位和个位之间的等待
	BattVdis_Show1V, //显示个位
	BattVdis_Gap1to0_1V, //个位和十分位之间的等待
	BattVdis_Show0_1V, //显示小数点后一位(0.1V)
	BattVdis_WaitShowChargeLvl, //等待一段时间后显示当前电量
	BattVdis_ShowChargeLvl, //显示电池电量的等待
	BattVdis_WaitShowTempState,
	BattVdis_ShowTempState	
	}BattVshowFSMDef; //电池电量显示处理

/************************************************************************************/
/* Extern Flags and Variable definition - Battery Statu interface related */
/************************************************************************************/
extern bit IsBatteryAlert; 
extern bit IsBatteryFault; 			//电池低电量警告和故障发生标志位
extern BattStatusDef BattState; //电池当前大致的电量状态	
	
//滤波之后的电池组等效单节电压（LSB=1mV，不反应电池组在不平衡状态下的实际单节电压）	
extern xdata int CellVoltage; 	

/************************************************************************************/
/* Extern Flags and Variable definition - Voltage & Temperature Report FSM related  */
/************************************************************************************/
extern xdata BattVshowFSMDef VshowFSMState; //状态机状态	
	
/************************************************************************************/
/* Extern Functions definition - Runtime Battery statu Report Handler */
/************************************************************************************/	
void BatteryTelemHandler(void);  //电池测量和指示灯控制	
void BattDisplayTIM(void); //电池电量显示函数处理	
	
/************************************************************************************/
/* Extern Functions definition - Battery Voltage & Temperature Report Trigger */
/************************************************************************************/		
void TriggerTShowDisplay(void); //启动温度显示
void TriggerVshowDisplay(void); //启动电池电压显示

/************************************************************************************/
/* Extern Functions definition - Battery Report Initialization & other stuff */
/************************************************************************************/	
void DisplayVBattAtStart(bit IsPOR); 		//在启动时显示电池电压
bit LowPowerStrobe(void); 							//低电量提示闪
LEDStateDef VshowEnter_ShowIndex(void);	//在特定情况下显示进入特殊模式的先导闪
	
#endif /* _BattDisplay_ */

/*********************************  End Of File  ************************************/
