#include "ht32.h"
#include "24Cxx.h"
#include "GUI.h"
#include "delay.h"
#include "CapTest.h"
#include "Config.h"
#include <string.h>

//�����Խṹ��
ChargeTestStorDef CTestData;
ChargeTestUnionDef CurrentTestResult;

//�ָ�Ĭ�ϵ�����
void ClearHistoryData(void)
	{
	LastCData.ChargeTime=0;
	LastCData.IsDataValid=false;
	LastCData.MaxChargeCurrent=0;
	LastCData.MaxChargeRatio=0;
	LastCData.MaxChargeTemp=-100;
	LastCData.TotalmAH=0;
	LastCData.MaxVbatt=0;
	LastCData.StartVbatt=0;
	LastCData.TotalWh=0;
	}

//����������ݵ�CRC32
unsigned int CalcCapDataCRC32(ChargeTestUnionDef *IN)
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
	for(i=0;i<sizeof(ChargeTestDataDef);i++)wb(&HT_CRC->DR,IN->ByteBuf[i]);
	result=HT_CRC->CSR;
	CRC_DeInit(HT_CRC);//���CRC���
	HT_CRC->SDR = 0x0;//CRC-32 poly: 0x04C11DB7  
	HT_CRC->CR = CRC_32_POLY | CRC_BIT_RVS_WR | CRC_BIT_RVS_SUM | CRC_BYTE_RVS_SUM | CRC_CMPL_SUM;
	for(i=0;i<16;i++)
		{
		switch(result&0x03)
			{
			case 0:StorBuf='T';break;
			case 1:StorBuf='#';break;
			case 2:StorBuf='S';break;
			case 3:StorBuf='<';break;
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
	return result^0x2542AA52;
	}

//��ȡ��������
bool ReadCapData(ChargeTestStorDef *Out)
	{
	return !M24C512_PageRead(Out->ByteBuf,CfgFileSize,sizeof(ChargeTestStorDef));
	}
	
//д���������
bool WriteCapData(ChargeTestUnionDef *IN,bool ForceUpdate)
	{
	unsigned int CRCResult;
	ChargeTestStorDef buf;
	//���㲢��д��ǰROM��CRC,��ROM����ıȶ�,��ͬ��д��
	CRCResult=CalcCapDataCRC32(IN);
	if(!ReadCapData(&buf))return false;
	if(!ForceUpdate&&CRCResult==buf.ROMImage.CRCResult)return true;
	//��ʼ������������
	memcpy(buf.ROMImage.Data.ByteBuf,IN->ByteBuf,sizeof(ChargeTestUnionDef));	
	buf.ROMImage.CRCResult=CRCResult; //��дCRC����	
	//����д��
	return !M24C512_PageWrite(buf.ByteBuf,CfgFileSize,sizeof(ChargeTestStorDef));
	}

//��ȡ��������
void POR_ReadCapData(void)
	{
	int CRCResult;
	ShowPostInfo(50,"���ز�������\0","15",Msg_Statu);
	if(!ReadCapData(&CTestData))
		{
		ShowPostInfo(50,"�洢����ȡ�쳣\0","E5",Msg_Fault);
		SelfTestErrorHandler();
		}
	//�������
	CRCResult=CalcCapDataCRC32(&CTestData.ROMImage.Data);
	if(CRCResult!=LastCDataCRC)
		{
		ShowPostInfo(50,"����������","1E",Msg_Warning);
		delay_Second(1);
		ClearHistoryData();
		if(!WriteCapData(&CTestData.ROMImage.Data,true))
			{
			ShowPostInfo(50,"�洢��д���쳣","E6",Msg_Fault);
			SelfTestErrorHandler();
			}
		ShowPostInfo(50,"�����������","0E",Msg_Warning);	
		delay_Second(1);	
		}
	//��ʼ����ǰ�Ĳ�������
	CurrentTestResult.Data.ChargeTime=0;
	CurrentTestResult.Data.IsDataValid=false;
	CurrentTestResult.Data.MaxChargeCurrent=0;
	CurrentTestResult.Data.MaxChargeRatio=0;
	CurrentTestResult.Data.MaxChargeTemp=-100;
	CurrentTestResult.Data.TotalmAH=0;
	CurrentTestResult.Data.MaxVbatt=0;
	CurrentTestResult.Data.StartVbatt=0;
	CurrentTestResult.Data.TotalWh=0;
	}	
