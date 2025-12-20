/************************************************************************************/
/** \file SetupMenu.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个头文件为版本信息报告组件的外部声明文件。该文件声明了固件编译日期
查询功能的逻辑处理函数和触发显示函数，以及用于查询版本信息报告组件状态的接口变量。

**	History: Initial Release
**	
/*************************************************************************************/
#ifndef _VersionCheck_
#define _VersionCheck_
/*************************************************************************************/
/*	Global type definitions('typedef')
**************************************************************************************/
typedef enum
	{
	VersionCheck_InAct,
	VersionCheck_StartInit,
	VersionCheck_ShowNumber,
	VersionCheck_ShowNumberWait,
	VersionCheck_LoadNextNumber,
	}VersionChkFSMDef;

/************************************************************************************/
/* Extern Functions definition */
/************************************************************************************/		
void VersionCheck_Trigger(void);		//触发版本信息报告
char VersionCheckFSM(void);					//执行版本信息报告流程的逻辑处理

/************************************************************************************/
/* Extern Flags and Variable definition */
/************************************************************************************/	
extern xdata VersionChkFSMDef VChkFSMState;	
	
#endif /* _VersionCheck_ */

/*********************************  End Of File  ************************************/
