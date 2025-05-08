#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"
#include "CapTest.h"

//�����Ƿ���Զ�ȡ
bool IsEnableFactoryReset=false;

//�����Ƿ���Ҫ�ָ�����
void CalcIfNeedToReset(void)
	{
	unsigned int CurrentCRC;
	CfgUnionDef buf;
	//�����������õ�CRC
	CurrentCRC=CalcROMCRC32(&CfgUnion);
	LoadDefaultConfig(&buf);
	if(CurrentCRC!=CalcROMCRC32(&buf))IsEnableFactoryReset=true;
	else IsEnableFactoryReset=false;
	}

//�ص����˵�
void ReturnFromRSTMenu(void)
	{
	SwitchingMenu(&SetMainMenu);
	}

void ResetCTest(void)
	{
	SwitchingMenu(&ResetCTestMenu);
	}	

void ResetSysCfg(void)
	{
	SwitchingMenu(&ResetSysConfigtMenu);
	}	
	
void ResetColumGauge(void)
	{
	SwitchingMenu(&ResetColMenu);
	}	
	
//�˵������
const SetupMenuSelDef RSTSetup[4]=
	{
		{
		"����ϵͳ����",
		false,
		&IsEnableFactoryReset,
		&ResetSysCfg,
		},
		{
		"���ò���ϵͳ����",
		false,
		&LastCData.IsDataValid,
		&ResetCTest,
		},	
		{
		"���ÿ��ؼ�����",
		false,
		&AlwaysTrue,
		&ResetColumGauge,
		},	
		{
		"\0",
		true,
		&AlwaysTrue,
		NULL,
		}		
	};
	
//�˵�������	
const MenuConfigDef RSTMainMenu=
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
	"����ϵͳ����",
	RSTSetup,
	NULL,
	&ReturnFromRSTMenu, 
	//�����ʱ����Ҫ�����µ�ǰ�����ǲ��ǳ�������
	&CalcIfNeedToReset,
	NULL
	};	
