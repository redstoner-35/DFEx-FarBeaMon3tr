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

//��������
float fmaxf(float x,float y);
float fminf(float x,float y);

//����
RunLogEntryStrDef RunLogEntry;
static char SampleCount=0;
static float BATSample[2]={0};
static bool IsLogSaved=false;

//�ⲿ��������
void AttackDetectInit(void);

//������־
void UpdataRunTimeLog(void)
	{
	char i;
	float Cap;
	extern bool IsUpdateCDUI;
	BatteryStateDef State=Batt_StandBy;
	//��ȡType-C״̬
	IP2366_GetChargerState(&State);
	if(!IP2366_QueryCurrentStateIsACC(State)||fabsf(ADCO.Ibatt)<=MinimumCurrentFactor) //оƬ���ڷǳ�ŵ�״̬����λ��������
		{
		SampleCount=8;
		for(i=0;i<2;i++)BATSample[i]=0;
		//��־δ���棬����CRC32
		if(!IsLogSaved)
			{
			IsLogSaved=true;
			RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data); //����������־��CRC32
			WriteRuntimeLogToROM(); //������־
			}
		}
	//����ƽ��ֵ����
	else if(SampleCount>0)
		{
		BATSample[0]+=ADCO.Vbatt;
		BATSample[1]+=ADCO.Ibatt;
		SampleCount--;
		}
	//ʱ�䵽�����вɼ�
	else
		{
		//��־�����Ѹ���
		IsLogSaved=false;
		LogHeader.IsRunlogHasContent=true;
		//����ƽ������
		for(i=0;i<2;i++)BATSample[i]/=(float)8;
		//�ۼƳ�ŵ�ʱ��
		if(State==Batt_discharging)LogData.DischargeTime++;
		else LogData.ChargeTime++;		 
    if(BalanceState)LogData.BalanceTime++; //���⿪��ʱ�ۼӾ���ʱ��		
		//����AH
		Cap=fabsf(BATSample[1])/(float)3600; //��ǰ����*1000 /3600��õ�AH
   	if(State!=Batt_discharging)LogData.TotalChargeAh+=Cap;
    else LogData.TotalDischargeAh+=Cap;
		if(!BalanceState)LogData.UnbalanceBatteryAh+=Cap; //������������ڹر�״̬��������ͳ����δ��������
    //����Wh
		Cap=fabsf(BATSample[0]*BATSample[1])/(float)3600;//��ǰ��ѹ*����/3600��õ�Wh
		if(State==Batt_discharging)LogData.TotalDischargeWh+=Cap;
	  else LogData.TotalChargeWh+=Cap;
    //��ȡ��ߵ�ص������¶�
		if(!ADCO.IsNTCOK)LogData.SysMaxTemp=-100;
		else if(LogData.SysMaxTemp<ADCO.Systemp)LogData.SysMaxTemp=ADCO.Systemp;
		if(LogData.MaximumBattCurrent<fabsf(BATSample[1]))LogData.MaximumBattCurrent=fabsf(BATSample[1]);
	  //��λ����
		SampleCount=8;
	  for(i=0;i<2;i++)BATSample[i]=0; 
		//�����ݲ鿴�˵��ڣ���������
		IsUpdateCDUI=true;
		}
	}

/*******************************************
��ָ����������־���������ROM��ָ����entry��
������д�뵽RAM�ڡ�

���룺���ң�����ݵ�union��Ŀ���ȡ��entry
���:����ɹ���ȡ,�򷵻�true,���򷵻�false
********************************************/
static bool LoadRunLogDataFromROM(RunLogDataUnionDef *DataOut,int LogEntryNum)
 {
 //�������Ĳ����Ǵ��
 if(DataOut==NULL||LogEntryNum<0||LogEntryNum>RunTimeLoggerDepth-1)return false;
 //��ʼ��ȡ
 if(M24C512_PageRead(DataOut->DataCbuf,RunTimeLogBase+(LogEntryNum*sizeof(RunLogDataUnionDef)),sizeof(RunLogDataUnionDef)))
	 return false;
 //��ȡ��ϣ�����true
 return true;
 }

