/************************************************************************************/
/** \file LowVoltProt.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个头文件为系统的低电流保护模块的外部声明文件。该模块声明了实现极亮输入
自适应MPPT、常规挡位和无极调光低电量保护所需的处理函数，以及极亮MPPT所需要的部分接口。

**	History: Initial Release
**	
/*************************************************************************************/
#ifndef _LVProt_
#define _LvProt_
/*************************************************************************************/
/*	Include Files
**************************************************************************************/
#include "stdbool.h"
#include "TempControl.h"

/************************************************************************************/
/* Extern Flags and Variable definition - Battery Statu interface related */
/************************************************************************************/
extern xdata int TurboILIM; 						//极亮电流限制
extern xdata float BeforeRawBattVolt; 	//极亮前电池电压的采样(V)

/************************************************************************************/
/* Extern Functions definition - Low Voltage Protect Logic handler and timing 
	 handler for Other Mode */
/************************************************************************************/	
void BatteryLowAlertProcess(bool IsNeedToShutOff,ModeIdxDef ModeJump); 
void BattAlertTIMHandler(void);

/************************************************************************************/
/* Extern Functions definition - Low Voltage Protect Logic handler and Input 
	 Auto MPPT Tracking handler for Turbo Mode */
/************************************************************************************/	
void CalcTurboILIM(void); 
void TurboLVILIMProcess(void); //极亮专属的电流运行值的功能

/************************************************************************************/
/* Extern Functions definition - Low Voltage Protect Logic handler and timing 
	 handler for Ramp(StepLess Adjustment) Mode */
/************************************************************************************/	
void RampRestoreLVProtToMax(void); //每次开机进入无级模式时尝试恢复限流
void RampLowVoltHandler(void); 			//无极调光的专属处理

/************************************************************************************/
/*  Extern Functions definition - Query System Step down state  */
/************************************************************************************/	
StepDownReasonDef QuerySystemTurboILIMState(void); 

#endif /* _LvProt_ */

/*********************************  End Of File  ************************************/

