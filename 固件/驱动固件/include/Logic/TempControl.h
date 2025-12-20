/************************************************************************************/
/** \file TempControl.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个头文件为系统顶层逻辑模块的声明文件。该模块声明了负责系统温度管理的
相关逻辑的处理函数以及输出温控结果的输出函数，并且声明了温度控制器输出到其余逻辑模块的接
口变量。

**	History: Initial Release
**	
/************************************************************************************/
#ifndef _TempControl_
#define _TempControl_
/*************************************************************************************/
/*	Global type definitions('typedef')
**************************************************************************************/
typedef enum
	{
	//降档提示原因美剧
	StepDown_OFF, 					//提示未触发
	StepDown_Thermal, 			//过热
	StepDown_BattAlert, 		//电池撑不住
	StepDown_ECOModeEnabled //ECO模式开启
	}StepDownReasonDef;
/************************************************************************************/
/* Extern Functions definition - Result Export & Init Operation */
/************************************************************************************/	
int ThermalILIMCalc(void); 					//根据温控模块计算电流限制
void RecalcPILoop(int LastCurrent); //换挡的时候重新计算PI环路
	
/************************************************************************************/
/* Extern Functions definition - Thermal Management Logic Handler */
/************************************************************************************/		
void ThermalPILoopCalc(void); 	//温控PI环路的计算
void ThermalMgmtProcess(void); 	//温控管理函数
	
/************************************************************************************/
/* Extern Flags and Variable definition */
/************************************************************************************/
extern bit IsDisableTurbo; //关闭极亮进入
extern bit IsForceLeaveTurbo; //强制退出极亮

#endif /* _TempControl_ */

/*********************************  End Of File  ************************************/
