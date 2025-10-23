#include "LogSystem.h"
#include "24Cxx.h"
#include "ht32.h"
#include "IP2366_REG.h"
#include "ADC.h"
#include "BalanceMgmt.h"
#include <string.h>
#include <math.h>
#include "GUI.h"
#include "WatchDog.h"
#include "delay.h"

//函数声明
float fmaxf(float x,float y);
float fminf(float x,float y);
typedef struct
	{
	bool VBUSOVP;
	bool VBUSOCP;
	bool VBUSSCP;
	}VBUSEventCounterStrDef;

//变量
RunLogEntryStrDef RunLogEntry;
static char SampleCount=0;
static float BATSample[2]={0};
static bool IsLogSaved=false;
static VBUSEventCounterStrDef VBUSEventStor={false,false,false};

//内部conost
static const char LogAreaFrontKey[8]={0xAA,0x55,0x35,0x1A,0x4C,(RunTimeLoggerDepth>>8)&0xFF,0x88,RunTimeLoggerDepth&0xFF};
static const char LogAreaRearKey[8]={0x35,0x73,0xBC,0x90,RunTimeLoggerDepth&0xFF,0xDE,(RunTimeLoggerDepth>>8)&0xFF,0xA5};

//外部函数声明
void AttackDetectInit(void);

//更新日志
void UpdataRunTimeLog(void)
	{
	char i;
	float Cap;
	extern bool IsUpdateCDUI,Is2366Telem;
	extern float VTypec,ITypeC;
	BatteryStateDef State=Batt_StandBy;
	extern ChipStatDef CState;
	bool result;
	//获取过流和短路保护事件	
	result=CState.VSysState==VSys_State_OCP?true:false;
	if(VBUSEventStor.VBUSOCP!=result)
		{
		VBUSEventStor.VBUSOCP=result;
		if(result&&LogData.SystemOCPCount<20000)LogData.SystemOCPCount++;
		}
	result=CState.VSysState==VSys_State_Short?true:false;	
	if(VBUSEventStor.VBUSSCP!=result)
		{
		VBUSEventStor.VBUSSCP=result;
		if(result&&LogData.SystemSCPCount<20000)LogData.SystemSCPCount++;
		}
	//获取VBUS过压保护事件
	result=CState.VBusState==VBUS_OverVolt?true:false;
	if(VBUSEventStor.VBUSOVP!=result)
		{
		VBUSEventStor.VBUSOVP=result;
		if(result&&LogData.VBUSOVPCount<20000)LogData.VBUSOVPCount++;
		}
	//获取Type-C状态
	IP2366_GetChargerState(&State);
	if(!IP2366_QueryCurrentStateIsACC(State)||fabsf(ADCO.Ibatt)<=MinimumCurrentFactor) //芯片处于非充放电状态，复位采样缓存
		{
		SampleCount=8;
		for(i=0;i<2;i++)BATSample[i]=0;
		//当系统回归到待机状态之后，且日志未保存，则计算CRC32并保存日志
		if(State==Batt_StandBy&&!IsLogSaved)
			{
			IsLogSaved=true;
			RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data); //计算运行日志的CRC32
			WriteRuntimeLogToROM(); //保存日志
			}
		}
	//进行平均值计算
	else if(SampleCount>0)
		{
		Is2366Telem=true;          //触发C端采样
		BATSample[0]+=ADCO.Vbatt;
		BATSample[1]+=ADCO.Ibatt;
		SampleCount--;
		}
	//时间到，进行采集
	else
		{
		//日志内容已更新
		IsLogSaved=false;
		LogHeader.IsRunlogHasContent=true;
		//进行平均计算
		for(i=0;i<2;i++)BATSample[i]/=(float)8;
		//获取Type-C最高功率和电流
		Cap=fabs(VTypec*ITypeC);
		if(LogData.MaximumTypeCPower<Cap)LogData.MaximumTypeCPower=Cap;
		Cap=fabs(ITypeC);
		if(LogData.MaximumTypeCCurrent<Cap)LogData.MaximumTypeCCurrent=Cap;
		//累计充放电时间
		if(State==Batt_discharging)LogData.DischargeTime++;
		else LogData.ChargeTime++;		 
    if(BalanceState)LogData.BalanceTime++; //均衡开启时累加均衡时间		
		//计算AH
		Cap=fabsf(BATSample[1])/(float)3600; //当前电流*1000 /3600秒得到AH
   	if(State!=Batt_discharging)LogData.TotalChargeAh+=Cap;
    else LogData.TotalDischargeAh+=Cap;
		if(!BalanceState)LogData.UnbalanceBatteryAh+=Cap; //如果均衡器处于关闭状态，则将容量统计至未均衡区域
    //计算Wh
		Cap=fabsf(BATSample[0]*BATSample[1])/(float)3600;//当前电压*电流/3600秒得到Wh
		if(State==Batt_discharging)LogData.TotalDischargeWh+=Cap;
	  else LogData.TotalChargeWh+=Cap;
    //获取最高电池电流和温度
		if(!ADCO.IsNTCOK)LogData.SysMaxTemp=-100;
		else if(LogData.SysMaxTemp<ADCO.Systemp)LogData.SysMaxTemp=ADCO.Systemp;
		if(LogData.MaximumBattCurrent<fabsf(BATSample[1]))LogData.MaximumBattCurrent=fabsf(BATSample[1]);
	  //复位缓冲
		SampleCount=8;
	  for(i=0;i<2;i++)BATSample[i]=0; 
		//在数据查看菜单内，更新数据
		IsUpdateCDUI=true;
		}
	}

