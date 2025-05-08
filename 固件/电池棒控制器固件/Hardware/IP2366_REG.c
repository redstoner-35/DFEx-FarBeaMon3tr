#include "I2C.h"
#include "I2CAddr.h"
#include "delay.h"
#include "IP2366_REG.h"
#include <math.h>

//���Ĵ���
static bool IP2366_ReadReg(char *Data,IP2366REGDef Reg)
	{
	//��ʼͨ��,���͵�ַ
	IIC_Start();
	IIC_Send_Byte(IP2366ADDR);
	if(IIC_Wait_Ack())return false;
	delay_us(60); //�����ֲ���Ҫ��ACK֮����Ҫ��ʱ����50uS�ȴ�оƬ׼������
	//���ͼĴ����������
	IIC_Send_Byte((char)Reg);
	if(IIC_Wait_Ack())return false; //���ͼĴ�����
	delay_us(60); //�����ֲ���Ҫ��ACK֮����Ҫ��ʱ����50uS�ȴ�оƬ׼������
	//�������������͵�ַ׼����ȡ����
	IIC_Start();
	IIC_Send_Byte(IP2366ADDR+1);
	if(IIC_Wait_Ack())return false;
	delay_us(60); //�����ֲ���Ҫ��ACK֮����Ҫ��ʱ����50uS�ȴ�оƬ׼������
	*Data=IIC_Read_Byte(0); //����1�ֽں���NACK
	//����֮����Stop
	IIC_Stop();
	return true;
	}

//д�Ĵ���
static bool IP2366_WriteReg(char Data,IP2366REGDef Reg)
	{
	//��ʼͨ��,���͵�ַ
	IIC_Start();
	IIC_Send_Byte(IP2366ADDR);
	if(IIC_Wait_Ack())return false;
	delay_us(60); //�����ֲ���Ҫ��ACK֮����Ҫ��ʱ����50uS�ȴ�оƬ׼������
	//���ͼĴ����������
	IIC_Send_Byte((char)Reg);
	if(IIC_Wait_Ack())return false; //���ͼĴ�����
	delay_us(60); //�����ֲ���Ҫ��ACK֮����Ҫ��ʱ����50uS�ȴ�оƬ׼������
	IIC_Send_Byte(Data); //��������
	if(IIC_Wait_Ack())return false; //���ͼĴ�����
	//ͨ�Ž���
	IIC_Stop();
	return true;
	}

//��ȡһ����оƬ����Ϣ
bool IP2366_ReadChipState(ChipStatDef *State)
	{
	char buf;
	//��ȡSTATE CTL2
	if(!IP2366_ReadReg(&buf,REG_STATE_CTL2))return false;
	if(!(buf&0x80))State->VBusState=VBUS_NoPower;
	else if(buf&0x40)State->VBusState=VBUS_OverVolt;
	else State->VBusState=VBUS_Normal;
	//��ȡSTATE CTL3
	if(!IP2366_ReadReg(&buf,REG_STATE_CTL3))return false;
	if(buf&0x10)State->VSysState=VSys_State_Short;
	else if(buf&0x20)State->VSysState=VSys_State_OCP;
	else State->VSysState=VSys_State_Normal;
	//��ȡ��Ϸ���True
	return true;
	}	
	
//���OCP Flag
void IP2366_ClearOCFlag(void)
	{
	//��OC Bitд1��0
	IP2366_WriteReg(0x30,REG_STATE_CTL3);
	}	
	
//�Կ��ؼ�ͳ��״̬��������
const BatteryStateDef NotAccState[4]={Batt_StandBy,Batt_ChgWait,Batt_ChgDone,Batt_ChgError};	

bool IP2366_QueryCurrentStateIsACC(BatteryStateDef IN)
	{
	char i;
	for(i=0;i<4;i++)if(IN==NotAccState[i])return false;
	//״̬������ؼƿ�ʼͳ��
	return true;
	}
	
