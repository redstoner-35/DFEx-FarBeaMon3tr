#ifndef _LVProt_
#define _LvProt_

#include "stdbool.h"

//��������
#define BatteryAlertDelay 10 //��ؾ����ӳ�	
#define BatteryFaultDelay 2 //��ع���ǿ������/�ػ����ӳ�

//�ⲿ����
extern xdata int TurboILIM; //������������

//����
void BatteryLowAlertProcess(bool IsNeedToShutOff,ModeIdxDef ModeJump); //��ͨ��λ�ľ�������
void RampLowVoltHandler(void); //�޼������ר������
void BattAlertTIMHandler(void); //��ص͵�������������
void TurboLVILIMProcess(void); //����ר���ĵ�������ֵ�Ĺ���
void RampRestoreLVProtToMax(void); //ÿ�ο��������޼�ģʽʱ���Իָ�����
void CalcTurboILIM(void); //���㼫��������λ������ֵ

#endif