/*******************************************
重新写入缓存数据域前部和后部的guard Key

输入：无
输出:如果成功读取,则返回true,否则返回false
********************************************/		
bool WriteLogCacheKeyArea(void)
{
	if(M24C512_PageWrite((char *)LogAreaRearKey,RunTimeLogGuardKeyRearBase,sizeof(LogAreaRearKey)))return false;
	return !M24C512_PageWrite((char *)LogAreaFrontKey,RunTimeLogGuardKeyFrontBase,sizeof(LogAreaFrontKey));
}
	
/*******************************************
比较缓存数据域前部和后部的guard Key是否正常

输入：比较结果指针，如果key匹配，返回true否则
返回false
输出:如果成功读取,则返回true,否则返回false
********************************************/		
bool CompareLogCacheKeyIsOK(bool *Result)
{
	char KeyBuf[sizeof(LogAreaFrontKey)+sizeof(LogAreaRearKey)+1];
	//检查参数
	if(Result==NULL)return false;
	//读取数据到Cache Area
  memset(KeyBuf,0,sizeof(KeyBuf));
	if(M24C512_PageRead(KeyBuf,RunTimeLogGuardKeyFrontBase,sizeof(LogAreaFrontKey)))return false;
	if(M24C512_PageRead(&KeyBuf[sizeof(LogAreaFrontKey)],RunTimeLogGuardKeyRearBase,sizeof(LogAreaRearKey)))return false;
  //开始比较
  *Result=true;
	if(strncmp(KeyBuf,LogAreaFrontKey,sizeof(LogAreaFrontKey)))*Result=false;
	if(strncmp(&KeyBuf[sizeof(LogAreaFrontKey)],LogAreaRearKey,sizeof(LogAreaRearKey)))*Result=false;
	return true;
}

/*******************************************
将指定的运行日志的缓存数据域（用于从ROM内快速寻
找出最新的entry）从ROM内指定的位置中读出并写入
到RAM内。

输入：输出缓存数据的union
输出:如果成功读取,则返回true,否则返回false
********************************************/	
static bool ReadEntireLogCache(IncCodeROMUnionDef *CodeCache)	
{
//空指针不允许执行
if(CodeCache==NULL)return false;
//读取数据
return !M24C512_PageRead(CodeCache->ByteBuf,RunTimeLogGuardKeyFrontBase,sizeof(IncCodeROMUnionDef));
}
/*******************************************
从缓存中读取指定index的自增码

输入：自增码的具体数值输出，以及目标的位置
输出:如果成功读取,则返回true,否则返回false
********************************************/	
static bool ReadOneCodeFromIndex(int LogEntryNum,signed short *Result)
{
IncCodeSingleWriteDef Buf;
//传进来的参数是错的
if(Result==NULL)return false;
if(LogEntryNum<0||LogEntryNum>RunTimeLoggerDepth-1)return false; //Entry不合法
//读取数据
if(M24C512_PageRead(Buf.ByteBuf,RunTimeLogCacheBase+(LogEntryNum*sizeof(IncCodeSingleWriteDef)),sizeof(IncCodeSingleWriteDef)))return false;
//成功读取，返回结果
*Result=Buf.IncCode;
return true;
}


