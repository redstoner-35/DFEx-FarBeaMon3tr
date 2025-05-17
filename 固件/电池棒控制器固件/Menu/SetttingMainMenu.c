#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"
#include "CapTest.h"
#include "LogSystem.h"
#include "delay.h"
#include "ADC.h"
#include "BalanceMgmt.h"

//����
bool IsEnablePowerOFF=false;
bool IsEnableAdapEmu=false;
bool IsPDOCanConfig=false;
bool IsEnableLVConfig=false;
bool EnableOneShotAct=false;
bool EnableManuBal=false;

//��������
void ShutSysOFF(void);
void IP2366_ReInitBasedOnConfig(void);
void IP2366_SetIBatLIMBaseOnSysCfg(void);

//����Ƿ���Թر�ϵͳ
void UpdateIfSysCanOFF(void)
	{
	BatteryStateDef State;
	//Type-C�Ͽ����Ӳ�����ػ�
	IP2366_GetChargerState(&State);
	IsEnablePowerOFF=State==Batt_StandBy?true:false;
	if(!CfgData.OutputConfig.IsEnableOutput)IsEnableAdapEmu=false;
	if(State==Batt_StandBy)IsEnableAdapEmu=true;
	else if(State==Batt_discharging)IsEnableAdapEmu=true;
	else IsEnableAdapEmu=false;
	//���PDO�����Ƿ���
	if(!CfgData.EnablePDOConfig)IsPDOCanConfig=false;
	else if(!CfgData.OutputConfig.IsEnableOutput)IsPDOCanConfig=false;
	else if(!CfgData.OutputConfig.IsEnablePDOut)IsPDOCanConfig=false;
	else IsPDOCanConfig=true;
	//�����Ƿ����������
	if(!CfgData.EnableLVProtectConfig)IsEnableLVConfig=false;
	else if(!CfgData.OutputConfig.IsEnableOutput)IsEnableLVConfig=false;
	else IsEnableLVConfig=true;
	//����һ���Բ���
	EnableOneShotAct=CfgData.InstantCTest==InstantCTest_NotTriggered?true:false;
	//�����Ƿ������ֶ�����	
	if(ADCO.Vbatt<10.1||!IsEnablePowerOFF)EnableManuBal=false;
	else EnableManuBal=true;
	}
	
//�ر�ϵͳ
void ManuallyShutSystemOFF(void)
	{
	WriteConfiguration(&CfgUnion,false);
	IsEnableAdvancedMode=false;
	Balance_ForceDiasble();
	delay_ms(100);
	ShutSysOFF();
	}

//�ص����˵�
void ReturnToMainMenu(void)
	{
	//��������Ƿ����仯����������仯�������³�ʼ��оƬӦ������
	if(!CheckIfConfigIsSame())
		{
		IP2366_ReInitBasedOnConfig(); //����оƬ����
		IP2366_SetIBatLIMBaseOnSysCfg(); //���ö�̬����
		}
	//��ȥ֮ǰ���ȱ������ã�Ȼ���˳�	
	WriteConfiguration(&CfgUnion,false);
	IsEnableAdvancedMode=false;
	ClearScreen(); //����
	SwitchingMenu(&MainMenu);
	}
	
//���빦�����ò˵�
void EnterPSet(void)
	{
	//��������ѡ����ĸ��˵�
	if(CfgData.MaxVPD==PDMaxIN_20V)SwitchingMenu(&PowerSetMenuNoEPR);
	else SwitchingMenu(&PowerSetMenu);
	}	
	
//���빦�����ò˵�
void EnterLVSet(void)
	{
	SwitchingMenu(&LVSetMenu);
	}		

//����ŵ�ϵͳ����
void EnterDisMgmt(void)
	{
	extern bool IsEnableHSCPMode;
	//������̼�
	if(IsEnableHSCPMode)SwitchingMenu(&DisChgCfgMenu);
	else SwitchingMenu(&DisChgCfgMenuNoHSCP);
	}
//���������
void EnterChgMgmt(void)
	{
	SwitchingMenu(&SafeAlmMenu);
	}	

//������������
void EnterChargeTest(void)
	{
	SwitchingMenu(&CapTestMenu);
	}
