#include "cms8s6990.h"
#include "GPIO.h"
#include "delay.h"
#include "SideKey.h"
#include "LEDMgmt.h"
#include "ADCCfg.h"
#include "OutputChannel.h"
#include "PWMCfg.h"
#include "BattDisplay.h"
#include "ModeControl.h"
#include "TempControl.h"
#include "SOS.h"
#include "SelfTest.h"
#include "LowVoltProt.h"
#include "Beacon.h"
#include "LocateLED.h"
#include "Strobe.h"

//��������
void SleepMgmt(void);

//������
void main()
	{
	bit TaskSel=0;
	//ʱ�ӳ�ʼ��	
 	delay_init();	 //��ʱ������ʼ��
	SetSystemHBTimer(1);//����ϵͳ����8Hz��ʱ��	
	//��ʼ������
	OutputChannel_Init(); //�������ͨ��	
	ADC_Init(); //��ʼ��ADC
	PWM_Init(); //����PWM��ʱ��
	LED_Init(); //��ʼ���ఴLED
	ModeFSMInit(); //��ʼ����λ״̬��
  SideKeyInit(); //�ఴ��ʼ��	
	OutputChannel_TestRun(); //�������ͨ��������
	DisplayVBattAtStart(); //��ʾ�����ѹ
	EnableADCAsync(); //����ADC���첽ģʽ��ߴ����ٶ�
	//��ѭ��	
  while(1)
		{
	  //ʵʱ����
		SystemTelemHandler();//��ȡ�����Ϣ	
		SideKey_LogicHandler(); //����ఴ����
		BatteryTelemHandler(); //������ң��
		ModeSwitchFSM(); //��λ״̬��
		ThermalMgmtProcess(); //�¶ȹ��������ȱ�����
		OutputChannel_Calc(); //���ݵ����������ͨ������
		PWM_OutputCtrlHandler(); //����PWM�������	
		//8Hz��ʱ����
		if(!SysHFBitFlag)continue; //ʱ��û������������
		SysHFBitFlag=0;	
		//Task0������������Ƚϴ������
    if(!TaskSel)
			{
			RandStrobeHandler(); //����α�����������
			ThermalPILoopCalc(); //����������
			ModeFSMTIMHandler(); //ģʽ״̬����ʱ
			BeaconFSM_TIMHandler();	//�ű�ģʽ��ʱ��	
			LEDControlHandler();//�ఴָʾLED���ƺ���
			OCFSM_TIMHandler(); //���ͨ��״̬����ʱ
			HoldSwitchGearCmdHandler(); //��������
			SideKey_TIM_Callback();//�ఴ�����ļ�ⶨʱ������
			//���������������ѡ����з�ת������һ��
			TaskSel=1;
		  }			
		//Task1������������Ƚ�С�ļ�ʱ����
		else
			{			
			OutputFaultDetect();//������ϼ��
			BattDisplayTIM(); //��ص�����ʾTIM
			SOSTIMHandler(); //SOS��ʱ��
			BattAlertTIMHandler(); //��ؾ�����ʱ����
			DisplayErrorTIMHandler(); //���ϴ�����ʾ
			SleepMgmt(); //���߹���
			LocateLED_TIMHandler(); //��λLED��ʱ
			//���������������ѡ����з�ת������һ��
			TaskSel=0;
			}
		}
	}
