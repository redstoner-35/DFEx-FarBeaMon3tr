#ifndef _BALMGMT_
#define _BALMGMT_

#include <stdbool.h>

//�ⲿ�ο�
extern int BalanceForceEnableTIM; //ǿ�����þ���ϵͳ�ı��������ñ���д����0��ֵ���þ�����
extern bool EnableExtendedBal; //�ֶ�����������־λ
extern bool BalanceState; //��������ǰ״̬

//����
void BalanceMgmt_Init(void); //���þ��������
void Balance_ForceDiasble(void); //ǿ�ƹرվ���
void Balance_IOMgmt(void); //���й����п��ƾ��������õ�ģ��

#endif
