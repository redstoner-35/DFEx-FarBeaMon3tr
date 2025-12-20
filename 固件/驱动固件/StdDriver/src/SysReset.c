/****************************************************************************/
/** \file SysReset.c
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个文件负责控制MCU的RSTCU在特定条件下发出指令进行MCU的软复位
**
**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "cms8s6990.h"

/****************************************************************************/
/*	Global function implantation
****************************************************************************/

//生成系统复位
#pragma optimize(0)
void TriggerSoftwareReset(void)
	{
	EA=0;
	_nop_();
	TA=0xAA;
  TA=0x55;     //往TA寄存器写55AA解锁
  WDCON=0x80;  //令WDTCON[7]=1，触发系统复位
	while(1);
	}

//清除软件复位标志位
#pragma optimize(0)
void ClearSoftwareResetFlag(void)
	{
	EA=0;
	_nop_();
	TA=0xAA;
  TA=0x55;  //往TA寄存器写55AA解锁
  WDCON&=0x7F;  //令WDTCON[7]=0，触发系统复位
	}
/*************************  End Of File  ***********************/