//����ָ���������
void EnterResetFactory(void)
	{
	SwitchingMenu(&RSTMainMenu);
	}	
//���������ʷ�鿴
void EnterCTHistory(void)
	{
	SwitchingMenu(&CapTestHisMenu);
	}	
//�鿴���ؼ���ʷ����
void ViewColData(void)
	{
	SwitchingMenu(&ColHisMenu);
	}
	
void ViewChipState(void)
	{
	SwitchingMenu(&ChipStatMenu);
	}

void EnterPDOConfig(void)	
	{
	SwitchingMenu(&PDOCfgMenu);
	}

void EnterSecuCfg(void)
	{
	SwitchingMenu(&EnterSecuMenu);
	}	
	
void EnterAbout(void)
	{
	SwitchingMenu(&AboutMenu);
	}	
void EnterTset(void)
	{
	SwitchingMenu(&TSetMenu);
	}
void EnterAdapterEmu(void)
	{
	SwitchingMenu(&AdapterEmuMenu);
	}

void SendTCResetCommand(void)
	{
  SwitchingMenu(&TCResetMenu);
	}

void SetDisplayDir(void)
	{
	SwitchingMenu(&DisPlayDirMenu);
	}	
	
void ActOneShotTest(void)
	{
	SwitchingMenu(&ActOneShotCTestMenu);
	}
	
void EnterGUIPref(void)
	{
	SwitchingMenu(&GUIPrefMenu);
	}	
	
void EnterBalCfg(void)
	{
	SwitchingMenu(&BalSysSetMenu);
	}	
	
void EnterManuBal(void)
	{
	SwitchingMenu(&BALTestMenu);
	}
	
void EnterTypeCGaugeConfig(void)
	{
	SwitchingMenu(&TypeCGaugeSetMenu);
	}	
	
//�˵������
const SetupMenuSelDef MainSetup[23]=
	{
		{
		"ϵͳ��ȫ����",
		false,
		&AlwaysTrue,
		&EnterSecuCfg
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
		"���ȱ����¶�����",
		false,
		&CfgData.EnableOTPConfig,
		&EnterTset
		},
		{
		"��ŵ繦������",
		false,
		&CfgData.EnableChargPowerConfig,
		&EnterPSet
		},
		{
		"������������",
		false,
		&AlwaysTrue,
		&EnterBalCfg
		},
		{
		"�ŵ�ϵͳ����",
		false,
		&CfgData.EnableDischargeConfig,
		&EnterDisMgmt
		},
		{
		"PDO�㲥����",
		false,
		&IsPDOCanConfig,
		&EnterPDOConfig
		},
		{
		"���ϵͳ����",
		false,
		&CfgData.EnableChargeConfig,
		&EnterChgMgmt
		},
		{
		"�ŵ��ѹ��������",
		false,
		&IsEnableLVConfig,
		&EnterLVSet,
		},	
		{
		"�鿴��ʷ��������",
		false,
		&LastCData.IsDataValid,
		&EnterCTHistory
		},
		{
		"������ģ��",
		false,
		&IsEnableAdapEmu,			
		&EnterAdapterEmu
		},
		{
		"����һ���Բ���",
		false,
		&EnableOneShotAct,
		&ActOneShotTest
		},
		{
		"һ��������",
		false,
		&AlwaysTrue,
		&EnterChargeTest
		},
		{
		"�鿴���ؼ���ʷ����",
		false,
		&LogHeader.IsRunlogHasContent,
		&ViewColData,
		},
		{
		"�ֶ��������",
		false,
		&EnableManuBal,
		&EnterManuBal,
		},
		{
		"TypeC���ʼ�����",
		false,
		&CfgData.EnableTCCalibration,
		&EnterTypeCGaugeConfig
		},
		{
		"�鿴оƬ״̬",
		false,
		&AlwaysTrue,
		&ViewChipState,
		},
		{
		"�ָ���������",
		false,
		&AlwaysTrue,
		&EnterResetFactory,
		},		
		{
		"�ر�ϵͳ",
		false,
		&IsEnablePowerOFF,
		&ManuallyShutSystemOFF,
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

const MenuConfigDef SetMainMenu=
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
	"ϵͳ���ã������ߣ�",
	MainSetup,
	NULL,
	&ReturnToMainMenu, 
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};	
