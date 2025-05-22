#ifndef _LVProt_
#define _LvProt_

#include "stdbool.h"

//��������
#define BatteryMaximumTurboVdroop 1.2  //�������������У�����������ĺ�����ǰ��ѹ��(V)
#define BatteryAlertDelay 10 //��ؾ����ӳ�	
#define BatteryFaultDelay 2 //��ع���ǿ������/�ػ����ӳ�
#define TurboILIMTryCDTime 4 //ÿ�μ��������µ���������ȴʱ�䣨��λ��1/8�룩

//�ⲿ����
extern xdata int TurboILIM; //������������
extern xdata float BeforeRawBattVolt; //����ǰ��ѹ�Ĳ���

//����
void BatteryLowAlertProcess(bool IsNeedToShutOff,ModeIdxDef ModeJump); //��ͨ��λ�ľ�������
void RampLowVoltHandler(void); //�޼������ר������
void BattAlertTIMHandler(void); //��ص͵�������������
void TurboLVILIMProcess(void); //����ר���ĵ�������ֵ�Ĺ���
void RampRestoreLVProtToMax(void); //ÿ�ο��������޼�ģʽʱ���Իָ�����
void CalcTurboILIM(void); //���㼫��������λ������ֵ

#endif
