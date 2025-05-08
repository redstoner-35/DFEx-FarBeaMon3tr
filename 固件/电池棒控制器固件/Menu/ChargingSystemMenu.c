#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"

//�ⲿ��������
extern bool IsEnable17AMode;

//�����������������
void EnterIChargeSet(void)
	{
	SwitchingMenu(&IChargeSetMenu);
	}
//����Ԥ����������
void EnterPreChargeIset(void)
	{
	SwitchingMenu(&PreChargeISetMenu);
	}	
//����Ԥ����ѹ����
void EnterChgVsetMenu(void)
	{
	SwitchingMenu(&ChgVSetMenu);
	}
//�����ٳ������
void EnterRechargeSetMenu(void)
	{
	SwitchingMenu(&RechargeSetMenu);
	}	
	
void EnterIstopMenu(void)
	{
	extern bool IsEnable17AMode;
  //���ݹ̼�ģʽ��������ѡ���Ӧ�Ĵ���
	if(IsEnable17AMode)SwitchingMenu(&IstopSetMenu);
	else SwitchingMenu(&IstopStdSetMenu);
	}

void EnterMaxVPDMenu(void)
	{
	SwitchingMenu(&MaxVPDMenu);
	}	
	
//�˵������
const SetupMenuSelDef ChargeSystemSetup[7]=
	{
		{
		"��ط�ֵ��������",
		false,
		&AlwaysTrue,
		&EnterIChargeSet
		},
		{
		"Ԥ����������",
		false,
		&AlwaysTrue,
		&EnterPreChargeIset
		},
		{
		"��ѹ����ѹ����",
		false,
		&AlwaysTrue,
		&EnterChgVsetMenu
		},
		{
		"�ٳ����ֵ����",
		false,
		&AlwaysTrue,		
		&EnterRechargeSetMenu
		},
		{
		"ͣ���������",
		false,
		&AlwaysTrue,		
		&EnterIstopMenu
		},
		{
		"��߳�ŵ�ѹ����",
		false,
		&IsEnable17AMode,		
		&EnterMaxVPDMenu
		},
		{
		"\0",
		true,
		&AlwaysTrue,
		NULL,
		}
	};

void ReturnToMainSetMenu(void)
	{
	SwitchingMenu(&SetMainMenu);
	}
	
const MenuConfigDef ChgSysSetMenu=
	{
	MenuType_Setup,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//������Ⱦ�Ĵ���
	NULL, //��Ⱦ����
	NULL, //��������
	//�����ò˵�
	"���ϵͳ����",
	ChargeSystemSetup,
	NULL,
	&ReturnToMainSetMenu, 
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};	
