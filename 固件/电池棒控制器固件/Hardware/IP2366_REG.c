#include "I2C.h"
#include "I2CAddr.h"
#include "delay.h"
#include "IP2366_REG.h"
#include "ADC.h"
#include <math.h>
#include <string.h>

//固件ID和对应能力的Table
#define FWTableSize 7	
	
const IP2366FWCapDef IP2366FWTable[FWTableSize]=
	{
		{
		//YIQJS 140W固件，2.5mR，支持高级PDO编辑(把外部Pin选电阻的功能改回去了，硬件默认30W)以及全新的C口配置模块
		"YIQJS",
		16000,
		false,
		true,
		Power_140W,
	  2.50,           //2.5mR的shunt
		true,
		false,
		true,
		},
		{
		//YCNNH 140W固件，2.5mR，不支持高级PDO编辑
		"YCNNH",
		16000,
		false,
		false,
		Power_140W,
	  2.50,           //2.5mR的shunt
		true,
		false,
		false,
		},
		{
		//YFYMS 140W固件，2.5mR，支持高级PDO编辑
		"YFYMS",
		16000,
		false,
		true,
		Power_140W,
	  2.50,            //2.5mR的shunt		
		true,
		false,
		false,
		},
		{
		//XE2TC公版固件最大65W，5mR，不支持高级PDO编辑
		"XE2TC",
		9700,
		false,
		false,
		Power_65W,
	  5.00,            //5mR的shunt		
		false,
		false,
		false,
		},
		{
		//YATJK公版固件最大65W，5mR，不支持高级PDO编辑，支持额外的寄存器读取
		"YATJK",
		9700,
		false,
		false,
		Power_65W,
	  5.00,            //5mR的shunt		
		false,
		true,
		false,
		},
		{
		//YBNIN，最早一批的2366定制固件，4mR检流电阻，不支持高级PDO编辑
		"YBNIN",
		13000,
		false,
		false,
		Power_100W,
	  4.00,            //4mR的shunt		
		true,
		false,
		false,			
		},		
		{
		//YHKKL 140W固件，2.5mR，支持高级PDO编辑(把外部Pin选电阻的功能改回去了，硬件默认30W)
		"YHKKL",
		16000,
		false,
		true,
		Power_140W,
	  2.50,            //2.5mR的shunt		
		true,
		false,
	  false,
		}		
	};	

//全局变量	
const IP2366FWCapDef *CurrentIP2366FW=&IP2366FWTable[2]; //默认使用公版固件

//读寄存器
static bool IP2366_ReadReg(char *Data,IP2366REGDef Reg)
	{
	//开始通信,发送地址
	IIC_Start();
	IIC_Send_Byte(IP2366ADDR);
	if(IIC_Wait_Ack())return false;
	delay_us(60); //根据手册内要求，ACK之后需要延时至少50uS等待芯片准备数据
	//发送寄存器码和数据
	IIC_Send_Byte((char)Reg);
	if(IIC_Wait_Ack())return false; //发送寄存器码
	delay_us(60); //根据手册内要求，ACK之后需要延时至少50uS等待芯片准备数据
	//重新启动，发送地址准备读取数据
	IIC_Start();
	IIC_Send_Byte(IP2366ADDR+1);
	if(IIC_Wait_Ack())return false;
	delay_us(60); //根据手册内要求，ACK之后需要延时至少50uS等待芯片准备数据
	*Data=IIC_Read_Byte(0); //读完1字节后发送NACK
	//读完之后发送Stop
	IIC_Stop();
	return true;
	}

//写寄存器
static bool IP2366_WriteReg(char Data,IP2366REGDef Reg)
	{
	//开始通信,发送地址
	IIC_Start();
	IIC_Send_Byte(IP2366ADDR);
	if(IIC_Wait_Ack())return false;
	delay_us(60); //根据手册内要求，ACK之后需要延时至少50uS等待芯片准备数据
	//发送寄存器码和数据
	IIC_Send_Byte((char)Reg);
	if(IIC_Wait_Ack())return false; //发送寄存器码
	delay_us(60); //根据手册内要求，ACK之后需要延时至少50uS等待芯片准备数据
	IIC_Send_Byte(Data); //发送数据
	if(IIC_Wait_Ack())return false; //发送寄存器码
	//通信结束
	IIC_Stop();
	return true;
	}

//IP2366设置输入快充协议（不影响对外输出）
bool IP2366_SetSinkProtocol(IP2366SinkProtocolDef *Cfg)
	{
	char buf;
	//读取SYSCTL0
	if(!IP2366_ReadReg(&buf,REG_SYSCTL0))return false;
	//设置EN_VbusSinkDPDM
	if(Cfg->EnableSinkDPDM)buf|=0x10;
	else buf&=0xEF;	
	//设置EN_VbusSinkPd
	if(Cfg->EnableSinkPD)buf|=0x08;
	else buf&=0xF7;
	//设置EN_VbusSinkSCP
	if(Cfg->EnableSinkSCP)buf|=0x04;
	else buf&=0xFB;	
	//把结果写回去
	if(!IP2366_WriteReg(buf,REG_SYSCTL0))return false;
  //成功完成设置
	return true;
	}

//获取一部分芯片的信息
bool IP2366_ReadChipState(ChipStatDef *State)
	{
	char buf;
	//读取STATE CTL2
	if(!IP2366_ReadReg(&buf,REG_STATE_CTL2))return false;
	if(!(buf&0x80))State->VBusState=VBUS_NoPower;
	else if(buf&0x40)State->VBusState=VBUS_OverVolt;
	else State->VBusState=VBUS_Normal;
	//读取STATE CTL3
	if(!IP2366_ReadReg(&buf,REG_STATE_CTL3))return false;
	if(buf&0x10)State->VSysState=VSys_State_Short;
	else if(buf&0x20)State->VSysState=VSys_State_OCP;
	else State->VSysState=VSys_State_Normal;
	//读取完毕返回True
	return true;
	}	
	
