#include "ht32.h"
#include "24Cxx.h"
#include "GUI.h"
#include "delay.h"
#include "CapTest.h"
#include "Config.h"
#include <string.h>

//充电测试结构体
ChargeTestStorDef CTestData;
ChargeTestUnionDef CurrentTestResult;

//恢复默认的数据
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

//计算测容数据的CRC32
unsigned int CalcCapDataCRC32(ChargeTestUnionDef *IN)
	{
	int i;
	unsigned int result;
	unsigned char StorBuf;
  CKCU_PeripClockConfig_TypeDef CLKConfig={{0}};
	//初始化CRC32        
	CLKConfig.Bit.CRC = 1;
	CKCU_PeripClockConfig(CLKConfig,ENABLE);//启用CRC-32时钟 
	CRC_DeInit(HT_CRC);//清除配置
	HT_CRC->SDR = 0x0;//CRC-32 poly: 0x04C11DB7  
	HT_CRC->CR = CRC_32_POLY | CRC_BIT_RVS_WR | CRC_BIT_RVS_SUM | CRC_BYTE_RVS_SUM | CRC_CMPL_SUM;
	//写数据
	for(i=0;i<sizeof(ChargeTestDataDef);i++)wb(&HT_CRC->DR,IN->ByteBuf[i]);
	result=HT_CRC->CSR;
	CRC_DeInit(HT_CRC);//清除CRC结果
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
		result>>=2; //右移两位
		StorBuf^=0x01<<(i%8); //和i的8次结果进行XOR
		wb(&HT_CRC->DR,StorBuf+i);
		}
  //读取数据结果
	result=HT_CRC->CSR;
	CRC_DeInit(HT_CRC);
	CKCU_PeripClockConfig(CLKConfig,DISABLE);//禁用CRC-32时钟节省电力
	//返回结果
	return result^0x2542AA52;
	}

//读取测容数据
bool ReadCapData(ChargeTestStorDef *Out)
	{
	return !M24C512_PageRead(Out->ByteBuf,CfgFileSize,sizeof(ChargeTestStorDef));
	}
	
//写入测容数据
bool WriteCapData(ChargeTestUnionDef *IN,bool ForceUpdate)
	{
	unsigned int CRCResult;
	ChargeTestStorDef buf;
	//计算并填写当前ROM的CRC,和ROM里面的比对,相同则不写入
	CRCResult=CalcCapDataCRC32(IN);
	if(!ReadCapData(&buf))return false;
	if(!ForceUpdate&&CRCResult==buf.ROMImage.CRCResult)return true;
	//开始复制数据数据
	memcpy(buf.ROMImage.Data.ByteBuf,IN->ByteBuf,sizeof(ChargeTestUnionDef));	
	buf.ROMImage.CRCResult=CRCResult; //填写CRC数据	
	//正常写入
	return !M24C512_PageWrite(buf.ByteBuf,CfgFileSize,sizeof(ChargeTestStorDef));
	}

//读取测容数据
void POR_ReadCapData(void)
	{
	int CRCResult;
	ShowPostInfo(50,"加载测容数据\0","15",Msg_Statu);
	if(!ReadCapData(&CTestData))
		{
		ShowPostInfo(50,"存储器读取异常\0","E5",Msg_Fault);
		SelfTestErrorHandler();
		}
	//检查配置
	CRCResult=CalcCapDataCRC32(&CTestData.ROMImage.Data);
	if(CRCResult!=LastCDataCRC)
		{
		ShowPostInfo(50,"测容数据损坏","1E",Msg_Warning);
		delay_Second(1);
		ClearHistoryData();
		if(!WriteCapData(&CTestData.ROMImage.Data,true))
			{
			ShowPostInfo(50,"存储器写入异常","E6",Msg_Fault);
			SelfTestErrorHandler();
			}
		ShowPostInfo(50,"已清除损坏数据","0E",Msg_Warning);	
		delay_Second(1);	
		}
	//初始化当前的测容数据
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
