#include "GUI.h"
#include "Config.h"

const BoolListEntryDef SecuParam[10]=
	{
		{
		"�߼��˵�ֱ�ӷ���",
		true,
		&CfgData.EnableAdvAccess,
		false,
		false
		},
		{
		"ʹ�ܹ��Ƚ����",
		true,
		&CfgData.EnableThermalStepdown,
		false,
		false
		},
		{
		"ʹ�ܳ��ϵͳ����",
		true,
		&CfgData.EnableChargeConfig,
		false,
		false
		},		
		{
		"ʹ�ܷŵ�ϵͳ����",
		true,
		&CfgData.EnableDischargeConfig,
		false,
		false
		},		
		{
		"ʹ�ܵ�ѹ��������",
		true,
		&CfgData.EnableLVProtectConfig,
		false,
		false
		},	
		{
		"ʹ�ܳ�Ź�������",
		true,
		&CfgData.EnableChargPowerConfig,
		false,
		false
		},	
		{
		"ʹ������㲥����",
		true,
		&CfgData.EnablePDOConfig,
		false,
		false
		},	
		{
		"ʹ�ܹ��ȱ�������",
		true,
		&CfgData.EnableOTPConfig,
		false,
		false
		},	
		{
		"ʹ�ܸ߾��ȹ��ʼ�",
		true,
		&CfgData.EnableHPGauge,
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
	
//��ȫ�˵�����
void LeaveDisMgmtMenu(void);

const MenuConfigDef SecuCfgMenu=
	{
	MenuType_BoolListSetup,
	//����������
	SecuParam,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//������Ⱦ�Ĵ���
	NULL, //��Ⱦ����
	NULL, //��������
	//�����ò˵�
	"ϵͳ��ȫ����",
	NULL,
	NULL,
	&LeaveDisMgmtMenu, 
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};	
