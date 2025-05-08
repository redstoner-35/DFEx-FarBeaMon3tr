#ifndef _2366REG_
#define _2366REG_

#include <stdbool.h>

typedef enum
	{
	//�ɶ�д�Ĵ���
	REG_SYSCTL0=0,
	REG_SYSCTL2=2,
	REG_SYSCTL3=3,
	REG_SYSCTL6=6,
	REG_SYSCTL8=8,
	REG_SYSCTL9=9,
	REG_SYSCTL10=0x0A,
	REG_SYSCTL11=0x0B,
	REG_SYSCTL12=0x0C,
	REG_SELECT_PDO=0x0D,
	REG_TYPEC_CTL8=0x22,
	REG_TYPEC_CTL9=0x23,
	REG_TYPEC_CTL10=0x24,
	REG_TYPEC_CTL11=0x25,
	REG_TYPEC_CTL12=0x26,
	REG_TYPEC_CTL13=0x27,
	REG_TYPEC_CTL14=0x28,	
	REG_TYPEC_CTL17=0x2B,
	REG_TYPEC_CTL18=0x2C,
	REG_TYPEC_CTL23=0x29,
	REG_TYPEC_CTL24=0x2A,
  //ֻ���Ĵ���
	REG_STATE_CTL0=0x31,
  REG_STATE_CTL1=0x32,
	REG_STATE_CTL2=0x33,
	REG_STATE_CTL3=0x38,
  REG_TYPEC_STATE=0x34,
	REG_RECEIVED_PDO=0x35,
	REG_VBAT_LSB=0x50,
	REG_VBAT_MSB=0x51,
  REG_VSYS_LSB=0X52,
	REG_VSYS_MSB=0X53,
	REG_IBAT_LSB=0X6E,
	REG_IBAT_MSB=0X6F,
	REG_ISYS_LSB=0X70,
	REG_ISYS_MSB=0X71,
	REG_PSYS_LSB=0x74,
	REG_PSYS_MSB=0x75
	}IP2366REGDef;

typedef enum
	{
	Power_30W,
	Power_45W,
	Power_60W,
	Power_65W,
	Power_100W,
	Power_140W
	}ChargePowerDef;

typedef enum
	{
	IStop_50mA=1,
	IStop_100mA=2,
	IStop_150mA=3,
	IStop_200mA=4,
	IStop_250mA=5,
	IStop_300mA=6,
	IStop_350mA=7,
	IStop_400mA=8,
	IStop_450mA=9,
	IStop_500mA=10
	}IStopConfig;	 //ͣ���������
	
typedef enum
	{
	Recharge_Disable=0, //�ر��ٳ��
	Recharge_0V05=1, //��ص�ѹ���ڸ���0.05Vʱ�ٳ��
	Recharge_0V1=2, //��ص�ѹ���ڸ���0.1Vʱ�ٳ��
	Recharge_0V2=3 //��ص�ѹ���ڸ���0.2Vʱ�ٳ��
	}ReChargeConfig; //�ٳ������
	
	
typedef enum
 {
 QuickCharge_PD,
 QuickCharge_HV, //�����ѹ���
 QuickCharge_HC, //�͵�ѹ�ߵ������
 QuickCharge_None, //�޿��
 QuickCharge_PlaceHolder, //QCö�������ռλ������ʵû��ֻ��������λGUI
 }QuickChargeStateDef;	
 
typedef enum
 {
 Batt_StandBy,
 Batt_PreChage,
 Batt_CCCharge,
 Batt_CVCharge,
 Batt_ChgWait,
 Batt_ChgDone,
 Batt_ChgError,
 Batt_discharging,
 }BatteryStateDef;	
 
typedef enum
	{
	RecvPDO_None, //û��PDO
	RecvPDO_5V, //�յ�5V
	RecvPDO_9V, //9V
	RecvPDO_12V, //12V
	RecvPDO_15V, //15V
	RecvPDO_20V, //20V
	}RecvPDODef;	
 
typedef struct
	{
	bool EnablePPS2;
	bool EnablePPS1;
	bool Enable20V;
	bool Enable15V;
	bool Enable12V;
	bool Enable9V;
	}PDOBroadcastDef;	
	
typedef enum
 {
 PD_5VMode=1,
 PD_7VMode=2,
 PD_9VMode=3,
 PD_12VMode=4,
 PD_15VMode=5,
 PD_20VMode=6,
 PD_28VMode=7
 }PDStateDef;	
	
typedef struct
	{
	bool IsEnableCharger; //�Ƿ������
	int ChargeCurrent; //��������������(mA)
	int PreChargeCurrent; //�������������(mA)
	int FullVoltage; //��������ѹ
	ChargePowerDef ChargePower; //����������
	}IP2366InputDef;	
	
