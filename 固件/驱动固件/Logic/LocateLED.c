#include "delay.h"
#include "LEDMgmt.h"
#include "GPIO.h"
#include "LocateLED.h"
#include "SideKey.h"
#include "SysConfig.h"
#include "ModeControl.h"
#include "PinDefs.h"
#include "cms8s6990.h"

//ȫ�ֱ���
xdata LocLEDEditDef LocLEDState=LocateLED_NotEdit;
static xdata char LocLEDTIM;
static xdata u8 LocSetTimeOutTIM;

//��λLED�������ʱʱ��
#define LocateLEDTimeOut 30

//��λLED��ʾ��ʱ��
void LocateLED_TIMHandler(void)
	{
	if(LocLEDTIM)LocLEDTIM--;
	if(LocSetTimeOutTIM)LocSetTimeOutTIM--;
	}

//��ʾ��ǰϵͳ���õĶ�λLED����
LEDStateDef LocateLED_ShowType(void)
	{
	IsHalfBrightness=SysCfg.LocatorCfg?1:0;
	//����״̬	
	switch(SysCfg.LocatorCfg)
		{
	  case Locator_OFF:	//��ɫÿ��һ��ʱ�������ʾ�ر�
			if(!LocLEDTIM)
				{
        MakeFastStrobe(LED_Red);
				LocLEDTIM=6;
				}
			break;
		case Locator_Green:return LED_Green; //�̵�
		case Locator_Red:return LED_Red; //���
		case Locator_Amber:return LED_Amber; //�Ƶ�		
		}
	//����״̬����OFF
	return LED_OFF;
	}

//ʹ�ܶ�λLED
void LocateLED_Enable(void)
	{
	GPIOCfgDef LEDInitCfg;
	//���ýṹ��
	LEDInitCfg.Mode=GPIO_IPU;
  LEDInitCfg.Slew=GPIO_Slow_Slew;		
	LEDInitCfg.DRVCurrent=GPIO_High_Current; //����Ϊ��������
	//�����̵�GPIO	
	if(SysCfg.LocatorCfg&0x01)GPIO_ConfigGPIOMode(GreenLEDIOG,GPIOMask(GreenLEDIOx),&LEDInitCfg);
	GPIO_SetMUXMode(GreenLEDIOG,GreenLEDIOx,GPIO_AF_GPIO);	
	//���ú��GPIO
	if(SysCfg.LocatorCfg&0x02)GPIO_ConfigGPIOMode(RedLEDIOG,GPIOMask(RedLEDIOx),&LEDInitCfg);	
	GPIO_SetMUXMode(RedLEDIOG,RedLEDIOx,GPIO_AF_GPIO);	
	}

//��λLED״̬�༭
char LocateLED_Edit(char ClickCount)
	{
	switch(LocLEDState)
		{
		//Ĭ��״̬
		case LocateLED_NotEdit:
			//�ػ�״̬7����������༭
			if(ClickCount!=7)return 0;			
			LocLEDTIM=0;
			LocSetTimeOutTIM=8*LocateLEDTimeOut;
			LocLEDState=LocateLED_Sel;
		  break;
		//�༭����
		case LocateLED_Sel:
			if(ClickCount)SysCfg.LocatorCfg=SysCfg.LocatorCfg<3?SysCfg.LocatorCfg+1:0; //�����л�index
		  if(!LocSetTimeOutTIM)
				{
				//���ò˵���ʱ�������沢�˳�
				LocLEDState=LocateLED_NotEdit;
				LEDMode=LED_RedBlinkFifth;      //��ɫLED����α�ʾ�༭�쳣����
				}
			if(!getSideKeyHoldEvent())break; //�����⵽�����򱣴沢�˳�		  
			DisplayLockedTIM=4; //����ָʾ��һ��
			LocLEDState=LocateLED_WaitKeyRelease;
			SaveSysConfig(0);
		  break;
		//�ȴ������ſ�
		case LocateLED_WaitKeyRelease:
			if(getSideKeyHoldEvent())break;  //�ȴ������ſ�
		  getSideKeyLongPressEvent();  
			LocLEDState=LocateLED_NotEdit; //��ȡһ�鳤���¼����ⳤ���˳������ʱ�򿪻����¹� 
      break;
		}		
  //�����¼�������1
	return 1;	
	}