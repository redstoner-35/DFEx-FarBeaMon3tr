#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"

const EnumEditEntryDef PowerCfg[7]=
	{
		{
		"30W(12V2.5A)",
	  false,
		Power_30W,
		false,
		},
		{
		"45W(15V3A)",
	  false,
		Power_45W,
		false,
		},
		{
		"60W(20V3A)",
	  false,
		Power_60W,
		false,
		},	
		{
		"65W(20V3.25A)",
	  false,
		Power_65W,
		false,
		},		
		{
		"100W(������)",
	  true,
		Power_100W,
		false,
		},	
		{
		"140W(������ر���)",
	  true,
		Power_140W,
		false,
		},	
		{ //ռλ��
		"",
	  false,
		100,
		true
		}
	};

int ReadPWREnumValue(void)
	{
	//�жϲ����������Ƿ�����
	if(CfgData.MaxVPD==PDMaxIN_20V&&CfgData.InputConfig.ChargePower==Power_140W)	
		CfgData.InputConfig.ChargePower=Power_100W;		
	//���س�繦�ʵ�enumֵ	
	return (int)CfgData.InputConfig.ChargePower;
	}
	
void FedPWREnumValue(int Input)
	{
	//����enumֵ
	CfgData.InputConfig.ChargePower=(ChargePowerDef)Input;
	//�жϲ����������Ƿ�����
	if(CfgData.MaxVPD==PDMaxIN_20V&&CfgData.InputConfig.ChargePower==Power_140W)	
		CfgData.InputConfig.ChargePower=Power_100W;	
  //���ص���Ӧ���ò˵�
	if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
	else SwitchingMenu(&SetMainMenu); //�����˳�״̬,����ESC��ص����˵�
	}

const MenuConfigDef PowerSetMenu=
	{
	MenuType_EnumSetup,
	//����������
	NULL,
	//ö�ٱ༭�����
	PowerCfg,
  &ReadPWREnumValue,
  &FedPWREnumValue,		
	//������Ⱦ�Ĵ���
	NULL, //��Ⱦ����
	NULL, //��������
	//�����ò˵�
	"��ŵ繦������",
	NULL,
	NULL,
	NULL, 
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};	