/*******************************************
将指定的运行日志所对应的自增码，更新到的缓存数
据域（用于从ROM内快速寻找出最新的entry）的指定
位置

输入：自增码的具体数值，以及目标的位置
输出:如果成功写入,则返回true,否则返回false
********************************************/	
static bool UpdateLogCache(signed short IncCode,int LogEntryNum)
{
IncCodeSingleWriteDef Buf;
//传进来的参数是错的
if(IncCode<-(RunTimeLoggerDepth+1)||IncCode>RunTimeLoggerDepth+1)return false; //自增码不合法
if(LogEntryNum<0||LogEntryNum>RunTimeLoggerDepth-1)return false; //Entry不合法
//开始处理数据
Buf.IncCode=IncCode;
return !M24C512_PageWrite(Buf.ByteBuf,RunTimeLogCacheBase+(LogEntryNum*sizeof(IncCodeSingleWriteDef)),sizeof(IncCodeSingleWriteDef));
}
/*******************************************
将指定的运行日志的数据域从ROM内指定的entry中
读出并写入到RAM内。

输入：输出遥测数据的union，目标读取的entry
输出:如果成功读取,则返回true,否则返回false
********************************************/
static bool LoadRunLogDataFromROM(RunLogDataUnionDef *DataOut,int LogEntryNum)
 {
 //传进来的参数是错的
 if(DataOut==NULL||LogEntryNum<0||LogEntryNum>RunTimeLoggerDepth-1)return false;
 //开始读取
 if(M24C512_PageRead(DataOut->DataCbuf,RunTimeLogBase+(LogEntryNum*sizeof(RunLogDataUnionDef)),sizeof(RunLogDataUnionDef)))
	 return false;
 //读取完毕，返回true
 return true;
 }

/*******************************************
将指定的运行日志的数据域写入到ROM内指定的
entry中。
输入：输出遥测数据的union，目标写入的entry
输出:如果成功写入,则返回true,否则返回false
********************************************/
static bool SaveRunLogDataToROM(RunLogDataUnionDef *DataIn,int LogEntryNum)
 {
 //传进来的参数是错的
 if(DataIn==NULL||LogEntryNum<0||LogEntryNum>RunTimeLoggerDepth-1)return false;
 //计算CRC32
 DataIn->DataSec.LogContentSum=CalcLogContentCRC32(DataIn); //计算CRC32
 //开始写入
 if(M24C512_PageWrite(DataIn->DataCbuf,RunTimeLogBase+(LogEntryNum*sizeof(RunLogDataUnionDef)),sizeof(RunLogDataUnionDef)))
	 return false;
 //写入完毕，返回true
 return true;
 }

/*******************************************
计算传入数据的CRC32校验和用以确认是否要写log
区域等等。
输入：遥测数据的union
输出：该组数据的CRC32校验和
********************************************/
unsigned int CalcRunLogCRC32(RunLogDataUnionDef *DIN)
{
 unsigned int DATACRCResult; 
 int i;
 CKCU_PeripClockConfig_TypeDef CLKConfig={{0}};
 //初始化CRC32      
 CLKConfig.Bit.CRC = 1;
 CKCU_PeripClockConfig(CLKConfig,ENABLE);//启用CRC-32时钟  
 CRC_DeInit(HT_CRC);//清除配置
 HT_CRC->SDR = 0x0;//CRC-32 poly: 0x04C11DB7  
 HT_CRC->CR = CRC_32_POLY | CRC_BIT_RVS_WR | CRC_BIT_RVS_SUM | CRC_BYTE_RVS_SUM | CRC_CMPL_SUM;
 //开始校验
 for(i=0;i<sizeof(RunLogDataUnionDef);i++)wb(&HT_CRC->DR,DIN->DataCbuf[i]);//将内容写入到CRC寄存器内
 //校验完毕计算结果
 DATACRCResult=HT_CRC->CSR^0xA352EE4F;
 CRC_DeInit(HT_CRC);//清除CRC结果
 CKCU_PeripClockConfig(CLKConfig,DISABLE);//禁用CRC-32时钟节省电力
 return DATACRCResult;
}	
/*******************************************
在手电筒转换为运行状态的自检前，计算目前数据
的CRC32并更新结构体内上组数据记录的CRC校验和
方便对比LOG是否被更新
********************************************/ 
void CalcLastLogCRCBeforePO(void)
  {
	//计算CRC并填写结构体
	RunLogEntry.LastDataCRC=CalcRunLogCRC32(&RunLogEntry.Data);
	}

