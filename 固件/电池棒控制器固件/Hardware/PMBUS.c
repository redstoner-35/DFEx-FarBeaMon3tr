#include "PMBUS.h"
#include "delay.h"
#include <math.h>


//PMBUS的2^N指数计算
double PMBUS_2NPowCalc(int Exp)
  {
	//调用C语言数学库计算
  double result;
	result=pow(2,(double)Exp);
	return result;
	}
//将SLINEAR11换算为Float
float PMBUS_SLinearToFloat(unsigned int SIN)
  {
	int exp;
	float Buf;
	exp=SIN>>11&0x1F;//取指数
	if(exp&0x10)exp|=0xFFFFFFF0;//转32bit负数
	Buf=PMBUS_2NPowCalc(exp);//将换算出来的2*N放入缓冲区	
	SIN&=0x7FF;//将指数部分去掉剩下底数
	Buf*=(float)SIN;//乘上底数得到最终的结果
	return Buf;
	}
//仅发送命令
bool PMBUS_SendCommand(char Addr,char Command)
  {
	//发送起始位，地址，命令
  IIC_Start();
  IIC_Send_Byte(Addr);
  if(IIC_Wait_Ack())return false;//芯片无反应，返回false
	IIC_Send_Byte(Command);
	if(IIC_Wait_Ack())return false;//芯片无反应，返回false
	//成功完成操作
	IIC_Stop();
	return true;
	}
//单字节读写
bool PMBUS_ByteReadWrite(bool IsWrite,unsigned char *Byte,char Addr,char Command)
  {
	//发送起始位，地址，命令
  IIC_Start();
  IIC_Send_Byte(Addr);
  if(IIC_Wait_Ack())return false;//芯片无反应，返回false
	IIC_Send_Byte(Command);
	if(IIC_Wait_Ack())return false;//芯片无反应，返回false
	//写入
	if(IsWrite)
	  {
		IIC_Send_Byte(*Byte);
		if(IIC_Wait_Ack())return false;//芯片无反应，返回false
		}
	//读取
	else
	  {
		IIC_Start();
    IIC_Send_Byte(Addr+1);
		if(IIC_Wait_Ack())return false;//芯片无反应，返回false
		*Byte=IIC_Read_Byte(0);//读完1byte后nack表示不要送出更多数据
		}
	//成功完成操作
	IIC_Stop();
	return true;
	}
//双字节读写(word)
//MSB First表示发送和接收字节的顺序，如果是为true 则顺序为 [D15-D8][D7-D0]
bool PMBUS_WordReadWrite(bool MSBFirst,bool IsWrite,unsigned int *Word,char Addr,char Command)
  {
	unsigned int buf;
	//发送起始位，地址，命令
  IIC_Start();
  IIC_Send_Byte(Addr);
  if(IIC_Wait_Ack())return false;//芯片无反应，返回false
	IIC_Send_Byte(Command);
	if(IIC_Wait_Ack())return false;//芯片无反应，返回false
	//写入(根据参数设置LSB还是MSB First)
	if(IsWrite)
	  {
		IIC_Send_Byte(MSBFirst?((*Word>>8)&0xFF):(*Word&0xFF));//发送数据
		if(IIC_Wait_Ack())return false;//芯片无反应，返回false
		IIC_Send_Byte(MSBFirst?(*Word&0xFF):((*Word>>8)&0xFF));
		if(IIC_Wait_Ack())return false;//芯片无反应，返回false
		}
  //读取(根据参数设置LSB还是MSB First)
	else
	  {
		IIC_Start();
    IIC_Send_Byte(Addr+1);
		if(IIC_Wait_Ack())return false;//芯片无反应，返回false
		buf=MSBFirst?((IIC_Read_Byte(1)<<8)&0xFF00):IIC_Read_Byte(1);//调整接收字节顺序
		buf|=MSBFirst?IIC_Read_Byte(0):(IIC_Read_Byte(0)<<8);
	  *Word=buf;//读取完成，赋值
		}	
	//成功完成操作
	IIC_Stop();
	return true;
	}
//进行块写入操作前获取数据量
bool PMBUS_GetBlockWriteCount(int *Size,char Addr,char Command)
  {
  *Size=-1;//尺寸设置为-1，在命令没有成功时返回错误值
	//发送起始位，地址，命令
  IIC_Start();
  IIC_Send_Byte(Addr);
  if(IIC_Wait_Ack())return false;//芯片无反应，返回false
	IIC_Send_Byte(Command);
	if(IIC_Wait_Ack())return false;//芯片无反应，返回false
	//读取块数目
	IIC_Start();
  IIC_Send_Byte(Addr+1);
	if(IIC_Wait_Ack())return false;//芯片无反应，返回false
	*Size=(int)IIC_Read_Byte(0);
	//成功完成操作
	IIC_Stop();
	return true;
	}
