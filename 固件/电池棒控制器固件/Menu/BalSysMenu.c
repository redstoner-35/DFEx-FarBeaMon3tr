#include "Config.h"
#include "Key.h"
#include "GUI.h"

const EnumEditEntryDef BALModeCfg[5]=
	{
		{
		"ʼ�տ���",
	  true,
		Balance_AlwaysEnabled,
		false,
		},
		{
		"�����ʱ����",
	  true,
		Balance_ChgOnly,
		false,
		},
		{
		"����ŵ�ʱ����",
	  true,
		Balance_ChgDisOnly,
		false,
		},
		{
		"ʼ�չر�(���Ƽ�)",
	  true,
		Balance_Diasbled,
		false,
		},
		{ //ռλ��
		"",
	  false,
		100,
		true
		}
	};
int ReadBalEnumValue(void)
	{
	//���ؾ���ϵͳ��enumֵ
	return (int)CfgData.BalanceMode;
	}
	
void FedBalEnumValue(int Input)
	{
	CfgData.BalanceMode=(BalanceModeDef)Input;
	SwitchingMenu(&SetMainMenu); //�����˳�״̬,����ESC��ص����˵�
	}

const MenuConfigDef BalSysSetMenu=
	{
	MenuType_EnumSetup,
	//����������
	NULL,
	//ö�ٱ༭�����
	BALModeCfg,
  &ReadBalEnumValue,
  &FedBalEnumValue,		
	//������Ⱦ�Ĵ���
	NULL, //��Ⱦ����
	NULL, //��������
	//�����ò˵�
	"������������",
	NULL,
	NULL,
	NULL, 
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};
