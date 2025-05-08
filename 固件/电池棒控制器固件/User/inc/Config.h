#ifndef _CFG_
#define _CFG_

#include "IP2366_REG.h"
#include "LCD_Init.h"

typedef enum
	{
	InstantCTest_NotTriggered, //�´��ϵ�ֱ�ӽ���������δ����
	InstantCTest_Armed, //�´�ֱ�ӽ����������Ѽ����δ����
	InstantCTest_EnteredOK //˲ʱ��������˳������
	}InstantCTestDef;

typedef enum
	{
	PDMaxIN_20V,
	PDMaxIN_28V
	}MaximumPDVoltageDef;	
	
typedef enum
	{
	Balance_Diasbled, //���ùر���������
	Balance_ChgDisOnly, //����ŵ�ʱ����
	Balance_ChgOnly, //���ڳ��ʱ����
	Balance_AlwaysEnabled, //������Զ����
	}BalanceModeDef;	
	
//ϵͳ����
typedef struct
	{
	IP2366InputDef InputConfig;
	IP2366OutConfigDef OutputConfig;
	VBatLowDef Vlow;
	PDOBroadcastDef PDOCFG;
	//��ȫ����
	bool EnableHPGauge; //�����߾���GTypeC�������
  bool EnableAdvAccess; //���������߲˵���һ������		
	bool EnableChargeConfig; //�����������
  bool EnableDischargeConfig; //�����ŵ�����
  bool EnableChargPowerConfig; //������繦������
  bool EnablePDOConfig; //����PDO���õ�����		
	bool EnableLVProtectConfig; //������ѹ��������
	bool EnableOTPConfig; //�򿪹��ȱ�������
	bool EnableThermalStepdown; //�������Ⱥ��Զ������ʵĻ���
	//���ȱ�������
	int OverHeatLockTemp; //���ȱ���ʱ��
	//GUI����ʾ����
	bool EnableLargeMenu; //���ô�˵�
	bool EnableFastBoot; //���ÿ�������
	LCDDisplayDirDef DisplayDir;
	//������������	
	InstantCTestDef InstantCTest;
	//�ٳ���������
	ReChargeConfig VRecharge;
	IStopConfig IStop;
  //���PD�����������
	MaximumPDVoltageDef MaxVPD;
  //����ϵͳģʽ����
  BalanceModeDef BalanceMode;
	}SystemCfgDef;

typedef union
	{
	SystemCfgDef Data;
	char ByteBuf[sizeof(SystemCfgDef)];
	}CfgDataStorDef;	
	
typedef struct
	{	
	unsigned int CRCResult;
	CfgDataStorDef Data;
	}CfgROMImageDef;	

typedef union
	{
	CfgROMImageDef ROMImage;
	char ByteBuf[sizeof(CfgROMImageDef)];
	}CfgUnionDef;

//�ⲿ�ο�
extern CfgUnionDef CfgUnion;
#define CfgFileSize sizeof(CfgUnion)	
#define CfgData CfgUnion.ROMImage.Data.Data
#define CfgChecksum CfgUnion.ROMImage.CRCResult

//����
bool CheckIfConfigIsSame(void); //������ͬ
bool ReadConfiguration(CfgUnionDef *Out);
bool WriteConfiguration(CfgUnionDef *IN,bool ForceUpdate); //��д����
void RestoreDefaultConfig(void); //���ó���
void LoadConfig(void); //����ʱ��ȡ����
void LoadDefaultConfig(CfgUnionDef *IN); //����Ĭ�����õ�ĳ���ط�
unsigned int CalcROMCRC32(CfgUnionDef *IN); //����CRC32

#endif