/*******************************************
��ָ����������־��������д�뵽ROM��ָ����
entry�С�
���룺���ң�����ݵ�union��Ŀ��д���entry
���:����ɹ���ȡ,�򷵻�true,���򷵻�false
********************************************/
static bool SaveRunLogDataToROM(RunLogDataUnionDef *DataIn,int LogEntryNum)
 {
 //�������Ĳ����Ǵ��
 if(DataIn==NULL||LogEntryNum<0||LogEntryNum>RunTimeLoggerDepth-1)return false;
 //����CRC32
 DataIn->DataSec.LogContentSum=CalcLogContentCRC32(DataIn); //����CRC32
 //��ʼд��
 if(M24C512_PageWrite(DataIn->DataCbuf,RunTimeLogBase+(LogEntryNum*sizeof(RunLogDataUnionDef)),sizeof(RunLogDataUnionDef)))
	 return false;
 //д����ϣ�����true
 return true;
 }

/*******************************************
���㴫�����ݵ�CRC32У�������ȷ���Ƿ�Ҫдlog
����ȵȡ�
���룺ң�����ݵ�union
������������ݵ�CRC32У���
********************************************/
unsigned int CalcRunLogCRC32(RunLogDataUnionDef *DIN)
{
 unsigned int DATACRCResult; 
 int i;
 CKCU_PeripClockConfig_TypeDef CLKConfig={{0}};
 //��ʼ��CRC32      
 CLKConfig.Bit.CRC = 1;
 CKCU_PeripClockConfig(CLKConfig,ENABLE);//����CRC-32ʱ��  
 CRC_DeInit(HT_CRC);//�������
 HT_CRC->SDR = 0x0;//CRC-32 poly: 0x04C11DB7  
 HT_CRC->CR = CRC_32_POLY | CRC_BIT_RVS_WR | CRC_BIT_RVS_SUM | CRC_BYTE_RVS_SUM | CRC_CMPL_SUM;
 //��ʼУ��
 for(i=0;i<sizeof(RunLogDataUnionDef);i++)wb(&HT_CRC->DR,DIN->DataCbuf[i]);//������д�뵽CRC�Ĵ�����
 //У����ϼ�����
 DATACRCResult=HT_CRC->CSR^0xA352EE4F;
 CRC_DeInit(HT_CRC);//���CRC���
 CKCU_PeripClockConfig(CLKConfig,DISABLE);//����CRC-32ʱ�ӽ�ʡ����
 return DATACRCResult;
}	
/*******************************************
���ֵ�Ͳת��Ϊ����״̬���Լ�ǰ������Ŀǰ����
��CRC32�����½ṹ�����������ݼ�¼��CRCУ���
����Ա�LOG�Ƿ񱻸���
********************************************/ 
void CalcLastLogCRCBeforePO(void)
  {
	//����CRC����д�ṹ��
	RunLogEntry.LastDataCRC=CalcRunLogCRC32(&RunLogEntry.Data);
	}

