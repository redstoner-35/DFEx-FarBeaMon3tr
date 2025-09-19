#ifndef _SysCfg_
#define _SysCfg_

//内部define
#define IsLocked_MSK 0x01  //是否锁定 bit1
#define IsEnableIdleLED_MSK 0x02 //是否开启有源夜光 bit2
#define IsEnable2SMode_MSK 0x04 //是否开启2串模式 bit3
#define IsEnableBattCfgLock_MSK 0x08 //是否开启电池串数配置锁

//存储类型声明
typedef struct
	{
	float RampDuty;
	float RampVout;
  unsigned char BitfieldMem1;
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
#define SysCfgGroupLen (DataFlashLen/sizeof(SysROMImg))-1   //可用的配置组合长度

//函数	
void ReadSysConfig(void);
void SaveSysConfig(bit IsForceSave);	
void LoadMinimumRampCurrentToRAM(void);	
	
#endif
