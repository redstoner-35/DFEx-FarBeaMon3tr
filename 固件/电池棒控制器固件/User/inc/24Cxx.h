#ifndef _24Cxx_
#define _24Cxx_

typedef enum
{
LockState_EEPROM_NACK, //EEPROMû�лظ�
LockState_Locked,  //������
LockState_Unlocked  //δ����	
}SecuLockState;

//�ڲ�����
#include "I2CAddr.h"

//����ʹ�õ�EEPROM
//#define UsingEE_24C512
#define UsingEE_24C64
//#ifdef UsingEE_24C1024

/* EEPROM�Զ�����,��ֹ�޸ģ� */
#ifdef UsingEE_24C512
  #define MaxByteRange 65535
  #define MaxPageSize 128   //24C512 (64K)
#endif

#ifdef UsingEE_24C64
  #define MaxByteRange 8191
  #define MaxPageSize 32   //24C64 (8K)
#endif

#ifdef UsingEE_24C1024
	//���õ�ַ��Χ��ҳ��С
  #define MaxByteRange 131071
  #define MaxPageSize 256    //24C1024 (128K)
#endif

//����Ƿ�����EEPROM
#ifndef MaxPageSize
  #error "You forgot to select which EEPROM you want to use!"
#endif

//M24C512
char M24C512_PageWrite(char *Data,int StartAddr,int len);//д����
char M24C512_PageRead(char *Data,int StartAddr,int len);//������
char M24C512_Erase(int StartAddr,int len);//����
char M24C512_WriteSecuSct(char *Data,int StartAddr,int len);//д��ȫ����
char M24C512_ReadSecuSct(char *Data,int StartAddr,int len);//����ȫ����
char M24C512_ReadUID(char *Data,int len);//��ȡ128Bit UID
SecuLockState M24C512_QuerySecuSetLockStat(void);//���FM24C512�İ�ȫ�����Ƿ�����
char M24C512_LockSecuSct(void);//����ȫ�洢������

#endif