/*******************************************
ǿ��ִ�н�������־д�뵽ROM�Ķ������������
��Ҫ������ģʽ����������
*******************************************/
#pragma push
#pragma O0
void ForceWriteRuntimelog(void)
  {
	signed char SelfIncCode,OldCode;
	//�����µ�������
	SelfIncCode=RunLogEntry.Data.DataSec.LogIncrementCode;
	OldCode=RunLogEntry.Data.DataSec.LogIncrementCode;
  if(SelfIncCode<0)SelfIncCode--;
  else if(SelfIncCode>0)SelfIncCode++;
	else SelfIncCode=1; //���������λ�ڸ�����Χ����������-1�����1��������0�������Ϊ1
	if(SelfIncCode<(-RunTimeLoggerDepth))SelfIncCode=1;
	if(SelfIncCode>RunTimeLoggerDepth)SelfIncCode=-1;//��������뵽��������ת����һ������
  RunLogEntry.Data.DataSec.LogIncrementCode=SelfIncCode;//������õ�������д��ȥ
	RunLogEntry.Data.DataSec.TotalLogCount++; //��־д�������+1
	//���Ա��
  if(SaveRunLogDataToROM(&RunLogEntry.Data,RunLogEntry.ProgrammedEntry))
	  {
		CalcLastLogCRCBeforePO();  //��̽������µ�log��CRC-32ֵ�滻��ȥ�����ظ�д��
    RunLogEntry.ProgrammedEntry=(RunLogEntry.ProgrammedEntry+1)%RunTimeLoggerDepth;//��̳ɹ���ָ����һ��entry������ﵽ���entry��Ŀ��ת����  		
		}
	else 
		RunLogEntry.Data.DataSec.LogIncrementCode=OldCode;//���ʧ�ܣ�entry�������ӵ�ͬʱ����ԭ�����˵�������
	}

/*******************************************
���ֵ�Ͳ�رպ�������Ҫ������logд�뵽ROM��
�����ڼ䣬����������Ҫ��֤����log�Ƿ����仯
��������仯����ʼд�롣
*******************************************/
void WriteRuntimeLogToROM(void)
  {
  //���CRC-32��ͬ˵�����е�logû�з����ı�,����Ҫ����
	if(RunLogEntry.LastDataCRC==RunLogEntry.CurrentDataCRC)return;
  //��ʼ���
	ForceWriteRuntimelog();
	}
	
/*******************************************
�Ӷ�ȡ����increment-code�������ҳ����¼�¼��
entry��
���룺��������code������
��������µ�һ��entry���ڵ�λ��
********************************************/
static int FindLatestEntryViaIncCode(signed char *CodeIN)
  {
	int i;
  //�ж�����ĵ�0��Ԫ���������Ǹ�����0
	if(CodeIN[0]>0)
	  {
		for(i=0;i<RunTimeLoggerDepth-1;i++)//����0
			{
			/*
								i i+1
			[1 2 3 4 5 6 0 0 0 0 0 ]�������.
			6�����µģ�����ɶҲû���˷��ؽ��	
			*/
			if(CodeIN[i+1]==0)return i;
			/*
								i i+1
			[1 2 3 4 5 6 -5 -4 -3 -2 -1]�������.
			6�����µģ������Ǿ����ݣ����ؽ��	
			*/		
			if(CodeIN[i+1]<0)return i;
			}
		return RunTimeLoggerDepth-1;//�ҵ�����ĩβ����������ĩβ��ֵ
		}
	else if(CodeIN[0]<0)
	  {
		for(i=0;i<RunTimeLoggerDepth-1;i++)//С��0
			{
			/*
	                i  i+1
			[-10 -9 -8 -7 -6 6 7 8 9 10]�������.
			-6�����µģ�������Ǿ����ݣ����ؽ��	
			*/
			if(CodeIN[i+1]>0)return i;
			/*
										i  i+1
			[-10 -9 -8 -7 -6 0 0 0 0 0]�������.
			6�����µģ�����ɶҲû����,���ؽ��
			*/		
			if(CodeIN[i+1]==0)return i;
			}	
		return RunTimeLoggerDepth-1;//�ҵ�����ĩβ����������ĩβ��ֵ
		}
	return 0;//����0��ֱ�Ӵ����￪ʼ
	}
#pragma pop