/*******************************************
强制执行将运行日志写入到ROM的动作。这个函数
主要给锁定模式更新数据用
*******************************************/
#pragma push
#pragma O0
void ForceWriteRuntimelog(void)
  {
	signed short SelfIncCode,OldCode;
	bool result;
	//计算新的自增码
	SelfIncCode=RunLogEntry.Data.DataSec.LogIncrementCode;
	OldCode=RunLogEntry.Data.DataSec.LogIncrementCode;
  if(SelfIncCode<0)SelfIncCode--;
  else if(SelfIncCode>0)SelfIncCode++;
	else SelfIncCode=1; //如果自增码位于负数范围，则自增码-1否则加1，对于是0的情况则为1
	if(SelfIncCode<(-RunTimeLoggerDepth))SelfIncCode=1;
	if(SelfIncCode>RunTimeLoggerDepth)SelfIncCode=-1;//如果自增码到达上限则翻转到另一个极性
  RunLogEntry.Data.DataSec.LogIncrementCode=SelfIncCode;//将计算好的自增码写进去
	RunLogEntry.Data.DataSec.TotalLogCount++; //日志写入计数器+1
	//尝试编程
	result=SaveRunLogDataToROM(&RunLogEntry.Data,RunLogEntry.ProgrammedEntry); 
	result&=UpdateLogCache(SelfIncCode,RunLogEntry.ProgrammedEntry);  //写入缓存区域和数据域
  if(result)
	  {
		CalcLastLogCRCBeforePO();  //编程结束后将新的log的CRC-32值替换过去避免重复写入
    RunLogEntry.ProgrammedEntry=(RunLogEntry.ProgrammedEntry+1)%RunTimeLoggerDepth;//编程成功，指向下一个entry，如果达到额定的entry数目则翻转回来  		
		}
	else 
		RunLogEntry.Data.DataSec.LogIncrementCode=OldCode;//编程失败，entry数不增加的同时，还原更改了的自增码
	}

/*******************************************
在手电筒关闭后，我们需要将运行log写入到ROM内
在这期间，我们首先需要验证运行log是否发生变化
如果发生变化，则开始写入。
*******************************************/
void WriteRuntimeLogToROM(void)
  {
  //如果CRC-32相同说明运行的log没有发生改变,不需要操作
	if(RunLogEntry.LastDataCRC==RunLogEntry.CurrentDataCRC)return;
  //开始编程
	ForceWriteRuntimelog();
	}
	
/*******************************************
从读取到的increment-code数组中找出最新记录的
entry。
输入：包含自增code的数组
输出：最新的一组entry所在的位置
********************************************/
static int FindLatestEntryViaIncCode(signed short *CodeIN)
  {
	int i;
  //判断数组的第0个元素是正还是负还是0
	if(CodeIN[0]>0)
	  {
		for(i=0;i<RunTimeLoggerDepth-1;i++)//大于0
			{
			/*
								i i+1
			[1 2 3 4 5 6 0 0 0 0 0 ]这种情况.
			6是最新的，后面啥也没有了返回结果	
			*/
			if(CodeIN[i+1]==0)return i;
			/*
								i i+1
			[1 2 3 4 5 6 -5 -4 -3 -2 -1]这种情况.
			6是最新的，后面是旧数据，返回结果	
			*/		
			if(CodeIN[i+1]<0)return i;
			}
		return RunTimeLoggerDepth-1;//找到序列末尾，返回序列末尾的值
		}
	else if(CodeIN[0]<0)
	  {
		for(i=0;i<RunTimeLoggerDepth-1;i++)//小于0
			{
			/*
	                   i i+1
			[-10 -9 -8 -7 -6 6 7 8 9 10]这种情况.
			-6是最新的，后面的是旧数据，返回结果	
			*/
			if(CodeIN[i+1]>0)return i;
			/*
										i  i+1
			[-10 -9 -8 -7 -6 0 0 0 0 0]这种情况.
			6是最新的，后面啥也没有了,返回结果
			*/		
			if(CodeIN[i+1]==0)return i;
			}	
		return RunTimeLoggerDepth-1;//找到序列末尾，返回序列末尾的值
		}
	return 0;//等于0，直接从这里开始
	}
#pragma pop

	
/*******************************************
遍历并计算Log区域的当前已用空间（计算结果包括
日志数据区，缓存区以及缓存区前后部的Guard Key）
********************************************/		
int CalcCurrentAvailableLogCount(void)
	{
	int i,result;
	RunLogDataUnionDef Data;
	for(i=0;i<RunTimeLoggerDepth;i++)
		{
		WatchDog_Feed();
		Data.DataSec.IsRunlogHasContent=false;
		if(!LoadRunLogDataFromROM(&Data,i))return -1;
	  if(!Data.DataSec.IsRunlogHasContent)break;
		}
	if(i>1)i--;     //其余情况固定-1
	else if(!i)i=1; //统计的数量为0时固定=1
	//计算并返回结果
  result=i*sizeof(signed short);		
	result+=8+RunTimeLogCacheBase+(i*sizeof(RunLogDataUnionDef));
	return result;
	}
	
