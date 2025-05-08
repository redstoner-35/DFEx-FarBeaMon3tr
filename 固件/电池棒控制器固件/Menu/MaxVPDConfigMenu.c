#include "Config.h"
#include "Key.h"
#include "GUI.h"

const EnumEditEntryDef MaxVPDCfg[3]=
	{
		{
		"PD3.0(20V)",
	  false,
		PDMaxIN_20V,
		false,
		},
		{
		"PD3.1-EPR(28V)",
	  false,
		PDMaxIN_28V,
		false,
		},
		{ //ռλ��
		"",
	  false,
		100,
		true
		}
	};

int ReadMaxVPDEnumValue(void)
	{
	//���س�繦�ʵ�enumֵ
	return (int)CfgData.MaxVPD;
	}
	
void FedMaxVPDEnumValue(int Input)
	{
	extern bool IsEnable17AMode;
	MaximumPDVoltageDef buf;
	//��ȡ���	
	if(IsEnable17AMode)buf=(MaximumPDVoltageDef)Input;
	else buf=PDMaxIN_20V; //��ͨ�̼���ֹ����
	//���ó���ģʽ	
	if(CfgData.MaxVPD!=buf&&buf==PDMaxIN_28V)
		{
		CfgData.InputConfig.ChargeCurrent=IP2366_ICCMAX;
		CfgData.OverHeatLockTemp=100; //���䷢�Ⱦ޴���Ҫ�����¶�
		CfgData.InputConfig.ChargePower=Power_140W;
    CfgData.MaxVPD=PDMaxIN_28V;
		WriteConfiguration(&CfgUnion,false);	
		}
	//���½��
	CfgData.MaxVPD=buf;
	//�������Ϊ20V Max��ر�EPR
	if(CfgData.MaxVPD==PDMaxIN_20V&&CfgData.InputConfig.ChargePower==Power_140W)	
		{
		CfgData.OverHeatLockTemp=90; //��ͨģʽ�����¶�
		if(CfgData.InputConfig.ChargeCurrent>9700)CfgData.InputConfig.ChargeCurrent=9700;
		CfgData.InputConfig.ChargePower=Power_100W;
		WriteConfiguration(&CfgUnion,false);
		}		
	SwitchingMenu(&ChgSysSetMenu);
	}

const MenuConfigDef MaxVPDMenu=
	{
	MenuType_EnumSetup,
	//����������
	NULL,
	//ö�ٱ༭�����
	MaxVPDCfg,
  &ReadMaxVPDEnumValue,
  &FedMaxVPDEnumValue,		
	//������Ⱦ�Ĵ���
	NULL, //��Ⱦ����
	NULL, //��������
	//�����ò˵�
	"��߳�ŵ�ѹ����",
	NULL,
	NULL,
	NULL, 
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};