//����PDO�㲥
bool IP2366_SetPDOBroadCast(PDOBroadcastDef *PDOCfg)
	{
	char buf;
	//��ȡTYPEC-CTL17
	if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL17))return false;
	//����PPS2
	if(PDOCfg->EnablePPS2)buf|=0x40;
	else buf&=(~0x40);
	//����PPS1
	if(PDOCfg->EnablePPS1)buf|=0x20;
	else buf&=(~0x20);
	//����20VPDO
	if(PDOCfg->Enable20V)buf|=0x10;
	else buf&=(~0x10);
	//����15V PDO
	if(PDOCfg->Enable15V)buf|=0x08;
	else buf&=(~0x08);
	//����12V PDO
	if(PDOCfg->Enable12V)buf|=0x04;
	else buf&=(~0x04);
	//����9V PDO
	if(PDOCfg->Enable9V)buf|=0x02;
	else buf&=(~0x02);
  //�ѽ��д��ȥ
	if(!IP2366_WriteReg(buf,REG_TYPEC_CTL17))return false;
  //�ɹ��������
	return true;
	}	
	
//��ȡʱ���
bool IP2366_GetFirmwareTimeStamp(char TimeStamp[5])
	{
	char i;
	//��ѯ��ȡ�Ĵ���
	for(i=0;i<5;i++)
		{
		//��ʼͨ��,���͵�ַ
		IIC_Start();
		IIC_Send_Byte(IP2366ADDR);
		if(IIC_Wait_Ack())return false;
		delay_us(60); //�����ֲ���Ҫ��ACK֮����Ҫ��ʱ����50uS�ȴ�оƬ׼������
		//���ͼĴ����������
		IIC_Send_Byte(0x69+i);
		if(IIC_Wait_Ack())return false; //���ͼĴ�����
		delay_us(60); //�����ֲ���Ҫ��ACK֮����Ҫ��ʱ����50uS�ȴ�оƬ׼������
		//�������������͵�ַ׼����ȡ����
		IIC_Start();
		IIC_Send_Byte(IP2366ADDR+1);
		if(IIC_Wait_Ack())return false;
		delay_us(60); //�����ֲ���Ҫ��ACK֮����Ҫ��ʱ����50uS�ȴ�оƬ׼������
		TimeStamp[i]=IIC_Read_Byte(0); //����1�ֽں���NACK
		//����֮����Stop
		IIC_Stop();
		//����һ���ֽڣ���ʱһ���ٶ�
		delay_us(60);
		}
	//��ȡ�ɹ������ؽ��
	return true;
	}	

//��ȡ��ǰоƬ�ĳ�����
bool IP2366_getCurrentChargeParam(int *Istop,float *Vstop)
	{
	char buf;
	int result;
	float Fbuf;
	//���Զ�ȡ�Ĵ�����ȡͣ�����
	if(!IP2366_ReadReg(&buf,REG_SYSCTL8))return false;
	result=(int)buf;
	result=(result&0xF0)>>4; //mask�������bit����ֻ����Istop[3:0]
	result*=50; //50mA-per LSB
	if(Istop!=NULL)*Istop=result;
	//���Զ�ȡ�Ĵ�����ȡͣ���ѹ
	if(!IP2366_ReadReg(&buf,REG_SYSCTL2))return false;
	Fbuf=(float)buf;
	Fbuf=(Fbuf*10)+2500; //LSB=10mV,Base=2500mV
	Fbuf=(Fbuf/1000)*BATTCOUNT; //mVתV�����Ե�ؽ����õ�ʵ�ʵ�ѹ
	if(Vstop!=NULL)*Vstop=Fbuf;
	//�������
	return true;
	}
	
//��ȡ�ٳ���ѹ
bool IP2366_GetVRecharge(float *Vrecharge)
	{
	char buf;
	float Fbuf;
	//���Զ�ȡ�Ĵ�����ȡͣ���ѹ
	if(!IP2366_ReadReg(&buf,REG_SYSCTL2))return false;
	Fbuf=(float)buf;
	Fbuf=(Fbuf*10)+2500; //LSB=10mV,Base=2500mV
	Fbuf=(Fbuf/1000)*BATTCOUNT; //mVתV�����Ե�ؽ����õ�ʵ�ʵ�ѹ
	//���Զ�ȡ�Ĵ�����ȡ�ٳ������
	if(!IP2366_ReadReg(&buf,REG_SYSCTL8))return false;
  buf=(buf>>2)&0x03; //mask���ٳ�繦��
	switch(buf)
		{
		case 0:Fbuf=-1;break; //�ٳ��ر�
		case 1:Fbuf-=(float)(BATTCOUNT*0.05);break; //�ٳ��Ϊÿ��-0.05V
		case 2:Fbuf-=(float)(BATTCOUNT*0.1);break; //�ٳ��Ϊÿ��-0.1V
		case 3:Fbuf-=(float)(BATTCOUNT*0.2);break; //�ٳ��Ϊÿ��-0.2V
		}		
	if(Vrecharge!=NULL)*Vrecharge=Fbuf;	
  //��ȡ�ɹ�
	return true;
	}
	
