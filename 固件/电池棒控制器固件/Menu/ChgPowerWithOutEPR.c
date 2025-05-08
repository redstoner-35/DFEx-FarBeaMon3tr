#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"

const EnumEditEntryDef PowerCfgNoEPR[5]=
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
		{ //ռλ��
		"",
	  false,
		100,
		true
		}
	};

//��������
int ReadPWREnumValue(void);
void FedPWREnumValue(int Input);

const MenuConfigDef PowerSetMenuNoEPR=
	{
	MenuType_EnumSetup,
	//����������
	NULL,
	//ö�ٱ༭�����
	PowerCfgNoEPR,
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