//进行块读写操作
bool PMBUS_BlockReadWrite(bool IsWrite,unsigned char *Buf,char Addr,char Command)
  {
	int size,i;
	//如果是写入的，会尝试获取尺寸
	if(IsWrite)
	  {
		//获取目标写入的尺寸
		if(!PMBUS_GetBlockWriteCount(&size,Addr,Command))
		  return false;
		delay_us(10);
		}
  //发送起始位，地址，命令
  IIC_Start();
  IIC_Send_Byte(Addr);
  if(IIC_Wait_Ack())return false;//芯片无反应，返回false
	IIC_Send_Byte(Command);
	if(IIC_Wait_Ack())return false;//芯片无反应，返回false
	//块写入
	if(IsWrite)
	  {
		IIC_Send_Byte(size);
		if(IIC_Wait_Ack())return false;//发送字节数量告诉从机我要写多少字节过来
		for(i=0;i<size;i++)//从低到高发送所欲内容
			{
			IIC_Send_Byte(Buf[i]);
			if(IIC_Wait_Ack())return false;//芯片无反应，返回false
			}		
		}
	//块读取
	else
	  {
		//读取块数目
		size=0;//初始化为0
	  IIC_Start();
    IIC_Send_Byte(Addr+1);
	  if(IIC_Wait_Ack())return false;//芯片无反应，返回false
	  size=(int)IIC_Read_Byte(1);
		if(!size||size>=0xFF)//读到的数据异常
		  {
			IIC_Stop();
			return false;//停止通信并返回错误
			}
		for(i=0;i<size;i++)Buf[i]=IIC_Read_Byte(i<(size-1)?1:0);//读取数据	
		}
	//成功完成操作
	IIC_Stop();
	return true;	
	}
//读写供应商额外定义寄存器(Renesas ISL68/691xx)
bool ISL_ReadWriteMFRReg(MFREXTRWCFG *CFGSTR)
 {
 int i;
 unsigned int DBUF;
 unsigned char PageNum;
 //操作供电芯片指向要写参数的page，不成功则退出
 if(CFGSTR->PageCommandRequired!=0)
   {
   PageNum=CFGSTR->PageNumber;
	 if(!PMBUS_ByteReadWrite(true,&PageNum,CFGSTR->VRMAddress,0x00))
		 return false;
	 }
 //发送命令代码
 IIC_Start();//开始信号
 IIC_Send_Byte(CFGSTR->VRMAddress);//发送目标地址（写）
 if(IIC_Wait_Ack()!=0)return false;
 IIC_Send_Byte(0xF7);	   //发送MFR Specific EXT Command
 if(IIC_Wait_Ack()!=0)return false;
 IIC_Send_Byte(CFGSTR->EXTRegCode);//发送额外寄存器代码 
 if(IIC_Wait_Ack()!=0)return false;
 IIC_Send_Byte(CFGSTR->EXTCommandCode);//发送额外命令代码
 if(IIC_Wait_Ack()!=0)return false;
 IIC_Stop();//停止
 delay_ms(2);//在发出命令之后要延迟2ms才能继续读写数据
 //如果是写入，则执行以下流程
 if(CFGSTR->IsWrite!=0)
  {
  IIC_Start();//开始信号
  IIC_Send_Byte(CFGSTR->VRMAddress);//发送目标地址（写）
  if(IIC_Wait_Ack()!=0)return false;
  IIC_Send_Byte(0xF5);	   //发送MFR Specific EXT Data
	if(IIC_Wait_Ack()!=0)return false;	
	DBUF=*CFGSTR->DataIO;//传入要发出的数据
  for(i=0;i<4;i++)
   {
	  IIC_Send_Byte(DBUF&0xFF);//发送配置数据
	  if(IIC_Wait_Ack()==1)return false;//等待发送完毕,如果NACK则退出
		 DBUF>>=8;//发送完毕1个字节，把高字节移动下来继续发
	 }
	}
 //否则执行读取
 else
  {
	IIC_Start();//开始信号
  IIC_Send_Byte(CFGSTR->VRMAddress);//发送目标地址（写）
  if(IIC_Wait_Ack()!=0)return false;
  IIC_Send_Byte(0xF5);	   //发送MFR Specific EXT Data
	if(IIC_Wait_Ack()!=0)return false;	
	IIC_Start();//开始信号
  IIC_Send_Byte(CFGSTR->VRMAddress+1);//发送目标地址（读）
  if(IIC_Wait_Ack()!=0)return false;
  for(i=0;i<4;i++)//开始接受数据
   {
   DBUF>>=8;//将放在高位的数据往低位方向移动
	 DBUF|=IIC_Read_Byte(i<3?1:0)<<24;//接收数据放到最高位
	 }
	*CFGSTR->DataIO=DBUF;//将读取的数据传出过去
	}
 //操作完毕
 IIC_Stop();//通信结束，停止
 return true;
 }
