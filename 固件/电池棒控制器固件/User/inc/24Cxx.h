#ifndef _24Cxx_
#define _24Cxx_

typedef enum
{
LockState_EEPROM_NACK, //EEPROM没有回复
LockState_Locked,  //已上锁
LockState_Unlocked  //未上锁	
}SecuLockState;

//内部包含
#include "I2CAddr.h"

//设置使用的EEPROM
//#define UsingEE_24C512
#define UsingEE_24C64
//#ifdef UsingEE_24C1024

/* EEPROM自动配置,禁止修改！ */
#ifdef UsingEE_24C512
  #define MaxByteRange 65535
  #define MaxPageSize 128   //24C512 (64K)
#endif

#ifdef UsingEE_24C64
  #define MaxByteRange 8191
  #define MaxPageSize 32   //24C64 (8K)
#endif

#ifdef UsingEE_24C1024
	//设置地址范围和页大小
  #define MaxByteRange 131071
  #define MaxPageSize 256    //24C1024 (128K)
#endif

//检测是否定义了EEPROM
#ifndef MaxPageSize
  #error "You forgot to select which EEPROM you want to use!"
#endif

//M24C512
char M24C512_PageWrite(char *Data,int StartAddr,int len);//写数据
char M24C512_PageRead(char *Data,int StartAddr,int len);//读数据
char M24C512_Erase(int StartAddr,int len);//擦除
char M24C512_WriteSecuSct(char *Data,int StartAddr,int len);//写安全扇区
char M24C512_ReadSecuSct(char *Data,int StartAddr,int len);//读安全扇区
char M24C512_ReadUID(char *Data,int len);//读取128Bit UID
SecuLockState M24C512_QuerySecuSetLockStat(void);//检查FM24C512的安全扇区是否被锁定
char M24C512_LockSecuSct(void);//给安全存储区上锁

#endif