//清除OCP Flag
void IP2366_ClearOCFlag(void)
	{
	//向OC Bit写1清0
	IP2366_WriteReg(0x30,REG_STATE_CTL3);
	}	
	
//对库仑计统计状态进行运算
const BatteryStateDef NotAccState[4]={Batt_StandBy,Batt_ChgWait,Batt_ChgDone,Batt_ChgError};	

bool IP2366_QueryCurrentStateIsACC(BatteryStateDef IN)
	{
	char i;
	for(i=0;i<4;i++)if(IN==NotAccState[i])return false;
	//状态允许库仑计开始统计
	return true;
	}
	
//设置PDO广播
bool IP2366_SetPDOBroadCast(PDOBroadcastDef *PDOCfg)
	{
	char buf;
	//读取TYPEC-CTL17
	if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL17))return false;
	//设置PPS2
	if(PDOCfg->EnablePPS2)buf|=0x40;
	else buf&=(~0x40);
	//设置PPS1
	if(PDOCfg->EnablePPS1)buf|=0x20;
	else buf&=(~0x20);
	//设置20VPDO
	if(PDOCfg->Enable20V)buf|=0x10;
	else buf&=(~0x10);
	//设置15V PDO
	if(PDOCfg->Enable15V)buf|=0x08;
	else buf&=(~0x08);
	//设置12V PDO
	if(PDOCfg->Enable12V)buf|=0x04;
	else buf&=(~0x04);
	//设置9V PDO
	if(PDOCfg->Enable9V)buf|=0x02;
	else buf&=(~0x02);
  //把结果写回去
	if(!IP2366_WriteReg(buf,REG_TYPEC_CTL17))return false;
  //成功完成设置
	return true;
	}	
	
//获取时间戳
bool IP2366_GetFirmwareTimeStamp(char TimeStamp[5])
	{
	char i;
	//轮询读取寄存器
	for(i=0;i<5;i++)
		{
		//开始通信,发送地址
		IIC_Start();
		IIC_Send_Byte(IP2366ADDR);
		if(IIC_Wait_Ack())return false;
		delay_us(60); //根据手册内要求，ACK之后需要延时至少50uS等待芯片准备数据
		//发送寄存器码和数据
		IIC_Send_Byte(0x69+i);
		if(IIC_Wait_Ack())return false; //发送寄存器码
		delay_us(60); //根据手册内要求，ACK之后需要延时至少50uS等待芯片准备数据
		//重新启动，发送地址准备读取数据
		IIC_Start();
		IIC_Send_Byte(IP2366ADDR+1);
		if(IIC_Wait_Ack())return false;
		delay_us(60); //根据手册内要求，ACK之后需要延时至少50uS等待芯片准备数据
		TimeStamp[i]=IIC_Read_Byte(0); //读完1字节后发送NACK
		//读完之后发送Stop
		IIC_Stop();
		//读完一个字节，延时一会再读
		delay_us(60);
		}
	//读取成功，返回结果
	return true;
	}	

//获取当前芯片的充电参数
bool IP2366_getCurrentChargeParam(int *Istop,float *Vstop)
	{
	char buf;
	int result;
	float Fbuf;
	//尝试读取寄存器获取停充电流
	if(!IP2366_ReadReg(&buf,REG_SYSCTL8))return false;
	result=(int)buf;
	result=(result&0xF0)>>4; //mask掉其余的bit并且只保留Istop[3:0]
	result*=50; //50mA-per LSB
	if(Istop!=NULL)*Istop=result;
	//尝试读取寄存器获取停充电压
	if(!IP2366_ReadReg(&buf,REG_SYSCTL2))return false;
	Fbuf=(float)buf;
	Fbuf=(Fbuf*10)+2500; //LSB=10mV,Base=2500mV
	Fbuf=(Fbuf/1000)*BATTCOUNT; //mV转V并乘以电池节数得到实际电压
	if(Vstop!=NULL)*Vstop=Fbuf;
	//操作完成
	return true;
	}
	
//获取再充电电压
bool IP2366_GetVRecharge(float *Vrecharge)
	{
	char buf;
	float Fbuf;
	//尝试读取寄存器获取停充电压
	if(!IP2366_ReadReg(&buf,REG_SYSCTL2))return false;
	Fbuf=(float)buf;
	Fbuf=(Fbuf*10)+2500; //LSB=10mV,Base=2500mV
	Fbuf=(Fbuf/1000)*BATTCOUNT; //mV转V并乘以电池节数得到实际电压
	//尝试读取寄存器获取再充电设置
	if(!IP2366_ReadReg(&buf,REG_SYSCTL8))return false;
  buf=(buf>>2)&0x03; //mask掉再充电功能
	switch(buf)
		{
		case 0:Fbuf=-1;break; //再充电关闭
		case 1:Fbuf-=(float)(BATTCOUNT*0.05);break; //再充电为每节-0.05V
		case 2:Fbuf-=(float)(BATTCOUNT*0.1);break; //再充电为每节-0.1V
		case 3:Fbuf-=(float)(BATTCOUNT*0.2);break; //再充电为每节-0.2V
		}		
	if(Vrecharge!=NULL)*Vrecharge=Fbuf;	
  //获取成功
	return true;
	}

//IP2366设置低功耗睡眠模式是否使能	
bool IP2366_SetDeepSleepModeEnabled(bool IsEnableSleep)
	{
	char buf;
	//读取SYS_CTL9
	if(!IP2366_ReadReg(&buf,REG_SYSCTL9))return false;
	if(IsEnableSleep)buf|=0x80;
	else buf&=0x7F;
	//尝试写数据
	if(!IP2366_WriteReg(buf,REG_SYSCTL9))return false;
	return true;	
	}

