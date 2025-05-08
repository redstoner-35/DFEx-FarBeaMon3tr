#include "cms8s6990.h"
#include "delay.h"
#include "SideKey.h"
#include "PWMCfg.h"
#include "PinDefs.h"
#include "ModeControl.h"
#include "OutputChannel.h"
#include "SpecialMode.h"
#include "BattDisplay.h"
#include "ADCCfg.h"
#include "LEDMgmt.h"
#include "LocateLED.h"

//˯�߶�ʱ��
volatile int SleepTimer;

//����/��������ϵͳ����
void SystemPeripheralCTRL(bit IsEnable)
	{
	if(IsEnable)
		{
		ADC_Init(); //��ʼ��ADC
		PWM_Init(); //��ʼ��PWM������
		LED_Init(); //��ʼ���ఴLED
		OutputChannel_Init(); //��ʼ�����ͨ��
		return;
		}
	//�ر���������
	SetSystemHBTimer(0); //����������ʱ��
	PWM_DeInit();
	ADC_DeInit(); //�ر�PWM��ADC
	LocateLED_Enable(); //�򿪶�λLED
	OutputChannel_DeInit(); //�����ͨ�����и�λ
	}
	
//���ض�ʱ��ʱ��
void LoadSleepTimer(void)	
	{
	//����˯��ʱ��
	SleepTimer=SysMode>Operation_Locked?4800:8*SleepTimeOut; //˯��ʱ���ӳ�		
	}

//���ϵͳ�Ƿ��������˯�ߵ�����
static char QueryIsSystemNotAllowToSleep(void)
	{
	//ϵͳ���ڶ�λָʾ��ѡ��״̬��������˯��
	if(LocLEDState!=LocateLED_NotEdit)return 1;
	//ϵͳ����ʾ��ص�ѹ������˯��
	if(VshowFSMState!=BattVdis_Waiting)return 1;
	//ϵͳ������
	if(CurrentMode->ModeIdx!=Mode_OFF)return 1;
	//����˯��
	return 0;
	}	
	
//˯�߹�����
void SleepMgmt(void)
	{
	bit sleepsel;
	//�ǹػ�����Ȼ����ʾ��ص�ѹ��ʱ��ʱ����λ��ֹ˯��
	if(QueryIsSystemNotAllowToSleep())LoadSleepTimer();
	//����˯�߿�ʼ����ʱ
	if(SleepTimer>0)SleepTimer--;
	//��������˯�߽׶�
	else
		{		
		if(SysMode>Operation_Locked)SysMode=Operation_Normal; //ǿ���˳�ս��ģʽ
		C0CON0=0; //�ఴ�ػ���رձȽ���
		SystemPeripheralCTRL(0);//�ر���������
		STOP();  //��STOP=1��ʹ��Ƭ������˯��
		//ϵͳ�ѻ��ѣ�������ʼ���
		delay_init();	 //��ʱ������ʼ��
		SetSystemHBTimer(1); 
		MarkAsKeyPressed(); //������ǰ�������
		do	
			{
			delay_ms(1);
			SideKey_LogicHandler(); //����ఴ����
			//�ఴ�����ļ�ⶨʱ������(ʹ��62.5mS����ʱ��,ͨ��2��Ƶ)
			if(!SysHFBitFlag)continue; 
			SysHFBitFlag=0;
			sleepsel=~sleepsel;
			if(sleepsel)SideKey_TIM_Callback();
			}
		while(!IsKeyEventOccurred()); //�ȴ���������
		//ϵͳ�ѱ����ѣ��������빤��ģʽ			
		SystemPeripheralCTRL(1);
		//���������ʼ����ϣ�����ADC�첽����ģʽ����ϵͳ�ж�
		EnableADCAsync(); 
		}
	}
