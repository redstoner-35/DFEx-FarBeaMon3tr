#include "GUI.h"
#include "Config.h"

//�ص����ò˵�
void ReturnToTCCalMenu(void)
	{
	SwitchingMenu(&TypeCGaugeSetMenu);
	}
	
//���ò���
const intEditMenuCfg VoltageCal=
	{
	&CfgData.TypeCVoltageCal, //����Դ
	500,
	1500, //��Ӧ50%-150%ԭʼֵ
	1, //LSB=0.1%
	"  ", 
	"��ƫ",
	"��ƫ",
  &ReturnToTCCalMenu,
	};
	
const intEditMenuCfg AmpCal=
	{
	&CfgData.TypeCAmpereCal, //����Դ
	500,
	1500, //��Ӧ50%-150%ԭʼֵ
	1, //LSB=0.1%
	"  ", 
	"��ƫ",
	"��ƫ",
  &ReturnToTCCalMenu,
	};	
	
//ռλ���������Զ�����Ⱦģʽ��CALL�����༭�˵�
void VCALMenuDummy(void)
	{
	IntEditHandler(&VoltageCal);
	}
	
void ICALMenuDummy(void)
	{
	IntEditHandler(&AmpCal);
	}
	
//�˵�����
const MenuConfigDef TypeICALMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&ICALMenuDummy, //��Ⱦ����
	NULL, //��������
	//�������ò˵�����Ҫ�ñ������
	"TypeC����У׼",
	NULL,
	NULL, 
	NULL,
	//�����ʱ���ʼ���˵��༭
	&IntEditInitHandler,
	NULL
	};
	
//�˵�����
const MenuConfigDef TypeCVCALMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&VCALMenuDummy, //��Ⱦ����
	NULL, //��������
	//�������ò˵�����Ҫ�ñ������
	"TypeC��ѹУ׼",
	NULL,
	NULL, 
	NULL,
	//�����ʱ���ʼ���˵��༭
	&IntEditInitHandler,
	NULL
	};