//IP2366强制进入低功耗睡眠模式
bool IP2366_ForceEnterDeepSleep(void)	
	{
	char buf;
	//读取SYS_CTL9
	if(!IP2366_ReadReg(&buf,REG_SYSCTL9))return false;
	if(buf&0x80)buf|=0x40; //如果EN_SYS_Standby=1，则令Enter_Standby=1，强制系统进入睡眠
	//尝试写数据
	if(!IP2366_WriteReg(buf,REG_SYSCTL9))return false;
	return true;
	}

//IP2366设置停充电流和再充电阈值
bool IP2366_SetReChargeParam(ReChargeConfig Vrecharge,IStopConfig IStop)
	{
	char buf;
	//尝试读取寄存器
	if(!IP2366_ReadReg(&buf,REG_SYSCTL8))return false;
	//进行寄存器bit的处理
	buf&=0x03;
	buf|=(char)(IStop&0x0F)<<4; //应用Istop[3:0]
	buf|=(char)(Vrecharge&0x03)<<2; //应用Vrch[1:0]
	//写数据
	if(!IP2366_WriteReg(buf,REG_SYSCTL8))return false;
	return true; 
	}	
	
//监测IP2366是否存在
bool IP2366_DetectIfPresent(void)
	{
	char buf;
	//尝试读取寄存器
	if(!IP2366_ReadReg(&buf,REG_SYSCTL0))return false;
	//返回结果
	return buf?true:false; 
	}
//更新充电电压(传入的单位为mV)
bool IP2366_UpdateFullVoltage(int Volt)
	{
	char buf;
	//设置最大充电电压
	if(Volt>4230)Volt=4230;
	else if(Volt<3600)Volt=3600;
	Volt=(Volt-2500)/10; //LSB=10mV,Base=2500mV
	buf=(char)Volt&0xFF;
	if(!IP2366_WriteReg(buf,REG_SYSCTL2))return false;
	//设置完毕
	return true;
	}	
	
//更新系统的充电输入（Sink模式）的功率
bool IP2366_UpdateSinkPower(ChargePowerDef Power)
	{
	char buf;
	//当前固件版本不支持该操作，返回true
	if(!CurrentIP2366FW->ExtendedTCSetting)return true;
//设置充电功率
	if(!IP2366_ReadReg(&buf,REG_SYSCTL12))return false;
	buf&=0xF0;	//除了Vbus_Src_Power以外其他bit统一mask为0
	buf|=0x08;  //令Chg_Power_En=1，单独设置充电功率
	buf|=((char)Power)&0x07;
	if(!IP2366_WriteReg(buf,REG_SYSCTL12))return false;
	//设置完毕
	return true;	
	}
	
//更新充放电功率
bool IP2366_UpdataChargePower(ChargePowerDef Power)
	{
	char buf;
	//设置充电功率
	if(!IP2366_ReadReg(&buf,REG_SYSCTL12))return false;
	buf&=0x1F;
	buf|=((char)Power)<<5;
	if(!IP2366_WriteReg(buf,REG_SYSCTL12))return false;
	//设置完毕
	return true;
	}	
	
//设置输入状态
bool IP2366_SetInputState(IP2366InputDef * Cfg,bool IsSetChargePower)
	{
	char buf;
	int Current;
	//设置充电器使能
  if(!IP2366_ReadReg(&buf,REG_SYSCTL0))return false;		
	if(Cfg->IsEnableCharger)buf|=0x01;
	else buf&=0xFE; //设置En_Charger bit
	if(!IP2366_WriteReg(buf,REG_SYSCTL0))return false;	
	//设置充电限流
	if(Cfg->ChargeCurrent>CurrentIP2366FW->IP2366ICCMAX)Current=CurrentIP2366FW->IP2366ICCMAX;
	else if(Cfg->ChargeCurrent<3000)Current=3000;
	else Current=Cfg->ChargeCurrent;
	Current/=100; //LSB=100mA
	buf=(char)(Current&0xFF);
	if(!IP2366_WriteReg(buf,REG_SYSCTL3))return false;
	//设置涓流充电电流
	if(Cfg->PreChargeCurrent>2000)Current=2000;
	else if(Cfg->PreChargeCurrent<100)Current=100;
	else Current=Cfg->PreChargeCurrent;
	Current/=50; //LSB=50mA
	if(!IP2366_WriteReg(buf,REG_SYSCTL6))return false;
	//设置最大充电电压
	if(!IP2366_UpdateFullVoltage(Cfg->FullVoltage))return false;
	//设置充电功率
  if(!IsSetChargePower)return true; //特定场合要禁止设置充放电功率
	if(!IP2366_UpdataChargePower(Cfg->ChargePower))return false;
	//设置完毕
	return true;
	}	

//根据系统充放电状态智能更新涓流电压的函数
void IP2366_DynamicUpdateVlow(VBatLowDef Vlow)
	{
	char buf;
	VBatLowDef CurrentVlow;
	//获取当前电流参数
	if(!IP2366_ReadReg(&buf,REG_SYSCTL10))return;
	buf=(buf&0xE0)>>5;
	CurrentVlow=(VBatLowDef)(buf&0x07);
  //当前系统电流参数和预期值不一致，更新Vlow
  if(CurrentVlow!=Vlow)IP2366_SetVLowVolt(Vlow);
	}	
	
