#include "PCA9536.h"
#include "I2C.h"
#include "I2CAddr.h"
#include <stdlib.h>

//����PCA9536оƬ��Ӧ��IO����
bool PCA9536_SetIODirection(SMIOPinDef IOPINNum,SMIODirDef Direction)
	{
	char buf;
	//��ȡ���üĴ���
	IIC_Start();
	IIC_Send_Byte(SMIOADDR);
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(0x03); //ָ��PORT Configuration Reg
	if(IIC_Wait_Ack())return false;
	IIC_Start();
	IIC_Send_Byte(SMIOADDR+1);
	if(IIC_Wait_Ack())return false;
	buf=IIC_Read_Byte(0);
	IIC_Stop();
	//���÷���
	if(Direction==PCA9536_IODIR_IN)buf|=(char)IOPINNum;
	else buf&=~(char)IOPINNum; //��Ӧ��bit�����1��ʾ���룬0��ʾ���
	//��д�Ĵ���
	IIC_Start();
	IIC_Send_Byte(SMIOADDR);
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(0x03); //ָ��PORT Configuration Reg
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(buf); //д�ض�Ӧ�ļĴ���
  if(IIC_Wait_Ack())return false;
	//�ɹ����ͨ�ţ�����true
	IIC_Stop();
  return true;
	}
	
//����PCA9536оƬ��Ӧ��IO������ĵ�ƽ
bool PCA9536_SetIOState(SMIOPinDef IOPINNum,bool PinState)
	{
	char buf;
	//��ȡ����Ĵ���
	IIC_Start();
	IIC_Send_Byte(SMIOADDR);
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(0x01); //ָ��Output Reg
	if(IIC_Wait_Ack())return false;
	IIC_Start();
	IIC_Send_Byte(SMIOADDR+1);
	if(IIC_Wait_Ack())return false;
	buf=IIC_Read_Byte(0);
	IIC_Stop();
	//���ö�ӦIO���������
	if(PinState)buf|=(char)IOPINNum;
	else buf&=~(char)IOPINNum; //���ö�Ӧ��bit
	//��д�Ĵ���
	IIC_Start();
	IIC_Send_Byte(SMIOADDR);
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(0x01); //ָ��Output Reg
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(buf); //д�ض�Ӧ�ļĴ���
  if(IIC_Wait_Ack())return false;
	//�ɹ����ͨ�ţ�����true
	IIC_Stop();
  return true;
	}
	
//��ȡPCA9536оƬ��Ӧ��IO��ƽ��������ģʽ����Ч��
bool PCA9536_ReadInputState(SMIOPinDef IOPINNum,bool *PinState)
	{
	char buf;
	//��ȡ����Ĵ���
	IIC_Start();
	IIC_Send_Byte(SMIOADDR);
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(0x00); //ָ��Input Reg
	if(IIC_Wait_Ack())return false;
	IIC_Start();
	IIC_Send_Byte(SMIOADDR+1);
	if(IIC_Wait_Ack())return false;
	buf=IIC_Read_Byte(0);
	IIC_Stop();
	//����bit Mask
	buf&=IOPINNum;
	if(PinState!=NULL)*PinState=buf?true:false;
	//ͨ�ųɹ����
	return true;
	}
	
//����PCA9536оƬ��Ӧ��IO����
bool PCA9536_SetIOPolarity(SMIOPinDef IOPINNum,SMIODirPolarDef Polarity)
	{
	char buf;
	//��ȡ���ԼĴ���
	IIC_Start();
	IIC_Send_Byte(SMIOADDR);
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(0x02); //ָ��PORT Polarity Reg
	if(IIC_Wait_Ack())return false;
	IIC_Start();
	IIC_Send_Byte(SMIOADDR+1);
	if(IIC_Wait_Ack())return false;
	buf=IIC_Read_Byte(0);
	IIC_Stop();
	//���÷���
	if(Polarity==PCA9536_IO_Inverted)buf|=(char)IOPINNum;
	else buf&=~(char)IOPINNum; //��Ӧ��bit�����1��ʾ�����෴��0��ʾ������
	//��д�Ĵ���
	IIC_Start();
	IIC_Send_Byte(SMIOADDR);
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(0x02); //ָ��PORT Polarity Reg
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(buf); //д�ض�Ӧ�ļĴ���
  if(IIC_Wait_Ack())return false;
	//�ɹ����ͨ�ţ�����true
	IIC_Stop();
  return true;
	}