/*******************************************
����־�ڵ����ݴ���������CRC32��ֵ�ļ���
���룺ң�����ݵ�union����־���ݵĽṹ��
********************************************/	
unsigned int CalcLogContentCRC32(RunLogDataUnionDef *DIN)
	{
 unsigned int DATACRCResult;
 char i;
 unsigned char StorBuf;
 CKCU_PeripClockConfig_TypeDef CLKConfig={{0}};
 //��ʼ��CRC32      
 CLKConfig.Bit.CRC = 1;
 CKCU_PeripClockConfig(CLKConfig,ENABLE);//����CRC-32ʱ��  
 CRC_DeInit(HT_CRC);//�������
 HT_CRC->SDR = 0x0;//CRC-32 poly: 0x04C11DB7  
 HT_CRC->CR = CRC_32_POLY | CRC_BIT_RVS_WR | CRC_BIT_RVS_SUM | CRC_BYTE_RVS_SUM | CRC_CMPL_SUM;
 //��ʼУ��
 for(i=0;i<4;i++)wb(&HT_CRC->DR,DIN->DataSec.LogKey[i]);
 DATACRCResult=CRC_Process(HT_CRC,&DIN->DataSec.Data.ContentBuf[0],sizeof(LogContentDef));//������д�뵽CRC�Ĵ�����
 //�Գ����Ľ�����л�������У��
 CRC_DeInit(HT_CRC);//���CRC���
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
		DATACRCResult>>=2; //������λ
		StorBuf^=0x01<<(i%8); //��i��8�ν������XOR
		wb(&HT_CRC->DR,StorBuf+i);
		}
 //����У����ϼ�����
 DATACRCResult=HT_CRC->CSR^0xA3526E4F;
 CRC_DeInit(HT_CRC);//���CRC���
 CKCU_PeripClockConfig(CLKConfig,DISABLE);//����CRC-32ʱ�ӽ�ʡ����
 return DATACRCResult;
	}
/*******************************************
ʹ�ÿյ��������������־�ṹ������ݲ��֡���
��������Ҫ�����ϵ��Լ�ʱ��⵽�𻵵�log entry
�Լ��������������־��ʱ���õġ�
���룺ң�����ݵ�union
********************************************/
void LogDataSectionInit(RunLogDataUnionDef *DIN)
  {
	//�ָ�ͷ��
	DIN->DataSec.IsRunlogHasContent=false;
	DIN->DataSec.TotalLogCount=0;
	DIN->DataSec.LogIncrementCode=0;
	strncpy(DIN->DataSec.LogKey,RunTimeLogKey,4);
	//�ָ���������
	DIN->DataSec.Data.Content.IsEnablePunish=false; //�����־
	DIN->DataSec.Data.Content.BalanceTime=0; //�ܼƾ���ʱ��
	DIN->DataSec.Data.Content.ChargeTime=0; //�ܼƳ��ʱ��
	DIN->DataSec.Data.Content.UnbalanceBatteryAh=0; //δ��ƽ��ĵ��Ah��
	DIN->DataSec.Data.Content.TotalChargeAh=0; //�ܼƳ����Ah��
	DIN->DataSec.Data.Content.TotalChargeWh=0; //�ܼƳ����Wh��
	DIN->DataSec.Data.Content.DischargeTime=0; //�ܼƷŵ�ʱ��
	DIN->DataSec.Data.Content.TotalDischargeAh=0; //�ܼƷų���Ah��
	DIN->DataSec.Data.Content.TotalDischargeWh=0; //�ܼƷų���Wh��
	DIN->DataSec.Data.Content.SysMaxTemp=-100; //ϵͳ����¶�
	DIN->DataSec.Data.Content.MaximumBattCurrent=0; //��ض������� 
	}
