#ifndef _ModeControl_
#define _ModeControl_

#include "stdbool.h"
//��λLED����
typedef enum
	{
	Locator_OFF=0, //��λ�ر�
	Locator_Green=1, //�̵�
	Locator_Red=2, //���
	Locator_Amber=3, //�Ƶ�
	}LocatorLEDDef;	

typedef struct
	{
	int RampCurrent;
	int RampBattThres;
	int RampCurrentLimit;
	char RampLimitReachDisplayTIM;
	char CfgSavedTIM;
	LocatorLEDDef LocatorCfg;
	}SysConfigDef;	
	
typedef enum
	{
	Mode_OFF=0, //�ػ�
	Mode_Fault, //���ִ���
		
	Mode_Ramp, //�޼�����
	Mode_1Lumen, //1�������͵�λ
  Mode_Moon, //�¹�
	Mode_ExtremelyLow, //������
	Mode_Low, //����
	Mode_Mid, //����
	Mode_MHigh,   //�и���
	Mode_High,   //����
		
	Mode_Turbo, //����
	//����ģʽ
  Mode_Beacon, //�ű굲λ 		
  Mode_Strobe, //����		
	Mode_SOS, //SOS��λ
	}ModeIdxDef;
	

typedef struct
	{
  ModeIdxDef ModeIdx;
  int Current; //��λ����(mA)
	int MinCurrent; //��С����(mA)�����޼�������Ҫ
	int LowVoltThres; //�͵�ѹ����ѹ(mV)
	bool IsModeHasMemory; //�Ƿ������
	bool IsNeedStepDown; //�Ƿ���Ҫ����
	}ModeStrDef; 

//�ⲿ����
extern xdata char DisplayLockedTIM; //������ʾ��ʱ��
extern ModeStrDef *CurrentMode; //��ǰģʽ�ṹ��
extern xdata ModeIdxDef LastMode; //��һ����λ	
extern SysConfigDef SysCfg; //�޼���������	
extern bit IsRampEnabled; //�Ƿ������޼�����	
	
//����궨��
#define QueryCurrentGearILED() CurrentMode->Current //��ȡ��ǰ��λ�ĵ�������
	
//��������
#define HoldSwitchDelay 6 // ���������ӳ�	
#define SleepTimeOut 5 //����״̬��ʱ	
#define ModeTotalDepth 14 //ϵͳһ���м�����λ			
	
//����
void ModeFSMTIMHandler(void);//��λ״̬������������ʱ������
void ModeSwitchFSM();//��λ״̬��	
void SwitchToGear(ModeIdxDef TargetMode);//����ָ����λ
void ReturnToOFFState(void);//�ػ�	
void HoldSwitchGearCmdHandler(void); //�����������	
void ModeFSMInit(void); //��ʼ��״̬��	

#endif