/*******************************************
将日志内的数据储存区进行CRC32数值的计算
输入：遥测数据的union和日志数据的结构体
********************************************/	
unsigned int CalcLogContentCRC32(RunLogDataUnionDef *DIN)
	{
 unsigned int DATACRCResult;
 char i;
 unsigned short StorBuf;
 CKCU_PeripClockConfig_TypeDef CLKConfig={{0}};
 //初始化CRC32      
 CLKConfig.Bit.CRC = 1;
 CKCU_PeripClockConfig(CLKConfig,ENABLE);//启用CRC-32时钟  
 CRC_DeInit(HT_CRC);//清除配置
 HT_CRC->SDR = 0x0;//CRC-32 poly: 0x04C11DB7  
 HT_CRC->CR = CRC_32_POLY | CRC_BIT_RVS_WR | CRC_BIT_RVS_SUM | CRC_BYTE_RVS_SUM | CRC_CMPL_SUM;
 //开始校验
 for(i=0;i<4;i++)wb(&HT_CRC->DR,DIN->DataSec.LogKey[i]);
 DATACRCResult=CRC_Process(HT_CRC,&DIN->DataSec.Data.ContentBuf[0],sizeof(LogContentDef));//将内容写入到CRC寄存器内
 //对出来的结果进行混淆继续校验
 CRC_DeInit(HT_CRC);//清除CRC结果
 HT_CRC->SDR = 0x0;//CRC-32 poly: 0x04C11DB7  
 HT_CRC->CR = CRC_32_POLY | CRC_BIT_RVS_WR | CRC_BIT_RVS_SUM | CRC_BYTE_RVS_SUM | CRC_CMPL_SUM;
	for(i=0;i<16;i++)
		{
		switch(DATACRCResult&0x03)
			{
			case 0:StorBuf='G';break;
			case 1:StorBuf='j';break;
			case 2:StorBuf='!';break;
			case 3:StorBuf='o';break;
			}
		DATACRCResult>>=2; //右移两位
		StorBuf^=0x01<<(i%8); //和i的8次结果进行XOR
		wb(&HT_CRC->DR,StorBuf+i);
		}
 //二次校验完毕计算结果
 DATACRCResult=HT_CRC->CSR^0xA3526E4F;
 CRC_DeInit(HT_CRC);//清除CRC结果
 CKCU_PeripClockConfig(CLKConfig,DISABLE);//禁用CRC-32时钟节省电力
 return DATACRCResult;
	}
/*******************************************
使用空的内容填充运行日志结构体的数据部分。这
个函数主要是在上电自检时检测到损坏的log entry
以及清空整个运行日志的时候用的。
输入：遥测数据的union
********************************************/
void LogDataSectionInit(RunLogDataUnionDef *DIN)
  {
	//恢复头部
	DIN->DataSec.IsRunlogHasContent=false;
	DIN->DataSec.TotalLogCount=0;
	DIN->DataSec.LogIncrementCode=0;
	strncpy(DIN->DataSec.LogKey,RunTimeLogKey,4);
	//恢复基础设置
	DIN->DataSec.Data.Content.IsSystemBootFromSafeMode=false; //并非从安全模式启动
	DIN->DataSec.Data.Content.IsEnablePunish=false; //清除日志
	DIN->DataSec.Data.Content.BalanceTime=0; //总计均衡时间
	DIN->DataSec.Data.Content.ChargeTime=0; //总计充电时间
	DIN->DataSec.Data.Content.UnbalanceBatteryAh=0; //未经平衡的电池Ah数
	DIN->DataSec.Data.Content.TotalChargeAh=0; //总计充入的Ah数
	DIN->DataSec.Data.Content.TotalChargeWh=0; //总计充入的Wh数
	DIN->DataSec.Data.Content.DischargeTime=0; //总计放电时间
	DIN->DataSec.Data.Content.TotalDischargeAh=0; //总计放出的Ah数
	DIN->DataSec.Data.Content.TotalDischargeWh=0; //总计放出的Wh数
	DIN->DataSec.Data.Content.SysMaxTemp=-100; //系统最高温度
	DIN->DataSec.Data.Content.MaximumTypeCPower=-100;
	DIN->DataSec.Data.Content.MaximumTypeCCurrent=-100; //采集数据
	DIN->DataSec.Data.Content.MaximumBattCurrent=-50; //电池端最大电流 
	DIN->DataSec.Data.Content.SystemSCPCount=0;
	DIN->DataSec.Data.Content.VBUSOVPCount=0;
	DIN->DataSec.Data.Content.SystemOCPCount=0; //触发过流和短路保护的次数
	}
