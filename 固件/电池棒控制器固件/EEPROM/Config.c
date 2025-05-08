#include "Config.h"
#include "ht32.h"
#include "24Cxx.h"
#include "GUI.h"
#include "delay.h"

//���ýṹ��
CfgUnionDef CfgUnion;

//��ħ�����ü��ص�ָ��λ��
void LoadDefaultConfig(CfgUnionDef *IN)
	{
	extern bool IsEnable17AMode;	
	IN->ROMImage.Data.Data.Vlow=VLow_2V8;
	//��������
	IN->ROMImage.Data.Data.VRecharge=Recharge_0V1;
	IN->ROMImage.Data.Data.IStop=IStop_200mA;
	IN->ROMImage.Data.Data.InputConfig.ChargeCurrent=IsEnable17AMode?IP2366_ICCMAX:9700;
	IN->ROMImage.Data.Data.InputConfig.ChargePower=Power_100W;
	IN->ROMImage.Data.Data.InputConfig.FullVoltage=4200;
	IN->ROMImage.Data.Data.InputConfig.PreChargeCurrent=400;
	IN->ROMImage.Data.Data.InputConfig.IsEnableCharger=true;
	//�������
	IN->ROMImage.Data.Data.OutputConfig.IsEnableDPDMOut=true;
	IN->ROMImage.Data.Data.OutputConfig.IsEnableOutput=true;
	IN->ROMImage.Data.Data.OutputConfig.IsEnablePDOut=true;
	IN->ROMImage.Data.Data.OutputConfig.IsEnableSCPOut=true;
	IN->ROMImage.Data.Data.OutputConfig.IsEnableHSCPOut=false; //Ĭ�Ϲر�
	//PDO����
	IN->ROMImage.Data.Data.PDOCFG.EnablePPS1=true;
	IN->ROMImage.Data.Data.PDOCFG.EnablePPS2=true;
	IN->ROMImage.Data.Data.PDOCFG.Enable20V=true;
	IN->ROMImage.Data.Data.PDOCFG.Enable15V=true;
	IN->ROMImage.Data.Data.PDOCFG.Enable12V=true;
	IN->ROMImage.Data.Data.PDOCFG.Enable9V=true;
	//��ȫ����
	IN->ROMImage.Data.Data.EnableThermalStepdown=true;
	IN->ROMImage.Data.Data.EnableAdvAccess=false;
  IN->ROMImage.Data.Data.EnableChargeConfig=false;
  IN->ROMImage.Data.Data.EnableChargPowerConfig=true;
	IN->ROMImage.Data.Data.EnableDischargeConfig=true;
	IN->ROMImage.Data.Data.EnableLVProtectConfig=true;
	IN->ROMImage.Data.Data.EnablePDOConfig=true;
  IN->ROMImage.Data.Data.EnableOTPConfig=false;
	IN->ROMImage.Data.Data.OverHeatLockTemp=90;
	//��ʾ��������
  IN->ROMImage.Data.Data.EnableFastBoot=true;
	IN->ROMImage.Data.Data.EnableLargeMenu=true;
	IN->ROMImage.Data.Data.DisplayDir=LCDDisplay_Hori_Invert;
  //������������
	IN->ROMImage.Data.Data.InstantCTest=InstantCTest_NotTriggered;
	//���PD��������
  IN->ROMImage.Data.Data.MaxVPD=IsEnable17AMode?PDMaxIN_28V:PDMaxIN_20V;
  //����ϵͳ����
	IN->ROMImage.Data.Data.BalanceMode=Balance_ChgDisOnly; //������ڳ�ŵ�ʱ����
	}

//�ָ�Ĭ������
void RestoreDefaultConfig(void)
	{
  LoadDefaultConfig(&CfgUnion);
	}

//����CRC32
unsigned int CalcROMCRC32(CfgUnionDef *IN)
	{
	int i;
	unsigned int result;
	unsigned char StorBuf;
  CKCU_PeripClockConfig_TypeDef CLKConfig={{0}};
	//��ʼ��CRC32        
	CLKConfig.Bit.CRC = 1;
	CKCU_PeripClockConfig(CLKConfig,ENABLE);//����CRC-32ʱ�� 
	CRC_DeInit(HT_CRC);//�������
	HT_CRC->SDR = 0x0;//CRC-32 poly: 0x04C11DB7  
	HT_CRC->CR = CRC_32_POLY | CRC_BIT_RVS_WR | CRC_BIT_RVS_SUM | CRC_BYTE_RVS_SUM | CRC_CMPL_SUM;
	//д����
	for(i=0;i<sizeof(SystemCfgDef);i++)wb(&HT_CRC->DR,IN->ROMImage.Data.ByteBuf[i]);
	result=HT_CRC->CSR;
	CRC_DeInit(HT_CRC);//���CRC���
	HT_CRC->SDR = 0x0;//CRC-32 poly: 0x04C11DB7  
	HT_CRC->CR = CRC_32_POLY | CRC_BIT_RVS_WR | CRC_BIT_RVS_SUM | CRC_BYTE_RVS_SUM | CRC_CMPL_SUM;
	for(i=0;i<16;i++)
		{
		switch(result&0x03)
			{
			case 0:StorBuf='R';break;
			case 1:StorBuf='?';break;
			case 2:StorBuf='@';break;
			case 3:StorBuf='n';break;
			}
		result>>=2; //������λ
		StorBuf^=0x01<<(i%8); //��i��8�ν������XOR
		wb(&HT_CRC->DR,StorBuf+i);
		}
  //��ȡ���ݽ��
	result=HT_CRC->CSR;
	CRC_DeInit(HT_CRC);
	CKCU_PeripClockConfig(CLKConfig,DISABLE);//����CRC-32ʱ�ӽ�ʡ����
	//���ؽ��
	return result^0x5AA53453;
	}