//IP2366����ͣ��������ٳ����ֵ
bool IP2366_SetReChargeParam(ReChargeConfig Vrecharge,IStopConfig IStop)
	{
	char buf;
	//���Զ�ȡ�Ĵ���
	if(!IP2366_ReadReg(&buf,REG_SYSCTL8))return false;
	//���мĴ���bit�Ĵ���
	buf&=0x03;
	buf|=(char)(IStop&0x0F)<<4; //Ӧ��Istop[3:0]
	buf|=(char)(Vrecharge&0x03)<<2; //Ӧ��Vrch[1:0]
	//д����
	if(!IP2366_WriteReg(buf,REG_SYSCTL8))return false;
	return true; 
	}	
	
//���IP2366�Ƿ����
bool IP2366_DetectIfPresent(void)
	{
	char buf;
	//���Զ�ȡ�Ĵ���
	if(!IP2366_ReadReg(&buf,REG_SYSCTL0))return false;
	//���ؽ��
	return buf?true:false; 
	}
//���³���ѹ(����ĵ�λΪmV)
bool IP2366_UpdateFullVoltage(int Volt)
	{
	char buf;
	//����������ѹ
	if(Volt>4230)Volt=4230;
	else if(Volt<3600)Volt=3600;
	Volt=(Volt-2500)/10; //LSB=10mV,Base=2500mV
	buf=(char)Volt&0xFF;
	if(!IP2366_WriteReg(buf,REG_SYSCTL2))return false;
	//�������
	return true;
	}	
	
//���³�繦��
bool IP2366_UpdataChargePower(ChargePowerDef Power)
	{
	char buf;
	//���ó�繦��
	if(!IP2366_ReadReg(&buf,REG_SYSCTL12))return false;
	buf&=0x1F;
	buf|=((char)Power)<<5;
	if(!IP2366_WriteReg(buf,REG_SYSCTL12))return false;
	//�������
	return true;
	}	
	
//��������״̬
bool IP2366_SetInputState(IP2366InputDef * Cfg)
	{
	char buf;
	int Current;
	extern bool IsEnable17AMode;
	//���ó����ʹ��
  if(!IP2366_ReadReg(&buf,REG_SYSCTL0))return false;		
	if(Cfg->IsEnableCharger)buf|=0x01;
	else buf&=0xFE; //����En_Charger bit
	if(!IP2366_WriteReg(buf,REG_SYSCTL0))return false;	
	//���ó������
	if(Cfg->ChargeCurrent>IsEnable17AMode?IP2366_ICCMAX:9700)Current=IsEnable17AMode?IP2366_ICCMAX:9700;
	else if(Cfg->ChargeCurrent<3000)Current=3000;
	else Current=Cfg->ChargeCurrent;
	Current/=100; //LSB=100mA
	buf=(char)(Current&0xFF);
	if(!IP2366_WriteReg(buf,REG_SYSCTL3))return false;
	//�������������
	if(Cfg->PreChargeCurrent>2000)Current=2000;
	else if(Cfg->PreChargeCurrent<100)Current=100;
	else Current=Cfg->PreChargeCurrent;
	Current/=50; //LSB=50mA
	if(!IP2366_WriteReg(buf,REG_SYSCTL6))return false;
	//����������ѹ
	if(!IP2366_UpdateFullVoltage(Cfg->FullVoltage))return false;
	//���ó�繦��
	if(!IP2366_UpdataChargePower(Cfg->ChargePower))return false;
	//�������
	return true;
	}	
	
