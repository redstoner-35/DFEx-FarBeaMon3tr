#ifndef _CAPTEST_
#define _CAPTEST_

#include <stdbool.h>

typedef struct
	{
	bool IsDataValid; //数据区域是否有效
	unsigned long ChargeTime; //充电时长(秒)
	float TotalmAH; //冲入的电量(mAH)
	float MaxChargeCurrent; //充电期间最高电流
	float MaxChargeTemp; //充电期间最高温度
	float MaxChargeRatio; //最高充电倍率
	float MaxVbatt;   //最高电池电压
	float StartVbatt; //启动时的电池电压
	float TotalWh; //总共冲入的Wh数
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
	
//外部引用
extern ChargeTestStorDef CTestData;
extern ChargeTestUnionDef CurrentTestResult;	
#define LastCData CTestData.ROMImage.Data.Data
#define LastCDataCRC CTestData.ROMImage.CRCResult	
	
//函数
void POR_ReadCapData(void);	//读取当前测容数据
bool ReadCapData(ChargeTestStorDef *Out); //上电过程中读取测容数据	
unsigned int CalcCapDataCRC32(ChargeTestUnionDef *IN);	//计算测容数据的CRC32
void ClearHistoryData(void); //恢复默认的测容数据	
bool WriteCapData(ChargeTestUnionDef *IN,bool ForceUpdate); //写入数据到ROM	
	
#endif