//��ȡ����
bool ReadConfiguration(CfgUnionDef *Out)
	{
	return !M24C512_PageRead(Out->ByteBuf,0x0000,sizeof(CfgUnionDef));
	}
	
//д������
bool WriteConfiguration(CfgUnionDef *IN,bool ForceUpdate)
	{
	unsigned int CRCResult,bufReselt;
	CfgUnionDef buf;
	//���㲢��д��ǰROM��CRC,��ROM����ıȶ�,��ͬ��д��
	CRCResult=CalcROMCRC32(IN);
	if(!ReadConfiguration(&buf))return false;
	bufReselt=CalcROMCRC32(&buf);
	//ǿ�Ƹ���ģʽ�رգ�ROM������û�����ҵ�ǰ����һ��������д�벢����true
	if(!ForceUpdate&&CRCResult==buf.ROMImage.CRCResult&&bufReselt==buf.ROMImage.CRCResult)return true;
	//����д��
	IN->ROMImage.CRCResult=CRCResult;
	return !M24C512_PageWrite(IN->ByteBuf,0x0000,sizeof(CfgUnionDef));
	}
	
//���EEPROM�͵�ǰ�������Ƿ���ͬ
bool CheckIfConfigIsSame(void)
	{
	unsigned int CRCResult;
	CfgUnionDef buf;
	//���㲢��д��ǰROM��CRC,��ROM����ıȶ�
	CRCResult=CalcROMCRC32(&CfgUnion);
	if(!ReadConfiguration(&buf))return false;
	//����һ��
	if(CRCResult==buf.ROMImage.CRCResult)return true;
	return false;
	}
	
//����ʱ��ȡ����
void LoadConfig(void)
	{
	int CRCResult;
	extern bool EnableDetailOutput;	
	bool IsNeedToUpgrade=false;
	//��ȡ����	
	ShowPostInfo(30,"����ϵͳ����\0","0D",Msg_Statu);
	if(!ReadConfiguration(&CfgUnion))
		{
		ShowPostInfo(30,"�洢����ȡ�쳣\0","E5",Msg_Fault);
		SelfTestErrorHandler();
		}
	//�������
	CRCResult=CalcROMCRC32(&CfgUnion);
	if(CRCResult!=CfgChecksum)
		{
		ShowPostInfo(30,"����������","0E",Msg_Warning);
		delay_Second(1);
		RestoreDefaultConfig();
		if(!WriteConfiguration(&CfgUnion,true))
			{
			ShowPostInfo(30,"�洢��д���쳣","E6",Msg_Fault);
			SelfTestErrorHandler();
			}		
		ShowPostInfo(30,"�Ѽ��س�������","0E",Msg_Warning);	
		delay_Second(1);	
		}
	//��PD���ݽ�������	
	if(CfgData.InputConfig.ChargeCurrent>IP2366_ICCMAX)
		{
		ShowPostInfo(30,"��ֵ�������÷Ƿ�\0","1F",Msg_Warning);
		delay_Second(1);
    CfgData.InputConfig.ChargeCurrent=IP2366_ICCMAX;
		IsNeedToUpgrade=true;	
		}		
	if(CfgData.MaxVPD==PDMaxIN_20V&&(CfgData.InputConfig.ChargePower==Power_140W||CfgData.InputConfig.ChargePower==Power_100W))	
		{
		ShowPostInfo(30,"ϵͳ���÷Ƿ�\0","0F",Msg_Warning);
		delay_Second(1);
		CfgData.InputConfig.ChargePower=Power_65W;	
		IsNeedToUpgrade=true;	
		}
	//��Ҫ��������
	if(IsNeedToUpgrade)
		{
		if(!WriteConfiguration(&CfgUnion,true))
			{
			ShowPostInfo(30,"�洢��д���쳣\0","E6",Msg_Fault);
			SelfTestErrorHandler();
			}		
		else
			{			
			ShowPostInfo(30,"�ѽ����Զ�����\0","0F",Msg_Warning);
			delay_Second(1);
			}
		}
	//��ȡ���ú�����رտ�����������ʾʣ�µĲ���
	if(!CfgData.EnableFastBoot)EnableDetailOutput=true;
	else EnableDetailOutput=false;
	}
