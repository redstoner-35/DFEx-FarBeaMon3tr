#ifndef _CAPTEST_
#define _CAPTEST_

#include <stdbool.h>

typedef struct
	{
	bool IsDataValid; //���������Ƿ���Ч
	unsigned long ChargeTime; //���ʱ��(��)
	float TotalmAH; //����ĵ���(mAH)
	float MaxChargeCurrent; //����ڼ���ߵ���
	float MaxChargeTemp; //����ڼ�����¶�
	float MaxChargeRatio; //��߳�籶��
	float MaxVbatt;   //��ߵ�ص�ѹ
	float StartVbatt; //����ʱ�ĵ�ص�ѹ
	float TotalWh; //�ܹ������Wh��
	}ChargeTestDataDef;

typedef union
	{
	ChargeTestDataDef Data;
	char ByteBuf[sizeof(ChargeTestDataDef)];
	}ChargeTestUnionDef;	
	
typedef struct
	{
	ChargeTestUnionDef Data;
	unsigned int CRCResult;
	}ChargeTestROMImageDef;

typedef union
	{
  ChargeTestROMImageDef ROMImage;
	char ByteBuf[sizeof(ChargeTestROMImageDef)];
	}ChargeTestStorDef;	
	
//�ⲿ����
extern ChargeTestStorDef CTestData;
extern ChargeTestUnionDef CurrentTestResult;	
#define LastCData CTestData.ROMImage.Data.Data
#define LastCDataCRC CTestData.ROMImage.CRCResult	
	
//����
void POR_ReadCapData(void);	//��ȡ��ǰ��������
bool ReadCapData(ChargeTestStorDef *Out); //�ϵ�����ж�ȡ��������	
unsigned int CalcCapDataCRC32(ChargeTestUnionDef *IN);	//����������ݵ�CRC32
void ClearHistoryData(void); //�ָ�Ĭ�ϵĲ�������	
bool WriteCapData(ChargeTestUnionDef *IN,bool ForceUpdate); //д�����ݵ�ROM	
	
#endif
