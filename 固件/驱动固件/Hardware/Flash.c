/****************************************************************************/
/** \file Flash.c
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个文件负责实现对于MCU内置的Data Flash的驱动并完成读取、写入
擦除等操作。为中层的配置NVRAM存储模块提供底层驱动。

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "cms8s6990.h"
#include "Flash.h"

/****************************************************************************/
/*	Function implementation - global ('extern')
****************************************************************************/

//解锁Flash
void UnlockFlash(void)
	{
	EA=0;
	_nop_();
	MLOCK = 0xAA;	
	}

//重新把Flash锁上
void LockFlash(void)
	{
	MLOCK = 0x55;		
	_nop_();
	EA=1; //重新启用中断
	}

//对系统的Data Flash进行操作（读取和写入数据）
//需要注意的是，这个函数必须先解锁Flash后才能进行操作否则会触发MCU HardFault！！

void Flash_Operation(FlashOperationDef Operation,int ADDR,char *Data)
	{
	if(Operation==DataFlash_Write)MDATA=*Data; //写入模式下需要写数据	
	MADRL = ADDR&0xFF;
	MADRH = (ADDR>>8)&0xFF; //设置地址
	_nop_();	
	MCTRL = (unsigned char)Operation; //对数据区进行读取操作
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	while(MCTRL & 0x01); //等待读取结束
	if(Operation==DataFlash_Read)*Data=MDATA; //返回数据
	}
/*************************  End Of File  ***********************/
