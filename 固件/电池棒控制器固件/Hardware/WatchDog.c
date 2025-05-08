#include "ht32.h"
#include "WatchDog.h"

//�������Ź�ģ��
void WatchDog_Init(void)
	{
	//��λWDT
  WDT_DeInit();
	//����ʱ��Դ�ͷ�Ƶ��	
	RTC_LSILoadTrimData();
  WDT_SourceConfig(WDT_SOURCE_LSI); //ʹ��LSI��Ϊʱ��
	WDT_SetPrescaler(WDT_PRESCALER_64);	 //���÷�ƵֵΪ 32K/64 = 500 Hz
	//��������ֵ	
	WDT_SetReloadValue(500*WDTReloadSec); //����WDT���ؼ�����ֵ��2��������
	WDT_SetDeltaValue((500*WDTReloadSec)+10);	 //������⹦�ܹرգ�����˳�������ģ��ʱϵͳ������bug
	WDT_Restart(); //�Լ�������������
  //������λָ��Ϳ��Ź�����
	WDT_ResetCmd(ENABLE);
  WDT_Cmd(ENABLE);          
  //�������Ź����ñ������ñ��������		
  WDT_ProtectCmd(ENABLE);           
	}
