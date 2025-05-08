#include "GUI.h"
#include "Config.h"
#include "ADC.h"
#include "LogSystem.h"
#include <math.h>
#include <string.h>
#include "Key.h"
#include "BalanceMgmt.h"

//�˵��ڲ�״̬�궨��
typedef enum
	{
	BalMenu_SetTime,
	BalMenu_Running,
	BalMenu_Finished,
	BalMenu_Failed,	
	}BalMenuState;

//�ڲ�ȫ��
static int BalanceHour;
static BalMenuState BalState;
	
//ȫ��	
bool IsUpdateBalUI;
bool IsTimeMet;
	
//����˵�״̬��״̬������
void BalMenuFSMProcess(void)
	{
	extern int SleepTimer;
	switch(BalState)
		{
		case BalMenu_SetTime:
			 if(KeyState.KeyEvent==KeyEvent_Up&&BalanceHour<10)BalanceHour++;
		   if(KeyState.KeyEvent==KeyEvent_Down&&BalanceHour>1)BalanceHour--; //��������ʱ��
		   if(KeyState.KeyEvent==KeyEvent_ESC)
					{
					if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
					else SwitchingMenu(&SetMainMenu); //�����˳�״̬,����ESC��ص����˵�
					}
			 if(KeyState.KeyEvent==KeyEvent_Enter)
					{
					BalanceForceEnableTIM=3600*8*BalanceHour;
					BalState=BalMenu_Running;
					IsUpdateBalUI=true;
					}
			 break;
	  //����������
		case BalMenu_Running:
			  SleepTimer=480; //���������ڼ��ֹϵͳ��λ������ʱ�临λΪһ����
				if(KeyState.KeyEvent==KeyEvent_ESC||ADCO.Vbatt<10.1)
					{
					IsUpdateBalUI=true;
					BalanceForceEnableTIM=0;
					BalState=BalMenu_Failed;
					break;
					}
				if(BalanceForceEnableTIM>0)
					{
					if(!(BalanceForceEnableTIM%8)&&!IsTimeMet)
						 {
						 IsUpdateBalUI=true; //ÿ�����һ��UI
						 IsTimeMet=true; //ֻ��Ҫ����һ�ξ��У������ظ�����
						 LogData.BalanceTime++; //�����������ڼ��ۼ�ʱ��
						 }
					else if(BalanceForceEnableTIM%8)IsTimeMet=false; //�Ǹ���ʱ�䣬���и���
					break;
					}
				else //�����ѽ������رվ�����
					{
					//������־���δ�����ŵ�����
					LogData.UnbalanceBatteryAh=0; //���ξ��������	
					RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data); //����������־��CRC32
					WriteRuntimeLogToROM(); //������־
					//��ת����������Ľ���
					IsUpdateBalUI=true;
					BalState=BalMenu_Finished;
					}
				break; 
		//��ɺʹ���״̬������ESC�˳�
		case BalMenu_Finished:
		case BalMenu_Failed:
			 if(KeyState.KeyEvent!=KeyEvent_ESC)break;
	     RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data); //����������־��CRC32
			 WriteRuntimeLogToROM(); //������־
			 if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
			 else SwitchingMenu(&SetMainMenu); //�����˳�״̬,����ESC��ص����˵�
			 break;	  
		}
	//������Ӧ�������¼�
	if(KeyState.KeyEvent!=KeyEvent_None)IsUpdateBalUI=true; //UI�������£����»���
	KeyState.KeyEvent=KeyEvent_None;
	}

//�ֶ�����˵���GUI��Ⱦ	
void BalMenuGUIHandler(void)
	{
	int H,M,S;
	if(!IsUpdateBalUI&&KeyState.KeyEvent==KeyEvent_None)return;
	RenderMenuBG(); //��ʾ����
	switch(BalState)
		{
		//����ʱ��
		case BalMenu_SetTime:	
			LCD_ShowChinese(35,21,"��ָ������ʱ��",WHITE,LGRAY,0);
			LCD_ShowChinese(32,42,"����",WHITE,LGRAY,0);
		  LCD_ShowIntNum(54,42,BalanceHour,2,YELLOW,LGRAY,12);
		  LCD_ShowChinese(70,42,"����Сʱ",WHITE,LGRAY,12);
			LCD_ShowChinese(32,61,"����",WHITE,LGRAY,0);
		  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,61,"���˳�",WHITE,LGRAY,0);
		  break;
		//����������
		case BalMenu_Running:
		  LCD_ShowChinese(33,22,"�ֶ�����������",GREEN,LGRAY,0);
		  H=BalanceForceEnableTIM/8;
		  M=(H%3600)/60;
		  S=H%60;
		  H/=3600;
			LCD_ShowHybridString(14,40,"ʣ��:",WHITE,LGRAY,0);
		  LCD_ShowIntNum(51,40,H,1,GREEN,LGRAY,12);
		  LCD_ShowChinese(62,40,"ʱ",WHITE,LGRAY,12);
		  LCD_ShowIntNum(78,40,M,2,GREEN,LGRAY,12);
		  LCD_ShowChinese(98,40,"��",WHITE,LGRAY,12);
		  LCD_ShowIntNum(114,40,S,2,GREEN,LGRAY,12);
		  LCD_ShowChinese(134,40,"��",WHITE,LGRAY,12);
		  LCD_ShowChinese(32,61,"����",WHITE,LGRAY,0);
		  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,61,"���˳�",WHITE,LGRAY,0);
		  break;
		//�ֶ��������	
    case BalMenu_Finished:			
			LCD_ShowChinese(33,22,"�ֶ����������",GREEN,LGRAY,0);
			LCD_ShowChinese(32,41,"����",WHITE,LGRAY,0);
		  LCD_ShowIntNum(54,41,BalanceHour,2,GREEN,LGRAY,12);
		  LCD_ShowChinese(70,41,"����Сʱ",WHITE,LGRAY,12);
		  LCD_ShowChinese(32,61,"����",WHITE,LGRAY,0);
		  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,61,"���˳�",WHITE,LGRAY,0);
		  break;
		//����ʧ��
		case BalMenu_Failed:	
			LCD_ShowChinese(28,22,"�ֶ������쳣����",RED,LGRAY,0);
			LCD_ShowChinese(32,61,"����",WHITE,LGRAY,0);
			LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
			LCD_ShowChinese(86,61,"���˳�",WHITE,LGRAY,0);
		  break;
		}
	IsUpdateBalUI=false;
	}
	
//������˳�����ϵͳ��ʱ����в���
void ResetManuBalModule(void)
	{
	BalanceForceEnableTIM=0;
	BalanceHour=1;
	IsUpdateBalUI=true;
	BalState=BalMenu_SetTime;
	}	
	
//�˵�����
const MenuConfigDef BALTestMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
  &BalMenuGUIHandler,
	&BalMenuFSMProcess,	
	//�������ò˵�����Ҫ�ñ������
	"�ֶ���������\0",
	NULL,
	NULL, 
	NULL,
	//������˳����캯��
	&ResetManuBalModule,
	&ResetManuBalModule
	};