//���÷�������ѯ���ܵĵ������ú���
void IP2366_SetICCMax(int TargetCurrent)	
	{
	char buf,buf2;
	int Current;
	extern bool IsEnable17AMode;
	//��������ֵ����
	if(TargetCurrent>IsEnable17AMode?IP2366_ICCMAX:9700)Current=IsEnable17AMode?IP2366_ICCMAX:9700;
	else if(TargetCurrent<3000)Current=3000;
	else Current=TargetCurrent;
	Current/=100; //LSB=100mA
	buf=(char)(Current&0xFF);
	//��ȡ�����������һ���򷴸���дֱ��һ��
	if(!IP2366_ReadReg(&buf2,REG_SYSCTL3))return;
	if(buf2!=buf)IP2366_WriteReg(buf,REG_SYSCTL3);
	}

//��ȡSYSCTL3�����õķ�ֵ����
bool IP2366_GetCurrentPeakCurrent(int *Result)
	{
	int buf2;
	char buf;
	//��ȡ����	
	if(!IP2366_ReadReg(&buf,REG_SYSCTL3))return false;
	//����
	buf2=(int)buf&0xFF;
	buf2*=100;
	if(Result!=NULL)*Result=buf2;
	//����ɹ����ؽ��
	return true;
	}	
	
//��ȡIP2366�Ƿ�Ϊ��������
bool IP2366_GetIfInputConnected(void)
	{
	char buf;
	if(!IP2366_ReadReg(&buf,REG_TYPEC_STATE))return false; //��ȡTypeC״̬�Ĵ���
	if(buf&0x90)return true;
	//�����������false
	return false;
	}	

//2366ʹ�ܻ��߳���оƬ�ĳ�ŵ�ģ��
bool IP2366_EnableDCDC(bool IsEnableCharger,bool IsEnableDischarge)	
	{
	char buf;
	//���ó����
	if(!IP2366_ReadReg(&buf,REG_SYSCTL0))return false;		
	if(IsEnableCharger)buf|=0x01;
	else buf&=0xFE; //����En_Charger bit
	if(!IP2366_WriteReg(buf,REG_SYSCTL0))return false;	
	//���÷ŵ�ϵͳ
	if(!IP2366_ReadReg(&buf,REG_SYSCTL11))return false;
	if(IsEnableDischarge)buf|=0x80;
	else buf&=0x7F; //����EN-DCDCOutput
	if(!IP2366_WriteReg(buf,REG_SYSCTL11))return false;
	//������ϣ�����True
	return true;
	}

//�������״̬
bool IP2366_SetOutputState(IP2366OutConfigDef * CFG)
	{
	char buf;
	extern bool IsEnableHSCPMode;
	//����Type-Cģʽ
  if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL8))return false;
	buf&=0x3F;
	buf|=CFG->IsEnableOutput?0xC0:0x40; //����Type-CģʽΪDFP��DRP
	if(!IP2366_WriteReg(buf,REG_TYPEC_CTL8))return false;
	//�������ʹ�ܼĴ���
	if(!IP2366_ReadReg(&buf,REG_SYSCTL11))return false;
	if(CFG->IsEnableOutput)buf|=0x80;
	else buf&=0x7F; //����EN-DCDCOutput
	if(CFG->IsEnableDPDMOut)buf|=0x40;
	else buf&=0xBF; //����EN-Vbus_SRC_DPDM
	if(CFG->IsEnableDPDMOut)buf|=0x20;
	else buf&=0xDF; //����EN-Vbus_SRC_PDO
	if(CFG->IsEnableDPDMOut)buf|=0x10;
	else buf&=0xEF; //����EN-Vbus_SRC_SCP	
	//����֧�����HSCP����bit�Ĺ̼��ϳ��Բ���bit3
  if(IsEnableHSCPMode)
		{		
		if(CFG->IsEnableHSCPOut)buf|=0x08;
		else buf&=0xF7; //����EN-Vbus_SRC_HSCP	
		}
	//�Ĵ���������Ͻ��л�д
	if(!IP2366_WriteReg(buf,REG_SYSCTL11))return false;
	//���ж���������ϣ�����1
	return true;
	}
