#include "PCA9536.h"
#include "I2C.h"
#include "I2CAddr.h"
#include <stdlib.h>

//设置PCA9536芯片对应的IO方向
bool PCA9536_SetIODirection(SMIOPinDef IOPINNum,SMIODirDef Direction)
	{
	char buf;
	//读取配置寄存器
	IIC_Start();
	IIC_Send_Byte(SMIOADDR);
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(0x03); //指向PORT Configuration Reg
	if(IIC_Wait_Ack())return false;
	IIC_Start();
	IIC_Send_Byte(SMIOADDR+1);
	if(IIC_Wait_Ack())return false;
	buf=IIC_Read_Byte(0);
	IIC_Stop();
	//设置方向
	if(Direction==PCA9536_IODIR_IN)buf|=(char)IOPINNum;
	else buf&=~(char)IOPINNum; //对应的bit如果是1表示输入，0表示输出
	//回写寄存器
	IIC_Start();
	IIC_Send_Byte(SMIOADDR);
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(0x03); //指向PORT Configuration Reg
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(buf); //写回对应的寄存器
  if(IIC_Wait_Ack())return false;
	//成功完成通信，返回true
	IIC_Stop();
  return true;
	}
	
//设置PCA9536芯片对应的IO所输出的电平
bool PCA9536_SetIOState(SMIOPinDef IOPINNum,bool PinState)
	{
	char buf;
	//读取输出寄存器
	IIC_Start();
	IIC_Send_Byte(SMIOADDR);
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(0x01); //指向Output Reg
	if(IIC_Wait_Ack())return false;
	IIC_Start();
	IIC_Send_Byte(SMIOADDR+1);
	if(IIC_Wait_Ack())return false;
	buf=IIC_Read_Byte(0);
	IIC_Stop();
	//设置对应IO的输出极性
	if(PinState)buf|=(char)IOPINNum;
	else buf&=~(char)IOPINNum; //设置对应的bit
	//回写寄存器
	IIC_Start();
	IIC_Send_Byte(SMIOADDR);
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(0x01); //指向Output Reg
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(buf); //写回对应的寄存器
  if(IIC_Wait_Ack())return false;
	//成功完成通信，返回true
	IIC_Stop();
  return true;
	}
	
//读取PCA9536芯片对应的IO电平（仅输入模式下有效）
bool PCA9536_ReadInputState(SMIOPinDef IOPINNum,bool *PinState)
	{
	char buf;
	//读取输入寄存器
	IIC_Start();
	IIC_Send_Byte(SMIOADDR);
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(0x00); //指向Input Reg
	if(IIC_Wait_Ack())return false;
	IIC_Start();
	IIC_Send_Byte(SMIOADDR+1);
	if(IIC_Wait_Ack())return false;
	buf=IIC_Read_Byte(0);
	IIC_Stop();
	//进行bit Mask
	buf&=IOPINNum;
	if(PinState!=NULL)*PinState=buf?true:false;
	//通信成功完成
	return true;
	}
	
//设置PCA9536芯片对应的IO极性
bool PCA9536_SetIOPolarity(SMIOPinDef IOPINNum,SMIODirPolarDef Polarity)
	{
	char buf;
	//读取极性寄存器
	IIC_Start();
	IIC_Send_Byte(SMIOADDR);
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(0x02); //指向PORT Polarity Reg
	if(IIC_Wait_Ack())return false;
	IIC_Start();
	IIC_Send_Byte(SMIOADDR+1);
	if(IIC_Wait_Ack())return false;
	buf=IIC_Read_Byte(0);
	IIC_Stop();
	//设置方向
	if(Polarity==PCA9536_IO_Inverted)buf|=(char)IOPINNum;
	else buf&=~(char)IOPINNum; //对应的bit如果是1表示极性相反，0表示正极性
	//回写寄存器
	IIC_Start();
	IIC_Send_Byte(SMIOADDR);
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(0x02); //指向PORT Polarity Reg
	if(IIC_Wait_Ack())return false;
	IIC_Send_Byte(buf); //写回对应的寄存器
  if(IIC_Wait_Ack())return false;
	//成功完成通信，返回true
	IIC_Stop();
  return true;
	}
