#include "LCD_Init.h"
#include "Config.h"
#include "GUI.h"

//��������
void SetScreenDirection(void);

const EnumEditEntryDef DirCfg[3]=
	{
		{
		"����������ʾ",
	  true,
		LCDDisplay_Hori_Invert,
		false,
		},
		{
		"��ת������ʾ",
	  true,
		LCDDisplay_Hori_Normal,
		false,
		},
		{ //ռλ��
		"",
	  false,
		100,
		true
		}
	};
	
int ReadDisplayEnumValue(void)
	{
	//���س�繦�ʵ�enumֵ
	return (int)CfgData.DisplayDir;
	}
	
void FedDisplayEnumValue(int Input)
	{
	CfgData.DisplayDir=(LCDDisplayDirDef)Input;
	SetScreenDirection(); //���³�ʼ����Ļ
	if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
	else SwitchingMenu(&SetMainMenu); //�����˳�״̬,����ESC��ص����˵�
	}

const MenuConfigDef DisPlayDirMenu=
	{
	MenuType_EnumSetup,
	//����������
	NULL,
	//ö�ٱ༭�����
	DirCfg,
  &ReadDisplayEnumValue,
  &FedDisplayEnumValue,		
	//������Ⱦ�Ĵ���
	NULL, //��Ⱦ����
	NULL, //��������
	//�����ò˵�
	"��ʾ��������",
	NULL,
	NULL,
	NULL, 
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};