//内置非阻塞轮询功能的电流设置函数
void IP2366_SetICCMax(int TargetCurrent)	
	{
	char buf,buf2;
	int Current;
	//进行限流值计算
	if(TargetCurrent>CurrentIP2366FW->IP2366ICCMAX)Current=CurrentIP2366FW->IP2366ICCMAX;
	else if(TargetCurrent<3000)Current=3000;
	else Current=TargetCurrent;
	Current/=100; //LSB=100mA
	buf=(char)(Current&0xFF);
	//读取电流，如果不一样则反复覆写直到一样
	if(!IP2366_ReadReg(&buf2,REG_SYSCTL3))return;
	if(buf2!=buf)IP2366_WriteReg(buf,REG_SYSCTL3);
	}

//获取SYSCTL3所设置的峰值电流
bool IP2366_GetCurrentPeakCurrent(int *Result)
	{
	int buf2;
	char buf;
	//读取参数	
	if(!IP2366_ReadReg(&buf,REG_SYSCTL3))return false;
	//换算
	buf2=(int)buf&0xFF;
	buf2*=100;
	if(Result!=NULL)*Result=buf2;
	//换算成功返回结果
	return true;
	}	
	
//获取IP2366是否为输入连接
bool IP2366_GetIfInputConnected(void)
	{
	char buf;
	if(!IP2366_ReadReg(&buf,REG_TYPEC_STATE))return false; //读取TypeC状态寄存器
	if(buf&0x90)return true;
	//其余情况返回false
	return false;
	}	

//获取VBUS是否有输入电压（该函数仅能在安全模式下使用）
bool GetIfVBUSHasSinkVolt(void)
	{
	char buf;
	int Volt;
	//按顺序读取并计算电压
	if(!IP2366_ReadReg(&buf,REG_VSYS_LSB))return false; 
	Volt=(int)buf;
	Volt&=0xFF;
	if(!IP2366_ReadReg(&buf,REG_VSYS_MSB))return false;
	Volt|=((int)buf)<<8;
	Volt&=0xFFFF;            //拼合电压值
	//检测电压值
	if(Volt>4550)return true;  //系统正在充电中
	//其余情况返回False
	return false;
	}	
	
//获取IP2366的C口是否已连接
bool IP2366_GetIfCPortConnected(void)
	{
	char buf;
	if(!IP2366_ReadReg(&buf,REG_TYPEC_STATE))return false; //读取TypeC状态寄存器
	if(buf&0xF0)return true;
	//其余情况返回false
	return false;
	}	
	
//2366使能或者除能芯片的充放电模块
bool IP2366_EnableDCDC(bool IsEnableCharger,bool IsEnableDischarge)	
	{
	char buf;
	//设置充电器
	if(!IP2366_ReadReg(&buf,REG_SYSCTL0))return false;		
	if(IsEnableCharger)buf|=0x01;
	else buf&=0xFE; //设置En_Charger bit
	if(!IP2366_WriteReg(buf,REG_SYSCTL0))return false;	
	//设置放电系统
	if(!IP2366_ReadReg(&buf,REG_SYSCTL11))return false;
	if(IsEnableDischarge)buf|=0x80;
	else buf&=0x7F; //设置EN-DCDCOutput
	if(!IP2366_WriteReg(buf,REG_SYSCTL11))return false;
	//处理完毕，返回True
	return true;
	}

//禁止充电器
bool IP2366_DisableCharger(void)	
	{
	char buf;
	//设置充电器
	if(!IP2366_ReadReg(&buf,REG_SYSCTL0))return false;		
	buf&=0xFE; //设置En_Charger bit=0
	if(!IP2366_WriteReg(buf,REG_SYSCTL0))return false;	
	//成功，返回true
	return true;
	}
//设置输出状态
bool IP2366_SetOutputState(IP2366OutConfigDef * CFG)
	{
	char buf;
	//设置输出使能寄存器
	if(!IP2366_ReadReg(&buf,REG_SYSCTL11))return false;
	if(CFG->IsEnableOutput)buf|=0x80;
	else buf&=0x7F; //设置EN-DCDCOutput
	if(CFG->IsEnableDPDMOut)buf|=0x40;
	else buf&=0xBF; //设置EN-Vbus_SRC_DPDM
	if(CFG->IsEnableDPDMOut)buf|=0x20;
	else buf&=0xDF; //设置EN-Vbus_SRC_PDO
	if(CFG->IsEnableDPDMOut)buf|=0x10;
	else buf&=0xEF; //设置EN-Vbus_SRC_SCP	
	//仅在支持这个HSCP设置bit的固件上尝试操作bit3
  if(CurrentIP2366FW->IsHSCPCapable)
		{		
		if(CFG->IsEnableHSCPOut)buf|=0x08;
		else buf&=0xF7; //设置EN-Vbus_SRC_HSCP	
		}
	//寄存器调整完毕进行回写
	if(!IP2366_WriteReg(buf,REG_SYSCTL11))return false;
	//所有东西设置完毕，返回1
	return true;
	}

//获取输入广播的list状态
bool IP2366_GetRecvPDOList(RecvPDOListDef *Result)
	{
	char buf;
	//读取RECV PDO
	if(!IP2366_ReadReg(&buf,REG_RECEIVED_PDO))return false;	
	buf&=0x1F; //去除掉无效位
	Result->PDO5VOK=buf&0x01?true:false;
	Result->PDO9VOK=buf&0x02?true:false;
	Result->PDO12VOK=buf&0x04?true:false;
	Result->PDO15VOK=buf&0x08?true:false;
	Result->PDO20VOK=buf&0x10?true:false;
	//计算完毕返回true
	return true;
	}	
	
