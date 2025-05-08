#include "I2C.h"
#include "I2CAddr.h"
#include "delay.h"
#include "IP2366_REG.h"
#include <math.h>

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
	
//更新充电功率
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
bool IP2366_SetInputState(IP2366InputDef * Cfg)
	{
	char buf;
	int Current;
	extern bool IsEnable17AMode;
	//设置充电器使能
  if(!IP2366_ReadReg(&buf,REG_SYSCTL0))return false;		
	if(Cfg->IsEnableCharger)buf|=0x01;
	else buf&=0xFE; //设置En_Charger bit
	if(!IP2366_WriteReg(buf,REG_SYSCTL0))return false;	
	//设置充电限流
	if(Cfg->ChargeCurrent>IsEnable17AMode?IP2366_ICCMAX:9700)Current=IsEnable17AMode?IP2366_ICCMAX:9700;
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
	if(!IP2366_UpdataChargePower(Cfg->ChargePower))return false;
	//设置完毕
	return true;
	}	
	
//内置非阻塞轮询功能的电流设置函数
void IP2366_SetICCMax(int TargetCurrent)	
	{
	char buf,buf2;
	int Current;
	extern bool IsEnable17AMode;
	//进行限流值计算
	if(TargetCurrent>IsEnable17AMode?IP2366_ICCMAX:9700)Current=IsEnable17AMode?IP2366_ICCMAX:9700;
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

//设置输出状态
bool IP2366_SetOutputState(IP2366OutConfigDef * CFG)
	{
	char buf;
	extern bool IsEnableHSCPMode;
	//设置Type-C模式
  if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL8))return false;
	buf&=0x3F;
	buf|=CFG->IsEnableOutput?0xC0:0x40; //设置Type-C模式为DFP或DRP
	if(!IP2366_WriteReg(buf,REG_TYPEC_CTL8))return false;
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
  if(IsEnableHSCPMode)
		{		
		if(CFG->IsEnableHSCPOut)buf|=0x08;
		else buf&=0xF7; //设置EN-Vbus_SRC_HSCP	
		}
	//寄存器调整完毕进行回写
	if(!IP2366_WriteReg(buf,REG_SYSCTL11))return false;
	//所有东西设置完毕，返回1
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
	//向不怎么会用到的Type-C CTL9写0x01用于监测
	if(!IP2366_WriteReg(0x01,REG_TYPEC_CTL9))return false;
	//设置Sign成功
	return true;
	}
	
//监测芯片是否复位
bool IP2366_DetectIfChipReset(bool *IsReset)
	{
	char buf;
	if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL9))return false;
	//寄存器内容发生更改，芯片已经复位	
	*IsReset=buf==0x01?false:true;
	return true;
	}	

//设置TypeC的模式
bool IP2366_SetTypeCRole(TypeCRoleDef Role)
	{
	char buf;
	//获取状态
	if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL8))return false;
	buf&=0x3F;
	buf|=((char)Role&0x03)<<6;
	if(!IP2366_WriteReg(buf,REG_TYPEC_CTL8))return false;
	//设置成功返回true
	return true;
	}	
	
//获取充电状态
bool IP2366_GetChargerState(BatteryStateDef *State)	
	{
	char buf;
	//获取状态
	if(!IP2366_ReadReg(&buf,REG_STATE_CTL0))return false; //STATE-CTL0
	if(buf&0x08)*State=Batt_discharging; //输出已启用，电池正在向外放电
	else if(buf&0x20)*State=(BatteryStateDef)(buf&0x07); //当CHGEN=1的时候获取电池充电状态
	else *State=Batt_StandBy; //待机状态
	//获取成功
	return true;
	}

//获取VBUS状态
bool IP2366_GetVBUSState(IP2366VBUSStateDef * State)
	{
	char buf,buf2;
	int ibuf;
	bool STAT;
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
				State->QuickChargeState=QuickCharge_PD; //收到PDO报文且当前充电电压符合要求
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
