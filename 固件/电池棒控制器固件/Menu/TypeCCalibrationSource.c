#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"

const EnumEditEntryDef TypeCDSCfg[3]=
	{
		{
		"IP2366(�;���)",
	  true,
		0,
		false,
		},
		{
		"INA226(�߾���)",
	  true,
		1,
		false,
		},
		{ //ռλ��
		"",
	  false,
		100,
		true
		}
	};

int ReadTCSourceEnumValue(void)
	{		
	//���س�繦�ʵ�enumֵ	
		return CfgData.EnableHPGauge?1:0;
	}
	
void FedTCSourceEnumValue(int Input)
	{
  //���ö�Ӧ����ֵ
	if(Input)CfgData.EnableHPGauge=true;
	else CfgData.EnableHPGauge=false;
  //���ص���Ӧ���ò˵�
  SwitchingMenu(&TypeCGaugeSetMenu); //�����˳�״̬,����ESC��ص�Type-CУ׼�˵�
	}

const MenuConfigDef TypeCCgaugeDSourceMenu=
	{
	MenuType_EnumSetup,
	//����������
	NULL,
	//ö�ٱ༭�����
	TypeCDSCfg,
  &ReadTCSourceEnumValue,
  &FedTCSourceEnumValue,		
	//������Ⱦ�Ĵ���
	NULL, //��Ⱦ����
	NULL, //��������
	//�����ò˵�
	"TypeC��������Դ",
	NULL,
	NULL,
	NULL, 
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};	