//获取输入的PDO状态
bool IP2366_GetRecvPDO(RecvPDODef *PDOResult)
	{
	char buf;
	//读取输入寄存器		
	if(!IP2366_ReadReg(&buf,REG_TYPEC_STATE))return false; //读取TypeC
	if((buf&0x90)!=0x90)//Type-C处于SNK模式且未握手
		{
		*PDOResult=RecvPDO_None;
		return true;
		}
	//读取RECV PDO
	if(!IP2366_ReadReg(&buf,REG_RECEIVED_PDO))return false;	
	buf&=0x1F; //去除掉无效位
	if(buf&0x10)*PDOResult=RecvPDO_20V;
  else if(buf&0x08)*PDOResult=RecvPDO_15V;		
	else if(buf&0x04)*PDOResult=RecvPDO_12V;		
	else if(buf&0x02)*PDOResult=RecvPDO_9V;				
	else if(buf&0x01)*PDOResult=RecvPDO_5V;		
  else *PDOResult=RecvPDO_None;	
	//计算完毕返回true
	return true;
	}

//设置充电节低电压保护
bool IP2366_SetVLowVolt(VBatLowDef Vlow)
	{
	char buf;
	//获取状态
	if(!IP2366_ReadReg(&buf,REG_SYSCTL10))return false;
	buf&=0x1F;
	buf|=((char)Vlow&0x07)<<5;
	if(!IP2366_WriteReg(buf,REG_SYSCTL10))return false;
	//设置成功返回true
	return true;
	}	

//IP2366设置OTP重载监测的寄存器
bool IP2366_SetOTPSign(void)	
	{
	char buf;
	if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL9))return false;	
	//令不怎么用到EN_5VPDO_Iset位置位为1，用于监测芯片复位（该位会在芯片复位后自动reset为0）
	buf|=0x01;	
	if(!IP2366_WriteReg(buf,REG_TYPEC_CTL9))return false;
	//设置Sign成功
	return true;
	}
	
//监测芯片是否复位
bool IP2366_DetectIfChipReset(bool *IsReset)
	{
	char buf;
	if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL9))return false;
	//寄存器内容发生更改，芯片已经复位	
	buf&=0x01; //Mask掉除了EN 5V PDO Iset之外的其他位
	*IsReset=buf==0x01?false:true;
	return true;
	}	

//设置固定模式PDO的输出电流（需要注意的是20V的PDO如果不是公版芯片不建议设置）
bool IP2366_SetFixedPDO(IP2366FixPDOSetDef *Cfg)
	{
	char buf;
	int buf2,V20Max;
	char PDOPlus10mA;	
	//读取TYPEC_CTL18寄存器
	if(!IP2366_ReadReg(&PDOPlus10mA,REG_TYPEC_CTL18))return false;	
	//读取TYPEC_CTL9寄存器,设置对应的位并且回写
	if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL9))return false;
	
	if(Cfg->IsEnable20VPDOSet)buf|=0x10;
	else buf&=0xEF;		//设置En_20VPdo_Iset
	
	if(Cfg->IsEnable15VPDOSet)buf|=0x08;
	else buf&=0xF7;		//设置En_15VPdo_Iset
	
	if(Cfg->IsEnable12VPDOSet)buf|=0x04;
	else buf&=0xFB; 	//设置En_12VPdo_Iset		
	
	if(Cfg->IsEnable9VPDOSet)buf|=0x02;
	else buf&=0xFD; 	//设置En_9VPdo_Iset	
  if(!IP2366_WriteReg(buf,REG_TYPEC_CTL9))return false; 		
	
	//设置TypeC_CTL14寄存器的20VPdo_Iset[7:0]
	if(Cfg->IsEnable20VPDOSet)
		{
		//对传入的PDO参数进行数值限幅
		V20Max=CurrentIP2366FW->IsExtendPDOCapable?7000:4000;
		if(Cfg->PDO20VICCMAX>V20Max)buf2=V20Max;
		else if(Cfg->PDO20VICCMAX<1000)buf2=1000;
		else buf2=Cfg->PDO20VICCMAX; 
	
		if(!CurrentIP2366FW->IsExtendPDOCapable&&buf2%20)PDOPlus10mA|=0x10;
    else PDOPlus10mA&=0xEF;        //如果是公版固件且检测到电流包含奇数部分，则设置EN_20VPDO_ADD=1凑出10mA				
			
		if(CurrentIP2366FW->IsExtendPDOCapable)buf2=(buf2/50)&0xFF; //非公版芯片，原始电流值转换为50mA per LSB的unsigned int
		else buf2=(buf2/20)&0xFF; //原始电流值转换为20mA per LSB的unsigned int
		buf=(char)buf2;
		if(!IP2366_WriteReg(buf,REG_TYPEC_CTL14))return false; 
		}	
	//设置TypeC_CTL13寄存器的15VPdo_Iset[7:0]
	if(Cfg->IsEnable15VPDOSet)
		{
		if(Cfg->PDO15VICCMAX>3000)buf2=3000;
		else if(Cfg->PDO15VICCMAX<500)buf2=500;
		else buf2=Cfg->PDO15VICCMAX;  //进行数值限幅确保读进来的数值不超过0.5A-3A范围
	  
		if(buf2%20)PDOPlus10mA|=0x08;
    else PDOPlus10mA&=0xF7;        //如果检测到电流包含奇数部分，则设置EN_15VPDO_ADD=1凑出10mA
			
		buf2=(buf2/20)&0xFF; //原始电流值转换为20mA per LSB的unsigned int
		buf=(char)buf2;
		if(!IP2366_WriteReg(buf,REG_TYPEC_CTL13))return false; 
		}		
	//设置TypeC_CTL12寄存器的12VPdo_Iset[7:0]
	if(Cfg->IsEnable12VPDOSet)
		{
		if(Cfg->PDO12VICCMAX>3000)buf2=3000;
		else if(Cfg->PDO12VICCMAX<500)buf2=500;
		else buf2=Cfg->PDO12VICCMAX;  //进行数值限幅确保读进来的数值不超过0.5A-3A范围
	
		if(buf2%20)PDOPlus10mA|=0x04;
    else PDOPlus10mA&=0xFB;        //如果检测到电流包含奇数部分，则设置EN_12VPDO_ADD=1凑出10mA			
			
		buf2=(buf2/20)&0xFF; //原始电流值转换为20mA per LSB的unsigned int
		buf=(char)buf2;
		if(!IP2366_WriteReg(buf,REG_TYPEC_CTL12))return false; 
		}	
	//设置TypeC_CTL11寄存器的9VPdo_Iset[7:0]
	if(Cfg->IsEnable9VPDOSet)
		{
		if(Cfg->PDO9VICCMAX>3000)buf2=3000;
		else if(Cfg->PDO9VICCMAX<500)buf2=500;
		else buf2=Cfg->PDO9VICCMAX;  //进行数值限幅确保读进来的数值不超过0.5A-3A范围
	
		if(buf2%20)PDOPlus10mA|=0x02;
    else PDOPlus10mA&=0xFD;        //如果检测到电流包含奇数部分，则设置EN_9VPDO_ADD=1凑出10mA				
			
		buf2=(buf2/20)&0xFF; //原始电流值转换为20mA per LSB的unsigned int
		buf=(char)buf2;
		if(!IP2366_WriteReg(buf,REG_TYPEC_CTL11))return false; 
		}	
	//写入TYPEC_CTL18寄存器设置10mA电流递增功能
	if(!IP2366_WriteReg(PDOPlus10mA,REG_TYPEC_CTL18))return false;		
	//所有通信完成，返回true	
	return true;
	}

