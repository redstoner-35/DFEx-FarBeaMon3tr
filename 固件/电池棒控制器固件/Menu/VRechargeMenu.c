#include "Config.h"
#include "Key.h"
#include "GUI.h"
#include <string.h>
#include <stdio.h>
#include "ADC.h"

//�ٳ���ѹ�ַ�
static char VRechargeMsg[3][21]={0};
static const char ShowVRProc[]="%.2fV(%.3fV/Cell)";

const EnumEditEntryDef RechargeCfg[5]=
	{
		{
		"�ر��ٳ�繦��",
	  true,
		Recharge_Disable,
		false,
		},
		{
		&VRechargeMsg[0][0],
	  false,
		Recharge_0V05,
		false,
		},
		{
		&VRechargeMsg[1][0],
	  false,
		Recharge_0V1,
		false,
		},	
		{
		&VRechargeMsg[2][0],
	  false,
		Recharge_0V2,
		false,
		},		
		{ //ռλ��
		"",
	  false,
		100,
		true
		}
	};

int ReadRechargeEnumValue(void)
	{
	float buf;
	int i;
	//���м���	
	for(i=0;i<3;i++)memset(VRechargeMsg[i],0,21);
	buf=((float)CfgData.InputConfig.FullVoltage)/1000;
	snprintf(VRechargeMsg[0],21,ShowVRProc,BattCellCount*(buf-0.05),buf-0.05);
	snprintf(VRechargeMsg[1],21,ShowVRProc,BattCellCount*(buf-0.1),buf-0.1);
	snprintf(VRechargeMsg[2],21,ShowVRProc,BattCellCount*(buf-0.2),buf-0.2);
	//���س�繦�ʵ�enumֵ
	return (int)CfgData.VRecharge;
	}
	
void FedRechargeEnumValue(int Input)
	{
	CfgData.VRecharge=(ReChargeConfig)Input;
	SwitchingMenu(&ChgSysSetMenu);
	}

const MenuConfigDef RechargeSetMenu=
	{
	MenuType_EnumSetup,
	//����������
	NULL,
	//ö�ٱ༭�����
	RechargeCfg,
  &ReadRechargeEnumValue,
  &FedRechargeEnumValue,		
	//������Ⱦ�Ĵ���
	NULL, //��Ⱦ����
	NULL, //��������
	//�����ò˵�
	"�ٳ����ֵ����",
	NULL,
	NULL,
	NULL, 
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};
