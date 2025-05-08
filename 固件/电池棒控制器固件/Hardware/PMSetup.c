#include "ht32.h"
#include "delay.h"
#include "Pindefs.h"
#include "GUI.h"
#include "BalanceMgmt.h"

//��Դ���������Զ�����
#define LDO_EN_IOB STRCAT2(GPIO_P,LDO_EN_IOBank)
#define LDO_EN_IOG STRCAT2(HT_GPIO,LDO_EN_IOBank)
#define LDO_EN_IOP STRCAT2(GPIO_PIN_,LDO_EN_IOPinNum) 

//int�����Զ�����
#define IP2366_INT_IOB STRCAT2(GPIO_P,IP2366_INT_IOBank)
#define IP2366_INT_IOG STRCAT2(HT_GPIO,IP2366_INT_IOBank)
#define IP2366_INT_IOP STRCAT2(GPIO_PIN_,IP2366_INT_IOPinNum) 

//EN�����Զ�����
#define IP2366_EN_IOB STRCAT2(GPIO_P,IP2366_EN_IOBank)
#define IP2366_EN_IOG STRCAT2(HT_GPIO,IP2366_EN_IOBank)
#define IP2366_EN_IOP STRCAT2(GPIO_PIN_,IP2366_EN_IOPinNum) 

//�ڲ�����
short SleepTimer; //˯�߶�ʱ 
short IPStallTime=0; //IP2368����

//��ʼ��IO����Ծٲ���
void PowerMgmtSetup(void)
  {
	 //����GPIO
   AFIO_GPxConfig(LDO_EN_IOB,LDO_EN_IOP, AFIO_FUN_GPIO);
   GPIO_DirectionConfig(LDO_EN_IOG,LDO_EN_IOP,GPIO_DIR_OUT);//����Ϊ��� 
	 GPIO_DriveConfig(LDO_EN_IOG,LDO_EN_IOP,GPIO_DV_16MA);	//����Ϊ16mA������	
	 GPIO_ClearOutBits(LDO_EN_IOG,LDO_EN_IOP);//Ĭ���������Ϊ0
	 
	 AFIO_GPxConfig(IP2366_INT_IOB,IP2366_INT_IOP, AFIO_FUN_GPIO);
	 GPIO_DirectionConfig(IP2366_INT_IOG,IP2366_INT_IOP,GPIO_DIR_IN);//����Ϊ��������
	 GPIO_InputConfig(IP2366_INT_IOG,IP2366_INT_IOP,ENABLE);  //������Ĵ���
	 GPIO_PullResistorConfig(IP2366_INT_IOG,IP2366_INT_IOP,GPIO_PR_DOWN); //������
		
   AFIO_GPxConfig(IP2366_EN_IOB,IP2366_EN_IOP, AFIO_FUN_GPIO);
   GPIO_DirectionConfig(IP2366_EN_IOG,IP2366_EN_IOP,GPIO_DIR_OUT);//����Ϊ��� 
	 GPIO_DriveConfig(IP2366_EN_IOG,IP2366_EN_IOP,GPIO_DV_16MA);	//����Ϊ16mA������			
	 //�������
   GPIO_ClearOutBits(IP2366_EN_IOG,IP2366_EN_IOP); //2366-EN=0
	 delay_ms(100);
	 GPIO_SetOutBits(LDO_EN_IOG,LDO_EN_IOP);//�������Ϊ1
	 //����������ʱ��
	 SleepTimer=480; //һ�����޲����Զ�����
	}
	
//Type-C����ʧ��ʱ�������������ֵĲ���
void IP2366StallRestore(void)
  {
	int wait;
	//��ʱ�ۼӲ���
	IPStallTime++;
	if(IPStallTime>=32) //IP2366���ߴ�Լ4���ʼ����
	  {
		IPStallTime=0;  //������λ���ȴ��´μ�ʱ
		//����Ϊ��������ʹ2366����
		GPIO_InputConfig(IP2366_INT_IOG,IP2366_INT_IOP,ENABLE); 
	  GPIO_DirectionConfig(IP2366_INT_IOG,IP2366_INT_IOP,GPIO_DIR_IN);
		//��EN��������
		delay_ms(200);	
		GPIO_SetOutBits(IP2366_EN_IOG,IP2366_EN_IOP); 
	  delay_ms(200);
    GPIO_ClearOutBits(IP2366_EN_IOG,IP2366_EN_IOP);	//��2366-EN=1�����Լ��		
		//����Ƿ�ɹ�����
		wait=300; //���Ѽ����ʱ300mS
		do
		  {
			wait--;
      if(wait<=0)break;	//200mS��IP2368����ʧ�ܣ��˳�
			delay_ms(1);		
			}
		while(GPIO_ReadInBit(IP2366_INT_IOG,IP2366_INT_IOP)==SET); //IP2366λ�ڻ���״̬���ȴ����ѽ���
		//����ɹ�������100mS����סINTʹ2366���ֻ���
		if(wait>0)delay_ms(100);
		GPIO_InputConfig(IP2366_INT_IOG,IP2366_INT_IOP,DISABLE); 
	  GPIO_DirectionConfig(IP2366_INT_IOG,IP2366_INT_IOP,GPIO_DIR_OUT);//����IDR������Ϊ���
	  GPIO_SetOutBits(IP2366_INT_IOG,IP2366_INT_IOP); //��INT������1,ʹIP2366��Զ���Ѳ���˯��	
    }
	}		
	