//��ȡ�����PDO״̬
bool IP2366_GetRecvPDO(RecvPDODef *PDOResult)
	{
	char buf;
	//��ȡ����Ĵ���		
	if(!IP2366_ReadReg(&buf,REG_TYPEC_STATE))return false; //��ȡTypeC
	if((buf&0x90)!=0x90)//Type-C����SNKģʽ��δ����
		{
		*PDOResult=RecvPDO_None;
		return true;
		}
	//��ȡRECV PDO
	if(!IP2366_ReadReg(&buf,REG_RECEIVED_PDO))return false;	
	buf&=0x1F; //ȥ������Чλ
	if(buf&0x10)*PDOResult=RecvPDO_20V;
  else if(buf&0x08)*PDOResult=RecvPDO_15V;		
	else if(buf&0x04)*PDOResult=RecvPDO_12V;		
	else if(buf&0x02)*PDOResult=RecvPDO_9V;				
	else if(buf&0x01)*PDOResult=RecvPDO_5V;		
  else *PDOResult=RecvPDO_None;	
	//������Ϸ���true
	return true;
	}

//���ó��ڵ͵�ѹ����
bool IP2366_SetVLowVolt(VBatLowDef Vlow)
	{
	char buf;
	//��ȡ״̬
	if(!IP2366_ReadReg(&buf,REG_SYSCTL10))return false;
	buf&=0x1F;
	buf|=((char)Vlow&0x07)<<5;
	if(!IP2366_WriteReg(buf,REG_SYSCTL10))return false;
	//���óɹ�����true
	return true;
	}	

//IP2366����OTP���ؼ��ļĴ���
bool IP2366_SetOTPSign(void)	
	{
	//����ô���õ���Type-C CTL9д0x01���ڼ��
	if(!IP2366_WriteReg(0x01,REG_TYPEC_CTL9))return false;
	//����Sign�ɹ�
	return true;
	}
	
//���оƬ�Ƿ�λ
bool IP2366_DetectIfChipReset(bool *IsReset)
	{
	char buf;
	if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL9))return false;
	//�Ĵ������ݷ������ģ�оƬ�Ѿ���λ	
	*IsReset=buf==0x01?false:true;
	return true;
	}	

//����TypeC��ģʽ
bool IP2366_SetTypeCRole(TypeCRoleDef Role)
	{
	char buf;
	//��ȡ״̬
	if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL8))return false;
	buf&=0x3F;
	buf|=((char)Role&0x03)<<6;
	if(!IP2366_WriteReg(buf,REG_TYPEC_CTL8))return false;
	//���óɹ�����true
	return true;
	}	
	
//��ȡ���״̬
bool IP2366_GetChargerState(BatteryStateDef *State)	
	{
	char buf;
	//��ȡ״̬
	if(!IP2366_ReadReg(&buf,REG_STATE_CTL0))return false; //STATE-CTL0
	if(buf&0x08)*State=Batt_discharging; //��������ã������������ŵ�
	else if(buf&0x20)*State=(BatteryStateDef)(buf&0x07); //��CHGEN=1��ʱ���ȡ��س��״̬
	else *State=Batt_StandBy; //����״̬
	//��ȡ�ɹ�
	return true;
	}