/*******************************************
将运行日志的log区域清空恢复为初始状态，清除
掉所有的日志内容。
输出:如果成功清除,则返回true,否则返回false
*******************************************/
bool ResetRunTimeLogArea(void)
 {
 int i;
 IncCodeROMUnionDef CodeBuf;	
 //复位系统RAM中的数据区域和存储
 LogDataSectionInit(&RunLogEntry.Data);
 RunLogEntry.ProgrammedEntry=0;
 CalcLastLogCRCBeforePO();
 RunLogEntry.CurrentDataCRC=RunLogEntry.LastDataCRC;//计算CRC-32
 //重置EEPROM内的数据
 for(i=0;i<RunTimeLoggerDepth;i++)
	 {
	 if(!SaveRunLogDataToROM(&RunLogEntry.Data,i))return false; //写入空白日志
	 if(!UpdateLogCache(RunLogEntry.Data.DataSec.LogIncrementCode,i))return false; //写入缓存区域
	 WatchDog_Feed(); //喂狗
	 }
 //重建缓存key区域
 if(!ReadEntireLogCache(&CodeBuf))return false;
 for(i=0;i<sizeof(LogAreaFrontKey);i++)
		{
		if(CodeBuf.CodeData.CacheFrontKey[i]!=LogAreaFrontKey[i])break;
		if(CodeBuf.CodeData.CacheRearKey[i]!=LogAreaRearKey[i])break;
		WatchDog_Feed();
		}	 
 if(i<sizeof(LogAreaFrontKey))WriteLogCacheKeyArea();	  //检查到key区域损坏，进行修复
 //操作完毕，返回true
 return true;
 }
 
/********************************************
给GUI处理函数用于检查Cache状态并进行损坏数据的
重建的处理
********************************************/ 
bool RunLogModule_VerifyDBHandler(int EntryNum,bool *IsCorrupted)
 {
 RunLogDataUnionDef Data; 
 signed short Result;
 *IsCorrupted=false;
 WatchDog_Feed();
 if(!LoadRunLogDataFromROM(&Data,EntryNum))return false;
 if(!ReadOneCodeFromIndex(EntryNum,&Result))return false;
 if(CalcLogContentCRC32(&Data)!=Data.DataSec.LogContentSum||strncmp(Data.DataSec.LogKey,RunTimeLogKey,4))
	 {
	 *IsCorrupted=true;
	 LogDataSectionInit(&Data);
	 SaveRunLogDataToROM(&Data,EntryNum);
	 }
	else if(Data.DataSec.LogIncrementCode!=Result)
	 {
	 *IsCorrupted=true;
	 UpdateLogCache(Data.DataSec.LogIncrementCode,EntryNum);
	 }
 //正常执行，返回true
 return true;
 }
 
/********************************************
当缓存数据库异常的时候，进行缓存重建和数据修复
的处理
********************************************/
static void ReCreteCacheHexValCalc(char Statu,char Result[3])
	{
	memset(Result,0,3);
	snprintf(Result,3,"%02X",Statu);
	}
 
