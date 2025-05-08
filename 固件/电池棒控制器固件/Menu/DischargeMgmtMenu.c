#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"

const BoolListEntryDef DisParam[6]=
	{
		{
		"�ŵ�ϵͳ�ܿ���",
		true,
		&CfgData.OutputConfig.IsEnableOutput,
		false,
		true
		},
		{
		"PDЭ��",
		true,
		&CfgData.OutputConfig.IsEnablePDOut,
		false,
		false
		},		
		{
		"SCP(��Ϊ25W)",
		true,
		&CfgData.OutputConfig.IsEnableSCPOut,
		false,
		false
		},
		{
		"QC,AFC����Э��",
		true,
		&CfgData.OutputConfig.IsEnableDPDMOut,
		false,
		false
		},		
		{
		"�߹���SCP(100W)",
		true,
		&CfgData.OutputConfig.IsEnableHSCPOut,
		false,
		false
		},	
		{ //ռλ��
		"",
		false,
		&AlwaysFalse,
		true,
		false
		}		
	};

	
const BoolListEntryDef DisParamNoHSCP[5]=
	{
		{
		"�ŵ�ϵͳ�ܿ���",
		true,
		&CfgData.OutputConfig.IsEnableOutput,
		false,
		true
		},
		{
		"PDЭ��",
		true,
		&CfgData.OutputConfig.IsEnablePDOut,
		false,
		false
		},		
		{
		"SCP(��Ϊ25W)",
		true,
		&CfgData.OutputConfig.IsEnableSCPOut,
		false,
		false
		},
		{
		"QC,AFC����Э��",
		true,
		&CfgData.OutputConfig.IsEnableDPDMOut,
		false,
		false
		},		
		{ //ռλ��
		"",
		false,
		&AlwaysFalse,
		true,
		false
		}		
	};	
	
void LeaveDisMgmtMenu(void)
	{
	SwitchingMenu(&SetMainMenu);
	}

	
const MenuConfigDef DisChgCfgMenuNoHSCP=
	{
	MenuType_BoolListSetup,
	//����������
	DisParamNoHSCP,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//������Ⱦ�Ĵ���
	NULL, //��Ⱦ����
	NULL, //��������
	//�����ò˵�
	"�ŵ�ϵͳ����",
	NULL,
	NULL,
	&LeaveDisMgmtMenu, 
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};
	
const MenuConfigDef DisChgCfgMenu=
	{
	MenuType_BoolListSetup,
	//����������
	DisParam,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//������Ⱦ�Ĵ���
	NULL, //��Ⱦ����
	NULL, //��������
	//�����ò˵�
	"�ŵ�ϵͳ����",
	NULL,
	NULL,
	&LeaveDisMgmtMenu, 
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};	
