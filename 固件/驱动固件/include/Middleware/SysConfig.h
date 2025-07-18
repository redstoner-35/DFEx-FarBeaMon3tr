#ifndef _SysCfg_
#define _SysCfg_

#include "ModeControl.h"

//内部define
#define IsRampEnabled_MSK 0x01 //Ramp是否启用 bit1
#define IsLocked_MSK 0x02  //是否锁定 bit2
#define IsEnableMainMemory_MSK 0x04 //是否启用主挡位记忆 bit3
#define IsEnableSpecMemory_MSK 0x08 //是否启用特殊挡位记忆 bit4
#define PowerECOMode_MSK 0x10 //Power和ECO模式切换 bit5

//存储类型声明
typedef struct
	{
	int SysCurrent;
  unsigned char BitfieldMem1;
	ShutdownFadingDef FadingCfg;
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
void ResetSysConfigToDefault(void); //尝试检测用户进行重置操作
void ReadSysConfig(void);
void SaveSysConfig(bit IsForceSave);	
void RestoreToMinimumSysCurrent(void);	
	
#endif
