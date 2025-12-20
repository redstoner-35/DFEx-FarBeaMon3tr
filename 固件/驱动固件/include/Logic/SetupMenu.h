/************************************************************************************/
/** \file SetupMenu.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个头文件为系统设置菜单逻辑组件的外部声明文件。该文件声明了系统设置菜单
的逻辑处理函数和触发设置函数，以及用于查询设置菜单组件状态的接口变量。

**	History: Initial Release
**	
/*************************************************************************************/
#ifndef _SetupMenu_
#define _SetupMenu_
/*************************************************************************************/
/*	Global type definitions('typedef')
**************************************************************************************/
typedef enum
	{
	SetupMenu_InACT=0,
	SetupMenu_InactiveExitWait=1,
	SetupMenu_Prepare,
	SetupMenu_ShowContent,
	SetupMenu_WaitShowContent,
	SetupMenu_SubmitContent,
	
	//位域编辑
	SetupMenu_BitFieldEdit
	}SetupMenuFSMDef;
/************************************************************************************/
/* Extern Functions definition */
/************************************************************************************/	
LEDStateDef SetupMenuFSM(void);       //设置系统的状态机
void TriggerSetupMenuDisplay(void);		//触发设置系统开始显示
	
/************************************************************************************/
/* Extern Flags and Variable definition */
/************************************************************************************/
extern xdata SetupMenuFSMDef SetupFSMState;

#endif /* _SetupMenu_ */

/*********************************  End Of File  ************************************/
