#ifndef _SysCfg_
#define _SysCfg_

#include "ModeControl.h"

//�洢��������
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

//������Flash����
#define	DataFlashLen 0x3FF  //CMS8S6990��Ƭ������������1KByte��Ѱַ��Χ��0-3FF
#define SysCfgGroupLen (DataFlashLen/sizeof(SysROMImg))-1   //���õ��޼������鳤��
	
	
//����
void ReadSysConfig(void);
void SaveSysConfig(bit IsForceSave);	
void RestoreToMinimumSysCurrent(void);	
	
#endif
