/************************************************************************************/
/** \file SysCfg.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个头文件为系统的非易失性存储模块的相关操作函数提供了声明

**	History: Initial Release
**	
/*************************************************************************************/
#ifndef _SysCfg_
#define _SysCfg_
/************************************************************************************/
/* Extern Functions definition */
/************************************************************************************/	
void ResetSysConfigToDefault(void);
void ReadSysConfig(void);
void SaveSysConfig(bit IsForceSave);	
void LoadMinimumRampCurrentToRAM(void);	
	
#endif /* _SysCfg_ */

/*********************************  End Of File  ************************************/
