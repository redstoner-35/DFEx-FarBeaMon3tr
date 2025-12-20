/************************************************************************************/
/** \file LocateLED.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个头文件为系统的侧按有源夜光功能模块的声明文件，该模块声明了侧按有源夜
光LED模块相关的硬件底层控制函数，以及负责实现设置菜单中对电流拖尾特效和有源夜光颜色编辑的
编辑子系统。
**	History: Initial Release
**	
/*************************************************************************************/
#ifndef _LOCLED_
#define _LOCLED_
/*************************************************************************************/
/*	Global type definitions('typedef')
**************************************************************************************/
typedef enum
	{
	LocateLED_NotEdit=0,
	LocateLED_WaitKeyRelease=1,
	LocateLED_Sel=2,
	LocateLED_SelFading=3
	
	}LocLEDEditDef;

/*************************************************************************************/
/*	  	Extern Functions definition - Display Handler and Initialization
**************************************************************************************/	
void InitLocateLEDEditSys(void);	
LEDStateDef LocateLED_ShowType(void);	
	
/*************************************************************************************/
/*	  	Extern Functions definition - Logic Handler
**************************************************************************************/		
char LocateLED_Edit(void);
void LocateLED_TIMHandler(void);		
	
/*************************************************************************************/
/*	  	Extern Functions definition - Hardware Configuration for Locate LED
**************************************************************************************/		
void LocateLED_Enable(void);

/************************************************************************************/
/* Extern Flags and Variable definition */
/************************************************************************************/
extern xdata LocLEDEditDef LocLEDState;
	
#endif /* _LOCLED_ */

/*********************************  End Of File  ************************************/
