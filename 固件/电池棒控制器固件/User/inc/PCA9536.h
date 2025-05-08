#ifndef _PCA9536_
#define _PCA9536_

//内部包含
#include <stdbool.h>

//枚举
typedef enum
	{
	PCA9536_IOPIN_0=0x01,
	PCA9536_IOPIN_1=0x02,
	PCA9536_IOPIN_2=0x04,
	PCA9536_IOPIN_3=0x08,
	}SMIOPinDef;

typedef enum
	{
	PCA9536_IODIR_OUT,
	PCA9536_IODIR_IN,
	}SMIODirDef;	
	
typedef enum
	{
	PCA9536_IO_Normal,
	PCA9536_IO_Inverted,
	}SMIODirPolarDef;		
	
//函数
bool PCA9536_SetIODirection(SMIOPinDef IOPINNum,SMIODirDef Direction);	//设置PCA9536芯片对应的IO方向
bool PCA9536_SetIOState(SMIOPinDef IOPINNum,bool PinState); //设置PCA9536芯片对应的IO所输出的电平
bool PCA9536_ReadInputState(SMIOPinDef IOPINNum,bool *PinState);  //读取PCA9536芯片对应的IO电平（仅输入模式下有效）
bool PCA9536_SetIOPolarity(SMIOPinDef IOPINNum,SMIODirPolarDef Polarity); //设置PCA9536芯片对应的IO极性
	
#endif