//ǿ�ƹػ�
void ShutSysOFF(void)
	{
	extern bool IsEnablePowerOFF;
	if(!IsEnablePowerOFF)return; //������ػ�
	ClearScreen();
	LCD_DeInit(); //����LCD
	Balance_ForceDiasble(); //��������رվ���ϵͳ
	GPIO_ClearOutBits(LDO_EN_IOG,LDO_EN_IOP);//�������Ϊ0,�ر�LDO��Դǿ�ȵ�Ƭ������
	delay_ms(300);
	NVIC_SystemReset();
	while(1);		
	}	
	
//��2366��һ�Ű�2366����
void KickIP2366ToWakeUp(void)
	{
	int retry=3;
	char WakeMsg[]={"�������Դ���:5"};
	ShowPostInfo(12,"���ѳ��IC","05",Msg_Statu);
	//���INT=0˵��2366��˯��
	if(GPIO_ReadInBit(IP2366_INT_IOG,IP2366_INT_IOP)==RESET)do
		{
		GPIO_SetOutBits(IP2366_EN_IOG,IP2366_EN_IOP); 
	  delay_ms(250);
    GPIO_ClearOutBits(IP2366_EN_IOG,IP2366_EN_IOP);	//��2366-EN=1�����Լ��
		delay_ms(200);	
		if(GPIO_ReadInBit(IP2366_INT_IOG,IP2366_INT_IOP)==SET)break; //���ѳɹ�
		//����ʧ�ܣ���ʾ����ʣ�����
		retry--;
		WakeMsg[13]=0x30+(3-retry);
		ShowPostInfo(12,WakeMsg,"W0",Msg_Warning);		
    //��ʱ2�������			
    delay_Second(2);	
		}	
	while(retry); 
	//����ʧ��
	if(!retry)
		{
		ShowPostInfo(12,"���IC����ʧ��","E2",Msg_Fault);
		SelfTestErrorHandler();
		}
	GPIO_SetOutBits(IP2366_INT_IOG,IP2366_INT_IOP);
	GPIO_DirectionConfig(IP2366_INT_IOG,IP2366_INT_IOP,GPIO_DIR_OUT);
	GPIO_InputConfig(IP2366_INT_IOG,IP2366_INT_IOP,DISABLE);  //����Ϊ�����������2366����
	}
	
//����״̬�ж�
void PowermanagementSleepControl(void)
  {
	extern bool IsEnablePowerOFF;
	extern int BalanceForceEnableTIM;
	//��ǰδ����˯��״̬�����⿪������Type-C���������У���ִ��
	if(!IsEnablePowerOFF||BalanceForceEnableTIM)
		{
		SleepTimer=480;	
		return; //��λ��ʱ��
		}
	if(SleepTimer<8)Balance_ForceDiasble(); //ǿ�ƹرվ�����	
	//ʱ��δ��������ʱ
	if(SleepTimer>0)return;
	//��ʼ���
	GPIO_InputConfig(IP2366_INT_IOG,IP2366_INT_IOP,ENABLE); 
	GPIO_DirectionConfig(IP2366_INT_IOG,IP2366_INT_IOP,GPIO_DIR_IN);//����Ϊ��������ʹ2366����
	if(GPIO_ReadInBit(IP2366_INT_IOG,IP2366_INT_IOP)==SET)
		{
		SleepTimer=10;
		return; //���IP2366δ����˯��������ȴ�
		}
	//�����ͷ�IO
	ClearScreen();
	GPIO_ClearOutBits(LDO_EN_IOG,LDO_EN_IOP);//�������Ϊ0,�ر�LDO��Դǿ�ȵ�Ƭ������
	while(1);		
	}
	
	
