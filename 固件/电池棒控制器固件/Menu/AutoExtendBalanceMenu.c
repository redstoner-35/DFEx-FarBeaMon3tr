#include "GUI.h"
#include "LogSystem.h"
#include "ADC.h"
#include <string.h>
#include "Key.h"
#include "BalanceMgmt.h"
	
extern bool IsUpdateBalUI;
extern bool IsTimeMet;

//�Զ�������Ⱦ
void AutoBalMenuRenderHandler(void)
	{
	int H,M,S;
	extern int SleepTimer;	
	//����Ƿ���Ҫ��Ⱦ
	SleepTimer=480; //���������ڼ��ֹϵͳ��λ������ʱ�临λΪһ����	
	if(!(BalanceForceEnableTIM%8)&&!IsTimeMet)
		{
		IsUpdateBalUI=true; //ÿ�����һ��UI
		IsTimeMet=true; //ֻ��Ҫ����һ�ξ��У������ظ�����
		LogData.BalanceTime++;
		}
	else if(BalanceForceEnableTIM%8)IsTimeMet=false; //�Ǹ���ʱ�䣬���и���		
	//ʱ�䵽���ص�������	
	if(BalanceForceEnableTIM<=0)
		{
		//������־
		LogData.UnbalanceBatteryAh=0; //���ξ��������	
		RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data); //����������־��CRC32
		WriteRuntimeLogToROM(); //������־
		//�ص�������
		ClearScreen(); //����
		SwitchingMenu(&MainMenu);
		}
	if(!IsUpdateBalUI&&KeyState.KeyEvent==KeyEvent_None)return;
	//ʵ�ʵ���Ⱦ����
	RenderMenuBG(); //��ʾ����	
	LCD_ShowChinese(33,22,"�Զ�����������",GREEN,LGRAY,0);
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
	//��Ⱦ��ϣ���λ
	IsUpdateBalUI=false;
	}
	
//�Զ����ⰴ������
void AutoBalMenuKeyHandler(void)
	{
	if(KeyState.KeyEvent==KeyEvent_ESC||ADCO.Vbatt<10.1)
		{
		IsUpdateBalUI=true;
		BalanceForceEnableTIM=0;
		RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data); //����������־��CRC32
		WriteRuntimeLogToROM(); //������־
		ClearScreen(); //����
	  SwitchingMenu(&MainMenu);
		}
	//��������¼�
  KeyState.KeyEvent=KeyEvent_None;
	}
	
//��������
void EnableAutoBal(void)	
	{
	IsUpdateBalUI=true;
	BalanceForceEnableTIM=3600*8*5; //5��Сʱ
	}

//�˵�����
const MenuConfigDef AutoBALMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
  &AutoBalMenuRenderHandler,
	&AutoBalMenuKeyHandler,	
	//�������ò˵�����Ҫ�ñ������
	"�Զ���������\0",
	NULL,
	NULL, 
	NULL,
	//������˳����캯��
	&EnableAutoBal,
	NULL
	};
