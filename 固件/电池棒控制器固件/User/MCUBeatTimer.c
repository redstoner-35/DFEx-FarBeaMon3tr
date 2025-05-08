#include "HT32.h"
#include "delay.h"
#include "GUI.h"

//����ϵͳ�ڵ�0.125��������ʱ��
void EnableHBTimer(void)
 {
 CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};  
 TM_TimeBaseInitTypeDef TimeBaseInit;
 ShowPostInfo(3,"Starting HBTIM","01",Msg_Statu);
 //�������ö�ʱ�����ڲ���0.125��Ķ�ʱ�ж�
 CKCUClock.Bit.GPTM0 = 1;
 CKCU_PeripClockConfig(CKCUClock, ENABLE);
 TimeBaseInit.Prescaler = 479;                         // 48MHz->100KHz
 TimeBaseInit.CounterReload = 12499;                   // 100KHz->8Hz
 TimeBaseInit.RepetitionCounter = 0;
 TimeBaseInit.CounterMode = TM_CNT_MODE_UP;
 TimeBaseInit.PSCReloadTime = TM_PSC_RLD_IMMEDIATE;
 TM_TimeBaseInit(HT_GPTM0, &TimeBaseInit);
 TM_ClearFlag(HT_GPTM0, TM_FLAG_UEV);
 //���ú��ж�Ȼ���ö�ʱ����������
 NVIC_EnableIRQ(GPTM0_IRQn);
 TM_IntConfig(HT_GPTM0,TM_INT_UEV,ENABLE);
 TM_Cmd(HT_GPTM0, ENABLE);
 }

void CheckIfHBTIMStart(void)
{
int retry=300;
extern bool SensorRefreshFlag;
ShowPostInfo(40,"����������ʱ��\0","7A",Msg_Statu);
//����Ƿ���WDT Reset
if(RSTCU_GetResetFlagStatus(RSTCU_FLAG_WDTRST))
	{
	ShowPostInfo(40,"��⵽ϵͳ����\0","7B",Msg_Warning);
	delay_Second(2);
	}
//��ʼ���Զ�ʱ��
SensorRefreshFlag=0;
do
	{
	delay_ms(1);
	if(SensorRefreshFlag)return; //�ɹ���ɼ��
	retry--;
	}
while(retry>0);
//��ⲻͨ��
ShowPostInfo(40,"������ʱ���쳣\0","7E",Msg_Fault);	
SelfTestErrorHandler();	
}	
 
//�ر�ϵͳ�ڵ�0.125��������ʱ��
void DisableHBTimer(void)
 {
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};  
 //���ö�ʱ�����ر��ж�
 TM_Cmd(HT_GPTM0, DISABLE);
 TM_ClearFlag(HT_GPTM0, TM_FLAG_UEV);
 NVIC_DisableIRQ(GPTM0_IRQn);
 //�رն�ʱ��ʱ��
 CKCUClock.Bit.GPTM0 = 1;
 CKCU_PeripClockConfig(CKCUClock, DISABLE);	 
 }	
