#include "cms8s6990.h"
#include "Flash.h"

//解锁/上锁Flash
void SetFlashState(bit IsUnlocked)
	{
	//禁止中断并解锁flash
	if(IsUnlocked)
		{
		EA=0;
		_nop_();
		MLOCK = 0xAA;	
		}
	//重新打开中断并锁住Flash
	else
		{
		MLOCK = 0x55;		
		_nop_();
		EA=1; //重新启用中断
		}
	}
	
//读取数据
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
