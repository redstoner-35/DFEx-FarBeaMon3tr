/************************************************************************************/
/** \file SpecialMode.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个头文件为系统的低电流保护模块的外部声明文件。该模块声明了实现极亮输入
自适应MPPT、常规挡位和无极调光低电量保护所需的处理函数，以及极亮MPPT所需要的部分接口。

**	History: Initial Release
**	
/*************************************************************************************/
#ifndef _SpecialMode_
#define _SpecialMode_
/*************************************************************************************/
/*	Include Files
**************************************************************************************/
#include "ModeControl.h"

/*************************************************************************************/
/*	Global type definitions('typedef')
**************************************************************************************/
typedef enum
	{
	Operation_Normal=0, //正常操作
	Operation_Locked=1, //锁定模式
	Operation_TacTurbo=2, //战术模式(极亮)
	Operation_TacStrobe=3, //锁定模式
	}SpecialOperationDef;	

/************************************************************************************/
/* Extern Flags and Variable definition */
/************************************************************************************/
extern SpecialOperationDef SysMode; //特殊功能
extern bit IsDisplayLocked; //显示锁定
	
/************************************************************************************/
/*  Extern Functions definition */
/************************************************************************************/
void PowerToNormalMode(ModeIdxDef Mode);							//开启到普通模式
void TryEnterTurboStrobeProcess(char Count);					//尝试进入极亮和爆闪的内部处理
SpecialOperationDef SpecialModeOperation(char Click);	//特殊功能切换的处理
void EnterMoonProcess(void); 													//进入月光模式的处理
	
#endif /* _SpecialMode_ */

/*********************************  End Of File  ************************************/
