#ifndef _LSYS_
#define _LSYS_

//内部包含
#include "CapTest.h"
#include "Config.h"

typedef struct
	{
	long BalanceTime; //均衡器运行时间
	long ChargeTime; //总计充电时间
	float TotalChargeAh; //总计充入的Ah数
	float TotalChargeWh; //总计充入的Wh数
	long DischargeTime; //总计放电时间
	float UnbalanceBatteryAh; //未经平衡的电池Ah数
	float TotalDischargeAh; //总计放出的Ah数
	float TotalDischargeWh; //总计放出的Wh数
	float SysMaxTemp; //系统最高温度
	float MaximumBattCurrent; //电池端最大电流
	//是否触发保护系统
	bool IsEnablePunish; //是否触发保护机制
	}LogContentDef;
	
typedef union
	{
	LogContentDef Content;
	unsigned char ContentBuf[sizeof(LogContentDef)]; //日志内容union
	}LogContentUnionDef;

typedef struct
 {
 //运行日志头部
 signed char LogIncrementCode;//日志的递增码
 unsigned int TotalLogCount;//总日志条数
 bool IsRunlogHasContent; //运行日志是否有内容
 char LogKey[4];  //用于检查log输入的key
 unsigned int LogContentSum; //日志数据的校验和
 //运行日志内容
 LogContentUnionDef Data;
 }RunLogDataStrDef;

 typedef union
 {
 RunLogDataStrDef DataSec;
 char DataCbuf[sizeof(RunLogDataStrDef)];
 }RunLogDataUnionDef;	//使得数据域部分可以按字节操作的Union
 
typedef struct
 {
 RunLogDataUnionDef Data;
 unsigned int CurrentDataCRC;
 unsigned int LastDataCRC; //CRC结果
 char ProgrammedEntry; //目标编程的entry
 }RunLogEntryStrDef;	//运行日志结构体的定义
 
//定义
#define RunTimeLoggerDepth 110  //运行日志的深度
#define RunTimeLogBase CfgFileSize+sizeof(ChargeTestStorDef) //运行日志的起始位置
#define RunTimeLogSize RunTimeLoggerDepth*sizeof(RunLogDataStrDef)  //运行日志的大小
#define RunTimeLogKey "RLoG" //运行log的内容检查Key

//外部参考
#define LogHeader RunLogEntry.Data.DataSec
#define LogData RunLogEntry.Data.DataSec.Data.Content
extern RunLogEntryStrDef RunLogEntry;

//日志文件处理
bool ResetRunTimeLogArea(void); //复位日志区域
void WriteRuntimeLogToROM(void); //写日志
void ForceWriteRuntimelog(void); //强制写日志
unsigned int CalcRunLogCRC32(RunLogDataUnionDef *DIN); //计算CRC32 
void LogDataSectionInit(RunLogDataUnionDef *DIN); //初始化日志文件
void CalcLastLogCRCBeforePO(void); //计算旧文件的CRC
unsigned int CalcLogContentCRC32(RunLogDataUnionDef *DIN); //计算日志数据区的SUM

//对外日志处理
void UpdataRunTimeLog(void); //进行数据采集并更新日志
void RunLogModule_POR(void); //读取日志

#endif