static void RunLogModule_ReCreateCache(signed short *IncCodeCache,int CheckState,char Statu)
 {
 int i,CorruptedCount=0;
 char CorruptMsg[32];
 RunLogDataUnionDef Data; 
 bool IsLogFault; 
 char RebuildStatuSTR[3]={0};
 //写入Guard Key
 if(!WriteLogCacheKeyArea())
		 {
	   ShowPostInfo(CheckState,"写入Key失败\0","EE",Msg_Fault);
		 SelfTestErrorHandler();		 
		 }	 
 //刷新实际的日志区 
 for(i=0;i<RunTimeLoggerDepth;i++)
	 {
	 //进行重建进度运算	 
	 //显示加载进度
	 if(i==(RunTimeLoggerDepth/4))
			{
			CheckState++;
			ReCreteCacheHexValCalc(Statu+1,RebuildStatuSTR);
			ShowPostInfo(CheckState,"重建进度25%",RebuildStatuSTR,Msg_INFO);	
			IsLogFault=false;
			}
   if(i==(RunTimeLoggerDepth/2))
			{
			CheckState++;
			ReCreteCacheHexValCalc(Statu+2,RebuildStatuSTR);
			ShowPostInfo(CheckState,"重建进度50%",RebuildStatuSTR,Msg_INFO);		
			IsLogFault=false;	
			}
   if(i==((RunTimeLoggerDepth*3)/4))
			{
			CheckState++;
			ReCreteCacheHexValCalc(Statu+3,RebuildStatuSTR);
			ShowPostInfo(CheckState,"重建进度75%",RebuildStatuSTR,Msg_INFO);
			IsLogFault=false;	
			} 
	 //从ROM内读取数据
	 if(!LoadRunLogDataFromROM(&Data,i))
      {
	   	ShowPostInfo(CheckState,"存储器读取异常\0","E8",Msg_Fault);
			SelfTestErrorHandler();
	    }
	 //检查log entry(如果发生损坏，则使用默认配置去重写)
	 if(CalcLogContentCRC32(&Data)!=Data.DataSec.LogContentSum||strncmp(Data.DataSec.LogKey,RunTimeLogKey,4))
	   {
		 CorruptedCount++;
		 IncCodeCache[i]=0;//该处因为已经损坏，读取到的自增码等于0
		 if(!IsLogFault)
			 {
			 delay_Second(1);
			 ShowPostInfo(CheckState,"检测到损坏数据\0","W7",Msg_Warning);
			 delay_Second(1);
			 memset(CorruptMsg,0,32);
			 snprintf(CorruptMsg,32,"位于地址:%d处",i);
			 ShowPostInfo(CheckState,CorruptMsg,"W7",Msg_Warning);
			 delay_Second(1);
			 }
		 LogDataSectionInit(&Data);
		 SaveRunLogDataToROM(&Data,i);
		 if(!IsLogFault)
			 {
			 ShowPostInfo(CheckState,"已进行自动修正\0","W7",Msg_Warning);
			 delay_Second(1);
			 IsLogFault=true;
			 }
		 }
	 //检查通过的entry，将自增码写入到缓冲区内
	 else IncCodeCache[i]=Data.DataSec.LogIncrementCode;
	 //将重建之后的缓存数据写入到结果里面
	 if(!UpdateLogCache(IncCodeCache[i],i))
		 {
	   ShowPostInfo(CheckState,"更新缓存数据失败\0","EF",Msg_Fault);
		 SelfTestErrorHandler();		 
		 }
	 }
 ReCreteCacheHexValCalc(Statu+4,RebuildStatuSTR);
 ShowPostInfo(CheckState,"缓存重建完成\0",RebuildStatuSTR,Msg_INFO);
 delay_Second(1);
 if(CorruptedCount>0)
	 { 
	 memset(CorruptMsg,0,32);
	 snprintf(CorruptMsg,32,"共发现%d条损坏数据",CorruptedCount);
	 ShowPostInfo(CheckState,CorruptMsg,RebuildStatuSTR,Msg_Warning);
	 delay_Second(1);
	 }
 }

