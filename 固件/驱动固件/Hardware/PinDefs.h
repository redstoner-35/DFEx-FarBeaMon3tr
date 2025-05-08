#ifndef PINDEFS
#define PINDEFS

//�ڲ�����
#include "GPIOCfg.h"

/************************************************************************************
�������������������õ���IO���ŵ�����λ���������Ѿ��������Ź������ͽ����˷��飬���
���ƻ����ù̼��������������������Ը���Ӳ���������Ŷ��塣
************************************************************************************/



/******************** ��������DCDC�͸���DCDC�Լ�������Ƶ����� *********************/
#define AUXENIOP GPIO_PORT_2
#define AUXENIOG 2
#define AUXENIOx GPIO_PIN_6	//����6.0Vʹ�����(P2.6)


#define BOOSTRUNIOP GPIO_PORT_3
#define BOOSTRUNIOG 3
#define BOOSTRUNIOx GPIO_PIN_2	//��Boost����ʹ�����(P3.2)


#define LEDNegMOSIOP GPIO_PORT_2
#define LEDNegMOSIOG 2
#define LEDNegMOSIOx GPIO_PIN_5		//LED����MOSFET(P2.5)


/******************* �����Ǹ���CC/CV������ѹ���ɵ�PWMDAC������ *********************/
#define PWMDACENIOP GPIO_PORT_2
#define PWMDACENIOG 2
#define PWMDACENIOx GPIO_PIN_3 	//����(CC)����PWMDACʹ��(P2.3)


#define PWMDACIOG 2
#define PWMDACIOx GPIO_PIN_2		//����(CC)����PWMDAC���(P2.2)


#define PreChargeDACIOG 0
#define PreChargeDACIOx GPIO_PIN_2		//��ѹԤ��(CV)����PWMDAC���(P0.2)


/************************************************************************************
������ϵͳ��ģ���������ţ�������ص�ѹ�����������ѹ�������˷ź���״̬�����������¶�
������NTC�����Լ�NTC����
************************************************************************************/
#define NTCInputIOG 0
#define NTCInputIOx GPIO_PIN_5 
#define NTCInputAIN 5						//NTC����(P0.5,AN5)


#define VOUTFBIOG 3
#define VOUTFBIOx GPIO_PIN_1
#define VOUTFBAIN 13						//�����ѹ��������(P3.1,AN13)


#define OPFBIOG 0
#define OPFBIOx GPIO_PIN_1
#define OPFBAIN 1								//�˷ź���״̬��������(P0.1,AN1)


#define VBATInputIOG 3
#define VBATInputIOx GPIO_PIN_0
#define VBATInputAIN 22					//��ص�ѹ�������(P3.0,AN22)


#define NTCENIOG 0
#define NTCENIOx GPIO_PIN_3			//NTC���ʹ������(P0.3)


/****************** �����Ǹ��𰴼�С�岿�ֵ�����(ָʾ�ƺͰ���) ********************/
#define SideKeyGPIOP GPIO_PORT_0
#define SideKeyGPIOG 0
#define SideKeyGPIOx GPIO_PIN_0		//�ఴ����(P0.0)


#define RedLEDIOP GPIO_PORT_1
#define RedLEDIOG 1
#define RedLEDIOx GPIO_PIN_4		//��ɫָʾ��(P1.4)	


#define GreenLEDIOP GPIO_PORT_1
#define GreenLEDIOG 1
#define GreenLEDIOx GPIO_PIN_3		//��ɫָʾ��(P1.3)



/********************* �������������⹦�ܵ����Ż��߱������� ***********************/

#define SYSHBLEDIOP GPIO_PORT_0
#define SYSHBLEDIOG 0
#define SYSHBLEDIOx GPIO_PIN_4			//����LED(P0.4)






#endif
