#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"
#include "CapTest.h"
#include "LogSystem.h"


//����
extern bool IsEnablePowerOFF;
extern bool IsEnableAdapEmu;
extern bool EnableOneShotAct;
extern bool EnableManuBal;

//��������
void EnterChargeTest(void);
void ManuallyShutSystemOFF(void);
void EnterCTHistory(void);
void ViewColData(void);
void ResetColumGauge(void);
void EnterAbout(void);
void EnterAdapterEmu(void);
void SendTCResetCommand(void);
void SetDisplayDir(void);
void ActOneShotTest(void);
void EnterPSet(void);
void EnterGUIPref(void);
void ViewChipState(void);
void EnterManuBal(void);

//����߼�ģʽ֮ǰ������
void EnterAdvMode(void)
	{
	SwitchingMenu(&EnterAdvancedMenu);
	}

//�˵������
const SetupMenuSelDef EasySetup[19]=
	{	
		{
		"һ��������",
		false,
		&AlwaysTrue,
		&EnterChargeTest
		},
		{
		"��ŵ繦������",
		false,
		&CfgData.EnableChargPowerConfig,
		&EnterPSet
		},
		{
		"����һ���Բ���",
		false,
		&EnableOneShotAct,
		&ActOneShotTest
		},
		{
		"�鿴��ʷ��������",
		false,
		&LastCData.IsDataValid,
		&EnterCTHistory
		},
		{
		"�鿴���ؼ���ʷ����",
		false,
		&LogHeader.IsRunlogHasContent,
		&ViewColData,
		},
		{
		"���ÿ��ؼ�����",
		false,
		&LogHeader.IsRunlogHasContent,
		&ResetColumGauge,
		},		
		{
		"������ģ��",
		false,
		&IsEnableAdapEmu,			
		&EnterAdapterEmu
		},
		{
		"�ֶ��������",
		false,
		&EnableManuBal,
		&EnterManuBal,
		},
		{
		"Type-C��·����",
		false,
		&AlwaysTrue,		
		&SendTCResetCommand
		},
		{
		"��ʾ��������",
		false,
		&AlwaysTrue,			
		&SetDisplayDir
		},
		{
		"GUI��ѡ������",
		false,
		&AlwaysTrue,			
		&EnterGUIPref
		},
		{
		"�鿴оƬ״̬",
		false,
		&AlwaysTrue,
		&ViewChipState,
		},
		{
		"�ر�ϵͳ",
		false,
		&IsEnablePowerOFF,
		&ManuallyShutSystemOFF,
		},		
		{
		"����߼�ģʽ",
		false,
		&AlwaysTrue,
		&EnterAdvMode,
		},		
		{
		"����",
		false,
		&AlwaysTrue,
		&EnterAbout,
		},
		{
		"\0",
		true,
		&AlwaysTrue,
		NULL,
		}
	};

void BackFromEsetupToMainMenu(void)
	{
	ClearScreen(); //����
	WriteConfiguration(&CfgUnion,false); //д������
	SwitchingMenu(&MainMenu);
	}	
	
const MenuConfigDef EasySetMainMenu=
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
	"ϵͳ����",
	EasySetup,
	NULL,
	&BackFromEsetupToMainMenu, 
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};