/********************************************
驱动上电自检时检测整个运行数据区域的自检函数
负责检查并修复损坏的log entry，然后根据entry
内写入的自增码判断哪个entry是最新的，从里面
读取数据
********************************************/
void RunLogModule_POR(void)
 {
 int i,CodeCacheEntryData,j;
 RunLogDataUnionDef Data;
 bool IsLogEmpty;
 IncCodeROMUnionDef CodeBuf;
 //首先从EEPROM内读取一遍数据库缓存
 ShowPostInfo(55,"读取数据库缓存\0","14",Msg_Statu);
 if(!ReadEntireLogCache(&CodeBuf))
	 {
	 ShowPostInfo(55,"存储器读取异常\0","E8",Msg_Fault);
	 SelfTestErrorHandler();
	 }
 //检查数据库缓存
 for(i=0;i<sizeof(LogAreaFrontKey);i++)
	{
  if(CodeBuf.CodeData.CacheFrontKey[i]!=LogAreaFrontKey[i])break;
	if(CodeBuf.CodeData.CacheRearKey[i]!=LogAreaRearKey[i])break;
	}	 
 if(i<sizeof(LogAreaFrontKey))
  {
	//Guard Key损坏，数据库存在异常，尝试重建缓存
	ShowPostInfo(55,"缓存数据库异常\0","15",Msg_Warning);
  delay_Second(1);
	ShowPostInfo(55,"尝试重建缓存\0","15",Msg_Warning);
	RunLogModule_ReCreateCache(CodeBuf.CodeData.IncCodeCache,55,0x15);
	}
 //进行缓存命中尝试
 CodeCacheEntryData=FindLatestEntryViaIncCode(CodeBuf.CodeData.IncCodeCache);
 ShowPostInfo(59,"尝试缓存命中\0","1A",Msg_Statu);
 IsLogEmpty=true;
 for(i=0;i<RunTimeLoggerDepth+1;i++)
	{	 
 if(!LoadRunLogDataFromROM(&Data,CodeCacheEntryData))	//从ROM内读取选择的Entry作为目前数据的内容
   {
	 ShowPostInfo(59,"数据库读取失败\0","F0",Msg_Fault);
	 SelfTestErrorHandler();
	 } 
  //检查数据是否OK
	if(CalcLogContentCRC32(&Data)==Data.DataSec.LogContentSum&&!strncmp(Data.DataSec.LogKey,RunTimeLogKey,4))break;
	//数据不OK，往前找一个entry
	if(IsLogEmpty)
		{
		IsLogEmpty=false;
		ShowPostInfo(59,"缓存命中异常\0","1B",Msg_Warning);
		delay_Second(1);
		ShowPostInfo(59,"建议运行DB检查\0","1B",Msg_Warning);	
		delay_Second(1);
		}
  if(CodeCacheEntryData>0)CodeCacheEntryData--;
	else CodeCacheEntryData=RunTimeLoggerDepth-1;
	}
 if(IsLogEmpty)ShowPostInfo(60,"缓存命中成功\0","21",Msg_Statu);
 if(i==RunTimeLoggerDepth+1)
  {
	//整个数据库都遍历了一遍，没找到合法的数据，尝试重建数据库
	ShowPostInfo(59,"日志数据异常\0","1C",Msg_Warning);
  delay_Second(1);
	ShowPostInfo(59,"尝试重建缓存\0","1C",Msg_Warning);
	RunLogModule_ReCreateCache(CodeBuf.CodeData.IncCodeCache,59,0x1C);
	ReadEntireLogCache(&CodeBuf); //重建缓存之后需要重新读一遍数据库
	CodeCacheEntryData=FindLatestEntryViaIncCode(CodeBuf.CodeData.IncCodeCache); //重新获取一次自增数据
	}
 //将指定Entry的数据直接读入系统日志中 
 ShowPostInfo(63,"加载库仑计数据\0","22",Msg_Statu);
 if(!LoadRunLogDataFromROM(&RunLogEntry.Data,CodeCacheEntryData))	//从ROM内读取选择的Entry作为目前数据的内容
   {
	 ShowPostInfo(63,"数据读取失败\0","F0",Msg_Fault);
	 SelfTestErrorHandler();
	 } 	 
 ShowPostInfo(63,"缓存一致性检查\0","23",Msg_Statu);
 if(RunLogEntry.Data.DataSec.LogIncrementCode!=CodeBuf.CodeData.IncCodeCache[CodeCacheEntryData])
	{
	//如果读出来的entry日志和目标的数据不一致，则说明缓存数据异常，此时更新日志
	UpdateLogCache(RunLogEntry.Data.DataSec.LogIncrementCode,CodeCacheEntryData);  //写入缓存区域和数据域 
	ShowPostInfo(63,"缓存数据异常\0","24",Msg_Warning); 
	delay_Second(1);
	ShowPostInfo(63,"已进行覆盖处理\0","24",Msg_Warning);
	delay_Second(1);
	}
 RunLogEntry.LastDataCRC=CalcRunLogCRC32(&RunLogEntry.Data);
 RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data);//计算CRC-32 
 ///开始进行后续检查
 IsLogEmpty=true;
 if(CodeBuf.CodeData.IncCodeCache[0])for(i=0;i<RunTimeLoggerDepth;i++)if(CodeBuf.CodeData.IncCodeCache[i])IsLogEmpty=false; //如果第一个入口不是空的，则检查entry是不是已经空了 
 if(IsLogEmpty)RunLogEntry.ProgrammedEntry=0;//如果目前事件日志一组记录都没有，则从0开始记录
 else 
	{
	ShowPostInfo(65,"检查库仑计数据内容\0","25",Msg_Statu);
	j=0;
	RunLogEntry.ProgrammedEntry=(CodeCacheEntryData+1)%RunTimeLoggerDepth;//目前entry已经有数据了，从下一条entry开始
	for(i=0;i<4;i++)
	  {
		if(!LoadRunLogDataFromROM(&Data,(i+RunLogEntry.ProgrammedEntry)%RunTimeLoggerDepth))
      {
	   	ShowPostInfo(65,"存储器读取异常\0","F0",Msg_Fault);
			SelfTestErrorHandler();
	    }	
		//找到损坏的entry，统计损坏的条目数
		if(!Data.DataSec.TotalLogCount||!Data.DataSec.LogIncrementCode)j++;
		}	
	
	//当前要写入的条目发生损坏且只有该条目发生损坏，进行修复处理，然后指向下一个条目
	if(j==1)
			{
			ShowPostInfo(65,"数据库内容异常\0","26",Msg_Warning);
			delay_Second(1);
			ShowPostInfo(65,"已进行覆盖处理\0","26",Msg_Warning);
			delay_Second(1);
			ForceWriteRuntimelog(); //强制写入当前条目并更新到下一个内容
			}
	}
 //进行攻击监测
 AttackDetectInit();
 }