//��ȡVBUS״̬
bool IP2366_GetVBUSState(IP2366VBUSStateDef * State)
	{
	char buf,buf2;
	int ibuf;
	bool STAT;
	RecvPDODef PDO;	
	//��ȡVBUS��ѹ
	if(!IP2366_ReadReg(&buf,REG_VSYS_LSB))return false;	
	ibuf=((int)buf)&0xFF;
	if(!IP2366_ReadReg(&buf,REG_VSYS_MSB))return false;		
	ibuf|=(int)(buf<<8);	
	State->VBUSVolt=(float)ibuf/(float)1000; //����ΪV
	//��ȡVBUS����
	if(!IP2366_ReadReg(&buf,REG_ISYS_LSB))return false;	
	ibuf=((int)buf)&0xFF;
	if(!IP2366_ReadReg(&buf,REG_ISYS_MSB))return false;			
	ibuf|=(int)(buf<<8);	
	State->VBUSCurrent=(float)ibuf/(float)1000; //LSB=1mA���õ�����(A)
	State->VBUSCurrent/=BusCurrentCalFactor; //����У׼ϵ��
	//��ȡ�Ƿ��ڷŵ�״̬����������������Ϊ����
	if(!IP2366_ReadReg(&buf,REG_STATE_CTL0))return false;	
	if(buf&0x08)State->VBUSCurrent*=-1;
	//���״̬���
	if(!IP2366_ReadReg(&buf,REG_TYPEC_STATE))return false; //��ȡTypeC
	if((buf&0x60)==0x60) //Type-C����SRCģʽ��PD�ɹ�����
	  {
		//��ȡSYS-CTL11������Ƿ񼤻�
		if(!IP2366_ReadReg(&buf,REG_SYSCTL11))return false;
		if(buf&0x20) //PD�Ѿ�����
		    {
				STAT=true; //Ĭ��Ϊ��
				//��ȡTYPEC-CTL17
		    if(!IP2366_ReadReg(&buf,REG_TYPEC_CTL17))return false;
				if(State->VBUSVolt>25.0) //28V
					 {
					 STAT=buf&0x10?true:false; 
				   State->PDState=PD_28VMode; 
		       }
				//���ݵ�ѹ�����ж�
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
				else State->PDState=PD_5VMode; //ʶ���ѹ�ж����ģʽ
		    if(STAT)State->QuickChargeState=State->VBUSVolt>6.0?QuickCharge_PD:QuickCharge_None; //��ǰ��ѹģʽ����Ӧ��PDO�ǿ��ŵģ������ѹ����6V����PDģʽ	 
				else //��ǰ��ѹģʽ����Ӧ��PDO�ǹرյģ������ѹ����6V���Ǹ�ѹģʽ	 
				   {
					 State->PDState=PD_5VMode; //��ѹ��䣬ָʾ��5V��PD��λ
				   State->QuickChargeState=State->VBUSVolt>6.0?QuickCharge_HV:QuickCharge_None; 
					 }
				}
		else State->QuickChargeState=State->VBUSVolt>6.0?QuickCharge_HV:QuickCharge_None; //���PD�Ѿ����رգ����ж�Ϊ��ѹ���
		}
	else if((buf&0x90)==0x90)//Type-C����SNKģʽ��PD�ɹ�����
	  {
		if(!IP2366_ReadReg(&buf,REG_STATE_CTL2))return false; //STATE-CTL2
		if(IP2366_GetRecvPDO(&PDO)) //��ȡ�յ���PDO
			{
			//����PDO�Ա�
			buf&=0x07;
			if(PDO!=RecvPDO_None&&buf>0) //�յ���ǰPDO���ɹ�Э��
				{
				State->PDState=(PDStateDef)buf;//��дenumֵ
				State->QuickChargeState=QuickCharge_PD; //�յ�PDO�����ҵ�ǰ����ѹ����Ҫ��
				}
			else if(State->VBUSVolt>6.0||buf>0)State->QuickChargeState=QuickCharge_HV;//û��PDO���ĵ���VBUS��ѹ̧�ߣ��Ǹ�ѹ���
			else State->QuickChargeState=QuickCharge_None; //����״����Ϊδʶ����
			}
		else State->QuickChargeState=QuickCharge_None; //����״����Ϊδʶ����
		}
	else  //����״��
	  {
		State->PDState=PD_5VMode;
		if(State->VBUSVolt>6.0||buf&0x04)State->QuickChargeState=QuickCharge_HV;//��ѹ���
		else if(State->VBUSVolt<=6.0&&State->VBUSVolt>4.0&&fabsf(State->VBUSCurrent)>2.4)State->QuickChargeState=QuickCharge_HC;//VBUS��4.0-6֮�䣬��ѹ��������
		else State->QuickChargeState=QuickCharge_None; //����״����Ϊδʶ����
		}
	//���Type-C��״̬	��
	State->IsTypeCConnected=false; //Ĭ��Ϊδ����״̬
	if(!IP2366_ReadReg(&buf,REG_TYPEC_STATE))return false;
	if(!IP2366_ReadReg(&buf2,REG_STATE_CTL2))return false;  //��ȡState
	if(buf&0x60) //SRC-OK����SRC-PD-OK
		{
		//�����ӣ�����Ҫ������ʲôģʽ
		if(!IP2366_ReadReg(&buf,REG_SYSCTL11))return false;	
		if(!(buf2&0x80))State->IsTypeCConnected=false; //Vbus_OK=0,Type Cδ����	
		else State->IsTypeCConnected=buf&0x80?true:false;
		}		
	else if(buf2&0x80||buf&0x90)State->IsTypeCConnected=true; //Vbus_OK=1����SNK-OK��SNK-PD-OK bit=1,Type C������
  //������ϣ�����true
  return true;	
	}