//获取芯片PPS1和PPS2的输出电流
bool IP2366_GetPPSCurrent(int *PPS1Current,int *PPS2Current)	
	{
	char buf;
	int buf2;
	//获取PPS1电流	
	if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL23))return false; //获取Pps1Pdo_Iset[6:0]
	buf2=(int)buf;
	buf2&=0x7F;
	buf2*=50;  //转换为mA
	if(PPS1Current!=NULL)*PPS1Current=buf2;
	//获取PPS2电流	
	if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL24))return false; //获取Pps2Pdo_Iset[6:0]
	buf2=(int)buf;
	buf2&=0x7F;
	buf2*=50;  //转换为mA
	if(PPS2Current!=NULL)*PPS2Current=buf2;	
	//通信完毕，返回结果
	return true;
	}

//设置芯片PPS1和PPS2的输出电流
bool IP2366_SetPPSCurrent(IP2366PPSPDOSetDef *Cfg)	
	{
	char buf;
	int buf2;
	int ppsimax=CurrentIP2366FW->IsExtendPDOCapable?6350:3000;
	//根据是否使能PPS电流修改设置
	if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL9))return false;
	if(Cfg->IsEnablePPS1Set)buf|=0x20;
	else buf&=0xDF;  //设置En_Pps1Pdo_Iset
	if(Cfg->IsEnablePPS2Set)buf|=0x40;
  else buf&=0xBF;		
  if(!IP2366_WriteReg(buf,REG_TYPEC_CTL9))return false; 		
	//设置PPS1 PDO的输出电流
	if(Cfg->PPS1Current>ppsimax)buf2=ppsimax;
  else if(Cfg->PPS1Current<1500)buf2=1500;
  else buf2=Cfg->PPS1Current;  //限制数值范围为3A-6.35A
		
	buf2=(buf2/50)&0x7F; //原始值转换为50mA per LSB
	buf=(char)buf2;
	if(!IP2366_WriteReg(buf,REG_TYPEC_CTL23))return false; 	//写入Pps1Pdo_Iset[6:0]
	//设置PPS2 PDO的输出电流
	if(Cfg->PPS2Current>ppsimax)buf2=ppsimax;
  else if(Cfg->PPS2Current<2000)buf2=2000;
  else buf2=Cfg->PPS2Current;  //限制数值范围为2A-6.35A
		
	buf2=(buf2/50)&0x7F; //原始值转换为50mA per LSB
	buf=(char)buf2;
	if(!IP2366_WriteReg(buf,REG_TYPEC_CTL24))return false; 	//写入Pps2Pdo_Iset[6:0]
	//通信完毕，返回结果
	return true;
	}

//设置TypeC的模式
bool IP2366_SetTypeCRole(TypeCRoleDef Role)
	{
	char buf;
	//获取状态
	if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL8))return false;
	buf&=0x3F;
	if(CurrentIP2366FW->ExtendedTCSetting&&Role==TypeC_NoConnect)buf|=0x80; //如果固件支持Type-C真断联模式，则直接映射Type-C No Connect到2'b10
	else buf|=((char)Role&0x03)<<6;  //正常根据角色去设置bit
	if(!IP2366_WriteReg(buf,REG_TYPEC_CTL8))return false;
	//设置成功返回true
	return true;
	}	

//检测IP2366锁死
static unsigned char ChipLockUpTime=0;	
	
void IP2366_LockUpDetect(void)
	{
	char buf,buf2;
	//读取结果
	if(!IP2366_ReadReg(&buf2,REG_STATE_CTL2))return;
	if(!IP2366_ReadReg(&buf,REG_STATE_CTL0))return;  //读取STATE-CTL0和STATE-CTL2
	//当前Vbus反馈没电但是电池状态异常，说明芯片死机了
	if(!(buf2&0x80)&&(buf&0x20))
		{
		if(ChipLockUpTime>=18)
			{
			ChipLockUpTime=0;
			IP2366_WriteReg(0x40,REG_SYSCTL0); //芯片死机，强制写MCU复位bit
			}
		//时间没到继续累加
		else ChipLockUpTime++;
		}
	else ChipLockUpTime=0;
	}	
	
