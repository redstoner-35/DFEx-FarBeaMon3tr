/************************************************************************************/
/** \file Flash.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个头文件为系统Data Flash模块硬件驱动的外部声明文件。负责声明Flash操作
函数为中层的配置NVRAM存储模块提供底层驱动。

**	History: Initial Release
**	
/************************************************************************************/
#ifndef _FlashCtl_
#define _FlashCtl_
/*************************************************************************************/
/*	Global type definitions('typedef')
**************************************************************************************/
typedef enum
	{
	DataFlash_Read=0x11,
	DataFlash_Write=0x19,
	DataFlash_Erase=0x1D
	}FlashOperationDef;

/************************************************************************************/
/* Extern Functions definition */
/************************************************************************************/	
void UnlockFlash(void);						//解锁Flash
void LockFlash(void);							//重新把Flash锁上
void Flash_Operation(FlashOperationDef Operation,int ADDR,char *Data); //进行Flash操作
	
#endif /* _FlashCtl_ */
	
/*********************************  End Of File  ************************************/
