#include "GUI.h"
#include "Key.h"
#include "delay.h"

void ShutSysOFF(void);
void SetDebugPortState(bool IsEnable);

//�Լ������
void SelfTestErrorHandler(void)
	{
	extern bool SensorRefreshFlag;
	extern bool IsEnablePowerOFF;
	//��ʾ����
	LCD_Fill(5,3,159,18,BLACK);
	LCD_Fill(33,16,123,42,BLACK);
	LCD_ShowChinese(5,3,"ϵͳ��ʼ��ʧ�ܣ�",RED,BLACK,0);
	LCD_ShowChinese(5,17,"����������Ի�ر�ϵͳ",WHITE,BLACK,0);
	KeyState.KeyEvent=KeyEvent_None;
	SetDebugPortState(true); //���ֹ��ϵ�ʱ���debug��
	while(1)
		{
		SideKey_LogicHandler(); 
		if(KeyState.KeyEvent!=KeyEvent_None) //����������ػ�
			{
			IsEnablePowerOFF=true;
			KeyState.KeyEvent=KeyEvent_None;
			ShutSysOFF();
			}
		if(!SensorRefreshFlag)continue;
		SideKey_TIMCallback(); 
		SensorRefreshFlag=false;
		}
	}
