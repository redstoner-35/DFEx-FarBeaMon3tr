#ifndef _PCA9536_
#define _PCA9536_

//�ڲ�����
#include <stdbool.h>

//ö��
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
	
//����
bool PCA9536_SetIODirection(SMIOPinDef IOPINNum,SMIODirDef Direction);	//����PCA9536оƬ��Ӧ��IO����
bool PCA9536_SetIOState(SMIOPinDef IOPINNum,bool PinState); //����PCA9536оƬ��Ӧ��IO������ĵ�ƽ
bool PCA9536_ReadInputState(SMIOPinDef IOPINNum,bool *PinState);  //��ȡPCA9536оƬ��Ӧ��IO��ƽ��������ģʽ����Ч��
bool PCA9536_SetIOPolarity(SMIOPinDef IOPINNum,SMIODirPolarDef Polarity); //����PCA9536оƬ��Ӧ��IO����
	
#endif
