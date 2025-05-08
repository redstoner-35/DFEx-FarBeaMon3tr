#include "Config.h"
#include "Key.h"
#include "GUI.h"

const EnumEditEntryDef IStopStdCfg[11]=
	{
		{
		"50mA(���Ƽ�)",
	  true,
		IStop_50mA,
		false,
		},	
		{
		"100mA",
	  false,
		IStop_100mA,
		false,
		},	
		{
		"150mA",
	  false,
		IStop_150mA,
		false,
		},	
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
	
//����enum��ȡ����
int ReadIStopEnumValue(void);
void FedIStopEnumValue(int Input);

const MenuConfigDef IstopStdSetMenu=
	{
	MenuType_EnumSetup,
	//����������
	NULL,
	//ö�ٱ༭�����
	IStopStdCfg,
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