//获取充电状态	
bool IP2366_GetChargerState(BatteryStateDef *State)	
	{
	char buf,buf2;
	float Result,IMin;
	BatteryStateDef temp;
	bool IsEnteredCVMode=false;
	//获取状态
	if(!IP2366_ReadReg(&buf2,REG_STATE_CTL2))return false;
	if(!IP2366_ReadReg(&buf,REG_STATE_CTL0))return false;  //读取STATE-CTL0和STATE-CTL2
	if(!(buf2&0x80))temp=Batt_StandBy; 			//VBUS都没电哪来的待机状态
	else if(buf&0x08)temp=Batt_discharging; //输出已启用，电池正在向外放电
	else if(buf&0x20)
		{
		//获取充电状态
		temp=(BatteryStateDef)(buf&0x07); //当CHGEN=1的时候获取电池充电状态
		if(!IP2366_getCurrentChargeParam(NULL,&Result))return false; //获取浮充电压
		Result-=0.2; //比目标浮充电压低0.2V作为恒流充电判断条件
		//获取充电功率
		if(!IP2366_ReadReg(&buf,REG_SYSCTL12))return false;
		buf>>=5;
		buf&=0x07;
		switch((ChargePowerDef)buf)	
			{
			case Power_30W:IMin=1.50;break;
			case Power_45W:IMin=2.50;break;
			case Power_60W:
			case Power_65W:IMin=4.0;break;
			case Power_100W:IMin=5.5;break;
			case Power_140W:IMin=8.2;break;
			}
		//条件判断，电池电压达到额定满充电压且电池电流小于进入浮充的最小值，指示进入浮充
		if(ADCO.Vbatt>Result&&fabsf(ADCO.Ibatt)<IMin)IsEnteredCVMode=true;
		if(!IsEnteredCVMode&&temp==Batt_CVCharge)temp=Batt_CCCharge;
		}
	else temp=Batt_StandBy; //待机状态
	//获取成功
	if(State!=NULL)*State=temp; //赋值结果
	return true;
	}

//2366获取C口VBUS的电压
bool IP2366_GetVBUSVoltage(float *VBUS)
	{
	char buf;
	int ibuf;
	//获取VBUS电压
	if(!IP2366_ReadReg(&buf,REG_VSYS_LSB))return false;	
	ibuf=((int)buf)&0xFF;
	if(!IP2366_ReadReg(&buf,REG_VSYS_MSB))return false;		
	ibuf|=(int)(buf<<8);
  //获取成功返回结果		
	if(VBUS!=NULL)*VBUS=(float)ibuf/(float)1000; //换算为V
	return true;
	}	