typedef enum
	{
	VSys_State_Normal,
	VSys_State_OCP,
	VSys_State_Short
	}VSysStateDef;	
	
typedef enum
	{
	VBUS_NoPower,
	VBUS_Normal,
	VBUS_OverVolt,
	}VbusVoltStateDef;	
	
typedef struct
	{
	VbusVoltStateDef VBusState;
	VSysStateDef VSysState;
	}ChipStatDef;	
	
typedef enum
	{
	TypeC_Disconnect=0,
	TypeC_UFP=2,
	TypeC_DFP=1,
	TypeC_DRP=3
	}TypeCRoleDef;	

typedef enum
	{
	VLow_2V5,
	VLow_2V6,
	VLow_2V7,
	VLow_2V8,
	VLow_2V9,
	VLow_3V0,
	VLow_3V1,
	VLow_3V2,
	}VBatLowDef;
	
typedef struct
	{
	float VBUSVolt;
	float VBUSCurrent;
  QuickChargeStateDef QuickChargeState;
	PDStateDef PDState;	 
	bool IsTypeCConnected;
	}IP2366VBUSStateDef;	
	
typedef struct
	{
	bool IsEnableOutput;  //���������������
	bool IsEnablePDOut; //�Ƿ���PD���
	bool IsEnableDPDMOut; //�Ƿ�����ѹ���Э��
	bool IsEnableSCPOut; //�Ƿ���SCPģʽ
	bool IsEnableHSCPOut; //������ѹSCP
	}IP2366OutConfigDef;

//�����ض���������
#define BusCurrentCalFactor 0.985	//IP2366�������ĵ�������ֵ
	
//��ֵ������������
#if (BATTCOUNT == 4)	

#define IP2366_ICCMAX 13000 //�Ĵ��汾��13000mA
	
#elif (BATTCOUNT == 3 || BATTCOUNT == 2)
	
#define IP2366_ICCMAX 16000 //2-3���汾��16000mA
	
#else
	
#error "Invaild Battery Count Settings!"	
	
#endif
	
//������ֵ���㺯��
bool IP2366_QueryCurrentStateIsACC(BatteryStateDef IN); //��ѯ���״̬�Ƿ���Ҫ���ؼ�ͳ��	
	
//����
bool IP2366_GetVRecharge(float *Vrecharge); //��ȡ�ٳ���ѹ
bool IP2366_getCurrentChargeParam(int *Istop,float *Vstop); //��ȡ��ǰоƬ�ĳ�����(�����ѹ��ͣ�������)
bool IP2366_GetCurrentPeakCurrent(int *Result); //��ȡ��ֵ����
bool IP2366_SetOTPSign(void); //�������ؼ����
bool IP2366_DetectIfChipReset(bool *IsReset); //���оƬ�Ƿ�λ
bool IP2366_SetVLowVolt(VBatLowDef Vlow); //���õ�ѹ����
bool IP2366_GetFirmwareTimeStamp(char TimeStamp[5]);	//��ȡʱ���
bool IP2366_SetInputState(IP2366InputDef * Cfg); //��������״̬
bool IP2366_SetOutputState(IP2366OutConfigDef * CFG); //�������״̬	
bool IP2366_GetRecvPDO(RecvPDODef *PDOResult);	//��ȡ�����PDO״̬
bool IP2366_GetVBUSState(IP2366VBUSStateDef * State); //��ȡVBUS״̬
bool IP2366_GetChargerState(BatteryStateDef *State);	//��ȡ���״̬
bool IP2366_DetectIfPresent(void);	//���IP2366�Ƿ����
bool IP2366_SetTypeCRole(TypeCRoleDef Role); //����TypeC��ģʽ
bool IP2366_GetIfInputConnected(void);	//��ȡIP2366�Ƿ�Ϊ��������
void IP2366_SetICCMax(int TargetCurrent); //��������iset�Ĵ���ֱ������һ��Ϊֹ
bool IP2366_ReadChipState(ChipStatDef *State);	//��ȡһ����оƬ����Ϣ
bool IP2366_SetPDOBroadCast(PDOBroadcastDef *PDOCfg);	//����PDO�㲥
bool IP2366_EnableDCDC(bool IsEnableCharger,bool IsEnableDischarge); //���ó�����ͷŵ�	
bool IP2366_SetReChargeParam(ReChargeConfig Vrecharge,IStopConfig IStop); //�����ٳ�����	
bool IP2366_UpdataChargePower(ChargePowerDef Power);	//���³�繦��
bool IP2366_UpdateFullVoltage(int Volt); //���³���ѹ(����ĵ�λΪmV)
void IP2366_ClearOCFlag(void); //���OC Flag

#endif
