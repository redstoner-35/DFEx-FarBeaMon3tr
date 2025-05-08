#include "Config.h"
#include "Key.h"
#include "GUI.h"

const EnumEditEntryDef IStopCfg[8]=
	{
		{
		"200mA",
	  false,
		IStop_200mA,
		false,
		},		
		{
		"250mA",
	  false,
		IStop_250mA,
		false,
		},		
		{
		"300mA",
	  false,
		IStop_300mA,
		false,
		},	
		{
		"350mA",
	  false,
		IStop_350mA,
		false,
		},		
		{
		"400mA",
	  false,
		IStop_400mA,
		false,
		},	
		{
		"450mA",
	  false,
		IStop_450mA,
		false,
		},		
		{
		"0.5A(���Ƽ�)",
	  true,
		IStop_500mA,
		false,
		},			
		{ //ռλ��
		"",
	  false,
		100,
		true
		}
	};

int ReadIStopEnumValue(void)
	{
	//���س�繦�ʵ�enumֵ
	return (int)CfgData.IStop;
	}
	
void FedIStopEnumValue(int Input)
	{	
	extern bool IsEnable17AMode;
	CfgData.IStop=(IStopConfig)Input;
	if(IsEnable17AMode)
		{
		//2.5mR����̼���Ϊͣ��������Ƶ���100��150mA�䲻������Ҫǿ�����õ�200mA
		if(CfgData.IStop==IStop_100mA||CfgData.IStop==IStop_150mA)CfgData.IStop=IStop_200mA;
		}
	SwitchingMenu(&ChgSysSetMenu);
	}

const MenuConfigDef IstopSetMenu=
	{
	MenuType_EnumSetup,
	//����������
	NULL,
	//ö�ٱ༭�����
	IStopCfg,
  &ReadIStopEnumValue,
  &FedIStopEnumValue,		
	//������Ⱦ�Ĵ���
	NULL, //��Ⱦ����
	NULL, //��������
	//�����ò˵�
	"ͣ���������",
	NULL,
	NULL,
	NULL, 
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};
