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

//变量
RunLogEntryStrDef RunLogEntry;
static char SampleCount=0;
static float BATSample[2]={0};
static bool IsLogSaved=false;

//外部函数声明
void AttackDetectInit(void);

//更新日志
void UpdataRunTimeLog(void)
	{
	char i;
	float Cap;
	extern bool IsUpdateCDUI;
	BatteryStateDef State=Batt_StandBy;
	//获取Type-C状态
	IP2366_GetChargerState(&State);
	if(!IP2366_QueryCurrentStateIsACC(State)||fabsf(ADCO.Ibatt)<=MinimumCurrentFactor) //芯片处于非充放电状态，复位采样缓存
		{
		SampleCount=8;
		for(i=0;i<2;i++)BATSample[i]=0;
		//日志未保存，计算CRC32
		if(!IsLogSaved)
			{
			IsLogSaved=true;
			RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data); //计算运行日志的CRC32
			WriteRuntimeLogToROM(); //保存日志
			}
		}
	//进行平均值计算
	else if(SampleCount>0)
		{
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
输出:如果成功读取,则返回true,否则返回false
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
	signed char SelfIncCode,OldCode;
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
  if(SaveRunLogDataToROM(&RunLogEntry.Data,RunLogEntry.ProgrammedEntry))
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
static int FindLatestEntryViaIncCode(signed char *CodeIN)
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
	                i  i+1
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
将日志内的数据储存区进行CRC32数值的计算
输入：遥测数据的union和日志数据的结构体
********************************************/	
unsigned int CalcLogContentCRC32(RunLogDataUnionDef *DIN)
	{
 unsigned int DATACRCResult;
 char i;
 unsigned char StorBuf;
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
	DIN->DataSec.Data.Content.MaximumBattCurrent=0; //电池端最大电流 
	}
/*******************************************
将运行日志的log区域清空恢复为初始状态，清除
掉所有的日志内容。
输出:如果成功清除,则返回true,否则返回false
*******************************************/
bool ResetRunTimeLogArea(void)
 {
 int i;
 //复位系统RAM中的数据区域和存储
 LogDataSectionInit(&RunLogEntry.Data);
 RunLogEntry.ProgrammedEntry=0;
 CalcLastLogCRCBeforePO();
 RunLogEntry.CurrentDataCRC=RunLogEntry.LastDataCRC;//计算CRC-32
 //重置RAM内的数据
 for(i=0;i<RunTimeLoggerDepth;i++)
	 {
	 if(!SaveRunLogDataToROM(&RunLogEntry.Data,i))return false;
	 WatchDog_Feed(); //喂狗
	 }
 //操作完毕，返回true
 return true;
 }
 
 /********************************************
驱动上电自检时检测整个运行数据区域的自检函数
负责检查并修复损坏的log entry，然后根据entry
内写入的自增码判断哪个entry是最新的，从里面
读取数据
********************************************/
void RunLogModule_POR(void)
 {
 int i,j;
 RunLogDataUnionDef Data;
 unsigned int CRCResult;
 bool IsLogEmpty,IsLogFault=false;
 signed char SelfIncBuf[RunTimeLoggerDepth];
 //首先我们需要把整个log区域遍历一遍
 ShowPostInfo(55,"检查数据库\0","20",Msg_Statu);
 for(i=0;i<RunTimeLoggerDepth;i++)
	 {
	 //显示加载进度
	 if(i==28)ShowPostInfo(57,"检查进度25%","21",Msg_Statu);	
   else if(i==55)ShowPostInfo(62,"检查进度50%","22",Msg_Statu);		
   else if(i==83)ShowPostInfo(65,"检查进度75%","23",Msg_Statu);
   if(i==28||i==55||i==83)IsLogFault=false;	 
	 //从ROM内读取数据
	 if(!LoadRunLogDataFromROM(&Data,i))
      {
	   	ShowPostInfo(55,"存储器读取异常\0","E5",Msg_Fault);
			SelfTestErrorHandler();
	    }
	 //检查log entry(如果发生损坏，则使用默认配置去重写)
	 CRCResult=CalcLogContentCRC32(&Data); //计算CRC32
	 if(CRCResult!=Data.DataSec.LogContentSum||strncmp(Data.DataSec.LogKey,RunTimeLogKey,4))
	   {
		 SelfIncBuf[i]=0;//该处因为已经损坏，读取到的自增码等于0
		 if(!IsLogFault)
			 {
			 ShowPostInfo(55,"检测到损坏数据\0","2E",Msg_Warning);
			 delay_Second(1);
			 }
		 LogDataSectionInit(&Data);
		 SaveRunLogDataToROM(&Data,i);
		 if(!IsLogFault)
			 {
			 ShowPostInfo(55,"已进行自动修正\0","2E",Msg_Warning);
			 delay_Second(1);
			 IsLogFault=true;
			 }
		 continue;
		 }
	 //检查通过的entry，将自增码写入到缓冲区内
	 SelfIncBuf[i]=Data.DataSec.LogIncrementCode;
	 }
 //遍历完毕，查询自增码获得最新的log entry并计算CRC32
 i=FindLatestEntryViaIncCode(SelfIncBuf);
 ShowPostInfo(65,"读取数据库\0","24",Msg_Statu);
 if(!LoadRunLogDataFromROM(&RunLogEntry.Data,i))//从ROM内读取选择的Entry作为目前数据的内容
    {
		ShowPostInfo(65,"数据库读取失败\0","E9",Msg_Fault);
		SelfTestErrorHandler();
	  }
 IsLogEmpty=true;
 if(SelfIncBuf[0])for(j=0;j<RunTimeLoggerDepth;j++)if(SelfIncBuf[j])IsLogEmpty=false; //如果第一个入口不是空的，则检查entry是不是已经空了
 ShowPostInfo(70,"加载库仑计数据\0","25",Msg_Statu);
 RunLogEntry.LastDataCRC=CalcRunLogCRC32(&RunLogEntry.Data);
 RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data);//计算CRC-32
 if(IsLogEmpty)RunLogEntry.ProgrammedEntry=0;//如果目前事件日志一组记录都没有，则从0开始记录
 else RunLogEntry.ProgrammedEntry=(i+1)%RunTimeLoggerDepth;//目前entry已经有数据了，从下一条entry开始
 //进行攻击监测
 AttackDetectInit();
 }
