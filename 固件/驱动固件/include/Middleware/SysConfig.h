#ifndef _SysCfg_
#define _SysCfg_

#include "ModeControl.h"

//存储类型声明
typedef struct
	{
	int SysCurrent;
	bool IsRampEnabled;
	bool IsSystemLocked;
	LocatorLEDDef LocatorCfg;
	}SysStorDef;
	
typedef union
	{
	SysStorDef Data;
	char ByteBuf[sizeof(SysStorDef)];
	}SysDataUnion;

typedef struct
	{
	SysDataUnion SysConfig;
	char CheckSum;
	}SysROMImageDef;

typedef union
	{
	SysROMImageDef Data;
	char ByteBuf[sizeof(SysROMImageDef)];
	}SysROMImg;

//数据区Flash定义
#define	DataFlashLen 0x3FF  //CMS8S6990单片机的数据区有1KByte，寻址范围是0-3FF
#define SysCfgGroupLen (DataFlashLen/sizeof(SysROMImg))-1   //可用的无极调光组长度
	
	
//函数
void ReadSysConfig(void);
void SaveSysConfig(bit IsForceSave);	
void RestoreToMinimumSysCurrent(void);	
	
#endif
