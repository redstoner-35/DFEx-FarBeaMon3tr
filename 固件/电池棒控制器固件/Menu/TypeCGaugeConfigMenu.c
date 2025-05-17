#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"

//���빦�ʼƵ�ѹ����У׼�ĵط�
void EnterTCICAL(void)
	{
	SwitchingMenu(&TypeICALMenu);
	}
	
void EnterTCVCAL(void)
	{
	SwitchingMenu(&TypeCVCALMenu);
	}

void EnterDsourceSel(void)
	{
	SwitchingMenu(&TypeCCgaugeDSourceMenu);
	}	
	
//�˵������
const SetupMenuSelDef TCGaugeSetup[4]=
	{
		{
		"TypeC��������Դ",
		false,
		&AlwaysTrue,
		&EnterDsourceSel
		},
		{
		"���ʼƵ�ѹУ׼",
		false,
		&AlwaysTrue,
		&EnterTCVCAL
		},
		{
		"���ʼƵ���У׼",
		false,
		&AlwaysTrue,
		&EnterTCICAL
		},
		{
		"\0",
		true,
		&AlwaysTrue,
		NULL,
		}
	};

//�ⲿ��������
void ReturnToMainSetMenu(void);
	
const MenuConfigDef TypeCGaugeSetMenu=
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
	"TypeC���ʼ�����",
	TCGaugeSetup,
	NULL,
	&ReturnToMainSetMenu, 
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};	
