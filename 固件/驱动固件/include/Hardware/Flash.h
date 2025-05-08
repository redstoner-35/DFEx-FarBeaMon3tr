#ifndef _FSH_
#define _FSH_

typedef enum
	{
	DataFlash_Read=0x11,
	DataFlash_Write=0x19,
	DataFlash_Erase=0x1D
	}FlashOperationDef;

//º¯Êý
void SetFlashState(bit IsUnlocked);
void Flash_Operation(FlashOperationDef Operation,int ADDR,char *Data);
	
#endif