/*******************************************
��������־��log������ջָ�Ϊ��ʼ״̬�����
�����е���־���ݡ�
���:����ɹ����,�򷵻�true,���򷵻�false
*******************************************/
bool ResetRunTimeLogArea(void)
 {
 int i;
 //��λϵͳRAM�е���������ʹ洢
 LogDataSectionInit(&RunLogEntry.Data);
 RunLogEntry.ProgrammedEntry=0;
 CalcLastLogCRCBeforePO();
 RunLogEntry.CurrentDataCRC=RunLogEntry.LastDataCRC;//����CRC-32
 //����RAM�ڵ�����
 for(i=0;i<RunTimeLoggerDepth;i++)
	 {
	 if(!SaveRunLogDataToROM(&RunLogEntry.Data,i))return false;
	 WatchDog_Feed(); //ι��
	 }
 //������ϣ�����true
 return true;
 }
 
 /********************************************
�����ϵ��Լ�ʱ���������������������Լ캯��
�����鲢�޸��𻵵�log entry��Ȼ�����entry
��д����������ж��ĸ�entry�����µģ�������
��ȡ����
********************************************/
void RunLogModule_POR(void)
 {
 int i,j;
 RunLogDataUnionDef Data;
 unsigned int CRCResult;
 bool IsLogEmpty,IsLogFault=false;
 signed char SelfIncBuf[RunTimeLoggerDepth];
 //����������Ҫ������log�������һ��
 ShowPostInfo(55,"������ݿ�\0","20",Msg_Statu);
 for(i=0;i<RunTimeLoggerDepth;i++)
	 {
	 //��ʾ���ؽ���
	 if(i==28)ShowPostInfo(57,"������25%","21",Msg_Statu);	
   else if(i==55)ShowPostInfo(62,"������50%","22",Msg_Statu);		
   else if(i==83)ShowPostInfo(65,"������75%","23",Msg_Statu);
   if(i==28||i==55||i==83)IsLogFault=false;	 
	 //��ROM�ڶ�ȡ����
	 if(!LoadRunLogDataFromROM(&Data,i))
      {
	   	ShowPostInfo(55,"�洢����ȡ�쳣\0","E5",Msg_Fault);
			SelfTestErrorHandler();
	    }
	 //���log entry(��������𻵣���ʹ��Ĭ������ȥ��д)
	 CRCResult=CalcLogContentCRC32(&Data); //����CRC32
	 if(CRCResult!=Data.DataSec.LogContentSum||strncmp(Data.DataSec.LogKey,RunTimeLogKey,4))
	   {
		 SelfIncBuf[i]=0;//�ô���Ϊ�Ѿ��𻵣���ȡ�������������0
		 if(!IsLogFault)
			 {
			 ShowPostInfo(55,"��⵽������\0","2E",Msg_Warning);
			 delay_Second(1);
			 }
		 LogDataSectionInit(&Data);
		 SaveRunLogDataToROM(&Data,i);
		 if(!IsLogFault)
			 {
			 ShowPostInfo(55,"�ѽ����Զ�����\0","2E",Msg_Warning);
			 delay_Second(1);
			 IsLogFault=true;
			 }
		 continue;
		 }
	 //���ͨ����entry����������д�뵽��������
	 SelfIncBuf[i]=Data.DataSec.LogIncrementCode;
	 }
 //������ϣ���ѯ�����������µ�log entry������CRC32
 i=FindLatestEntryViaIncCode(SelfIncBuf);
 ShowPostInfo(65,"��ȡ���ݿ�\0","24",Msg_Statu);
 if(!LoadRunLogDataFromROM(&RunLogEntry.Data,i))//��ROM�ڶ�ȡѡ���Entry��ΪĿǰ���ݵ�����
    {
		ShowPostInfo(65,"���ݿ��ȡʧ��\0","E9",Msg_Fault);
		SelfTestErrorHandler();
	  }
 IsLogEmpty=true;
 if(SelfIncBuf[0])for(j=0;j<RunTimeLoggerDepth;j++)if(SelfIncBuf[j])IsLogEmpty=false; //�����һ����ڲ��ǿյģ�����entry�ǲ����Ѿ�����
 ShowPostInfo(70,"���ؿ��ؼ�����\0","25",Msg_Statu);
 RunLogEntry.LastDataCRC=CalcRunLogCRC32(&RunLogEntry.Data);
 RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data);//����CRC-32
 if(IsLogEmpty)RunLogEntry.ProgrammedEntry=0;//���Ŀǰ�¼���־һ���¼��û�У����0��ʼ��¼
 else RunLogEntry.ProgrammedEntry=(i+1)%RunTimeLoggerDepth;//Ŀǰentry�Ѿ��������ˣ�����һ��entry��ʼ
 //���й������
 AttackDetectInit();
 }
