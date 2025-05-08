#ifndef _LSYS_
#define _LSYS_

//�ڲ�����
#include "CapTest.h"
#include "Config.h"

typedef struct
	{
	long BalanceTime; //����������ʱ��
	long ChargeTime; //�ܼƳ��ʱ��
	float TotalChargeAh; //�ܼƳ����Ah��
	float TotalChargeWh; //�ܼƳ����Wh��
	long DischargeTime; //�ܼƷŵ�ʱ��
	float UnbalanceBatteryAh; //δ��ƽ��ĵ��Ah��
	float TotalDischargeAh; //�ܼƷų���Ah��
	float TotalDischargeWh; //�ܼƷų���Wh��
	float SysMaxTemp; //ϵͳ����¶�
	float MaximumBattCurrent; //��ض�������
	//�Ƿ񴥷�����ϵͳ
	bool IsEnablePunish; //�Ƿ񴥷���������
	}LogContentDef;
	
typedef union
	{
	LogContentDef Content;
	unsigned char ContentBuf[sizeof(LogContentDef)]; //��־����union
	}LogContentUnionDef;

typedef struct
 {
 //������־ͷ��
 signed char LogIncrementCode;//��־�ĵ�����
 unsigned int TotalLogCount;//����־����
 bool IsRunlogHasContent; //������־�Ƿ�������
 char LogKey[4];  //���ڼ��log�����key
 unsigned int LogContentSum; //��־���ݵ�У���
 //������־����
 LogContentUnionDef Data;
 }RunLogDataStrDef;

 typedef union
 {
 RunLogDataStrDef DataSec;
 char DataCbuf[sizeof(RunLogDataStrDef)];
 }RunLogDataUnionDef;	//ʹ�������򲿷ֿ��԰��ֽڲ�����Union
 
typedef struct
 {
 RunLogDataUnionDef Data;
 unsigned int CurrentDataCRC;
 unsigned int LastDataCRC; //CRC���
 char ProgrammedEntry; //Ŀ���̵�entry
 }RunLogEntryStrDef;	//������־�ṹ��Ķ���
 
//����
#define RunTimeLoggerDepth 110  //������־�����
#define RunTimeLogBase CfgFileSize+sizeof(ChargeTestStorDef) //������־����ʼλ��
#define RunTimeLogSize RunTimeLoggerDepth*sizeof(RunLogDataStrDef)  //������־�Ĵ�С
#define RunTimeLogKey "RLoG" //����log�����ݼ��Key

//�ⲿ�ο�
#define LogHeader RunLogEntry.Data.DataSec
#define LogData RunLogEntry.Data.DataSec.Data.Content
extern RunLogEntryStrDef RunLogEntry;

//��־�ļ�����
bool ResetRunTimeLogArea(void); //��λ��־����
void WriteRuntimeLogToROM(void); //д��־
void ForceWriteRuntimelog(void); //ǿ��д��־
unsigned int CalcRunLogCRC32(RunLogDataUnionDef *DIN); //����CRC32 
void LogDataSectionInit(RunLogDataUnionDef *DIN); //��ʼ����־�ļ�
void CalcLastLogCRCBeforePO(void); //������ļ���CRC
unsigned int CalcLogContentCRC32(RunLogDataUnionDef *DIN); //������־��������SUM

//������־����
void UpdataRunTimeLog(void); //�������ݲɼ���������־
void RunLogModule_POR(void); //��ȡ��־

#endif
