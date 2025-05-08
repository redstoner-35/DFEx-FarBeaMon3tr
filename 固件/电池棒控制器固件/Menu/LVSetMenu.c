#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"

const EnumEditEntryDef LVCfg[9]=
	{
		{
		"2500mV(2.5V)",
	  false,
		VLow_2V5,
		false,
		},
		{
		"2600mV(2.6V)",
	  false,
		VLow_2V6,
		false,
		},
		{
		"2700mV(2.7V)",
	  false,
		VLow_2V7,
		false,
		},
		{
		"2800mV(2.8V)",
	  false,
		VLow_2V8,
		false,
		},
		{
		"2900mV(2.9V)",
	  false,
		VLow_2V9,
		false,
		},
		{
		"3000mV(3.0V)",
	  false,
		VLow_3V0,
		false,
		},
		{
		"3100mV(3.1V)",
	  false,
		VLow_3V1,
		false,
		},
		{
		"3200mV(3.2V)",
	  false,
		VLow_3V1,
		false,
		},		
		{ //ռλ��
		"",
	  false,
		100,
		true
		}
	};

int ReadLVEnumValue(void)
	{
	//���س�繦�ʵ�enumֵ
	return (int)CfgData.Vlow;
	}
	
void FedLVEnumValue(int Input)
	{
	CfgData.Vlow=(VBatLowDef)Input;
	SwitchingMenu(&SetMainMenu);
	}

const MenuConfigDef LVSetMenu=
	{
	MenuType_EnumSetup,
	//����������
	NULL,
	//ö�ٱ༭�����
	LVCfg,
  &ReadLVEnumValue,
  &FedLVEnumValue,		
	//������Ⱦ�Ĵ���
	NULL, //��Ⱦ����
	NULL, //��������
	//�����ò˵�
	"�ŵ��ѹ��������",
	NULL,
	NULL,
	NULL, 
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};	