//获取VBUS状态
bool IP2366_GetVBUSState(IP2366VBUSStateDef * State)
	{
	char buf,buf2;
	int ibuf;
	bool STAT;
	float PDVMax;
	RecvPDODef PDO;	
	//获取VBUS电压
	if(!IP2366_ReadReg(&buf,REG_VSYS_LSB))return false;	
	ibuf=((int)buf)&0xFF;
	if(!IP2366_ReadReg(&buf,REG_VSYS_MSB))return false;		
	ibuf|=(int)(buf<<8);	
	State->VBUSVolt=(float)ibuf/(float)1000; //换算为V
	//获取VBUS电流
	if(!IP2366_ReadReg(&buf,REG_ISYS_LSB))return false;	
	ibuf=((int)buf)&0xFF;
	if(!IP2366_ReadReg(&buf,REG_ISYS_MSB))return false;			
	ibuf|=(int)(buf<<8);	
	State->VBUSCurrent=(float)ibuf/(float)1000; //LSB=1mA，得到电流(A)
	State->VBUSCurrent/=BusCurrentCalFactor; //除以校准系数
	//获取是否处于放电状态，如果是则电流设置为负数
	if(!IP2366_ReadReg(&buf,REG_STATE_CTL0))return false;	
	if(buf&0x08)State->VBUSCurrent*=-1;
	//快充状态监测
	if(!IP2366_ReadReg(&buf,REG_TYPEC_STATE))return false; //读取TypeC
	if((buf&0x60)==0x60) //Type-C处于SRC模式且PD成功握手
	  {
		//读取SYS-CTL11检查快充是否激活
		if(!IP2366_ReadReg(&buf,REG_SYSCTL11))return false;
		if(buf&0x20) //PD已经启用
		    {
				STAT=true; //默认为真
				//读取TYPEC-CTL17
		    if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL17))return false;
				if(State->VBUSVolt>25.0) //28V
					 {
					 STAT=buf&0x10?true:false; 
				   State->PDState=PD_28VMode; 
		       }
				//根据电压进行判断
				else if(State->VBUSVolt>19.0) //20V
				   {
					 STAT=buf&0x10?true:false; 
				   State->PDState=PD_20VMode; 
		       }
		    else if(State->VBUSVolt>14.0) //15V
				   {
				   STAT=buf&0x08?true:false; 
					 State->PDState=PD_15VMode;
			     }
				else if(State->VBUSVolt>11.0) //12V
				   {
				   STAT=buf&0x04?true:false; 
				   State->PDState=PD_12VMode;
			     }
				else if(State->VBUSVolt>8.0) //9V
				   {
				   STAT=buf&0x02?true:false; 
					 State->PDState=PD_9VMode;
		       }
				else if(State->VBUSVolt>6.0)State->PDState=PD_7VMode;
				else State->PDState=PD_5VMode; //识别电压判断输出模式
		    if(STAT)State->QuickChargeState=State->VBUSVolt>6.0?QuickCharge_PD:QuickCharge_None; //当前电压模式所对应的PDO是开着的，如果电压大于6V则是PD模式	 
				else //当前电压模式所对应的PDO是关闭的，如果电压大于6V则是高压模式	 
				   {
					 State->PDState=PD_5VMode; //高压快充，指示非5V的PD挡位
				   State->QuickChargeState=State->VBUSVolt>6.0?QuickCharge_HV:QuickCharge_None; 
					 }
				}
		else State->QuickChargeState=State->VBUSVolt>6.0?QuickCharge_HV:QuickCharge_None; //如果PD已经被关闭，则判断为高压快充
		}
	else if((buf&0x90)==0x90)//Type-C处于SNK模式且PD成功握手
	  {
		if(!IP2366_ReadReg(&buf,REG_STATE_CTL2))return false; //STATE-CTL2
		if(IP2366_GetRecvPDO(&PDO)) //获取收到的PDO
			{
			//进行PDO对比
			buf&=0x07;
			if(PDO!=RecvPDO_None&&buf>0) //收到当前PDO并成功协商
				{
				State->PDState=(PDStateDef)buf;//填写enum值
				if(!IP2366_ReadReg(&buf,REG_SYSCTL0))return false; //临时读一下SYSCTL0		
				if(buf&0x08)
					{
					//固件支持Sink功率单独设置，根据当前设置的Sink功率判断是否为合法的PD电压
					if(CurrentIP2366FW->ExtendedTCSetting)	
						{
						if(!IP2366_ReadReg(&buf,REG_SYSCTL12))return false; //临时读一下SYSCTL12	
						//确定PDVmax
					  if(((ChargePowerDef)buf&0x07)==Power_140W)PDVMax=28.50;
						else PDVMax=20.50;
						State->QuickChargeState=State->VBUSVolt<PDVMax?QuickCharge_PD:QuickCharge_HV;
						}
					//固件不支持Sink单独设置，只要系统使能PD Sink就判断为PD模式
					else State->QuickChargeState=QuickCharge_PD;	
					}
				else if(State->VBUSVolt>6.0)State->QuickChargeState=QuickCharge_HV;  //VBUS电压高于6.0V，显示高压快充激活
				else if(State->VBUSVolt<=6.0&&State->VBUSVolt>4.0&&fabsf(State->VBUSCurrent)>2.4)State->QuickChargeState=QuickCharge_HC;//VBUS在4.0-6之间，低压大电流快充
				else State->QuickChargeState=QuickCharge_None; //其余状况均为未识别快充	
				}
			else if(State->VBUSVolt>6.0||buf>0)State->QuickChargeState=QuickCharge_HV;//没有PDO报文但是VBUS电压抬高，是高压快充
			else State->QuickChargeState=QuickCharge_None; //其余状况均为未识别快充
			}
		else State->QuickChargeState=QuickCharge_None; //其余状况均为未识别快充
		}
	else  //其余状况
	  {
		State->PDState=PD_5VMode;
		if(State->VBUSVolt>6.0||buf&0x04)State->QuickChargeState=QuickCharge_HV;//高压快充
		else if(State->VBUSVolt<=6.0&&State->VBUSVolt>4.0&&fabsf(State->VBUSCurrent)>2.4)State->QuickChargeState=QuickCharge_HC;//VBUS在4.0-6之间，低压大电流快充
		else State->QuickChargeState=QuickCharge_None; //其余状况均为未识别快充
		}
	//检查Type-C的状态	‘
	State->IsTypeCConnected=false; //默认为未连接状态
	if(!IP2366_ReadReg(&buf,REG_TYPEC_STATE))return false;
	if(!IP2366_ReadReg(&buf2,REG_STATE_CTL2))return false;  //读取State
	if(buf&0x60) //SRC-OK包括SRC-PD-OK
		{
		//已连接，但是要区分是什么模式
		if(!IP2366_ReadReg(&buf,REG_SYSCTL11))return false;	
		if(!(buf2&0x80))State->IsTypeCConnected=false; //Vbus_OK=0,Type C未连接	
		else State->IsTypeCConnected=buf&0x80?true:false;
		}		
	else if(buf2&0x80||buf&0x90)State->IsTypeCConnected=true; //Vbus_OK=1或者SNK-OK或SNK-PD-OK bit=1,Type C已连接
  //处理完毕，返回true
  return true;	
	}
	
//获取芯片硬件版本
IP2366HWRevDef IP2366_GetChipHWRev(void)
	{
	char buf;
	if(!IP2366_ReadReg(&buf,REG_MFR_ICREV))return Hardware_Ver_Unknown; //读取失败
	buf&=0x03;
	return (IP2366HWRevDef)buf;
	}
	
//IP2366根据固件版本获取芯片能力
bool IP2366_UpdateChipCap(char VendorString[5])
	{
	int j;
	for(j=0;j<FWTableSize;j++)if(!strncmp(IP2366FWTable[j].FWID,VendorString,5))
		{
		CurrentIP2366FW=&IP2366FWTable[j]; //标记当前系统使用的Table
		return true;
		}
	//找遍整个FWID Library都没有找到匹配的，说明固件不被支持，返回Failed
	return false;
	}
	
//获取IP2366芯片本身的温度数据
bool IP2366_GetChipTemp(char *TempOut,bool *IsTempLimitTriggered)
	{
	char buf;
	if(!IP2366_ReadReg(&buf,REG_IC_TEMP))return false;
	//开始进行数据处理
	if(IsTempLimitTriggered!=NULL)*IsTempLimitTriggered=buf&0x80?true:false;
	buf&=0x7F; //去掉Temp位
	if(TempOut!=NULL)*TempOut=buf;
	//操作成功返回true
	return true;
	}
