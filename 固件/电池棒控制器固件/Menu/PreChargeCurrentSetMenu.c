#include "GUI.h"
#include "Config.h"

//�ص����ò˵�
void ReturnFromPreIset(void)
	{
	SwitchingMenu(&ChgSysSetMenu);
	}
	
//���ò���
const intEditMenuCfg PreChargeCurrentEdit=
	{
	&CfgData.InputConfig.PreChargeCurrent, //����Դ
	100,
	2000, //100-2000mA
	50, //LSB=50mA
	"mA", //����
	"����",
	"�ٶ�",
  &ReturnFromPreIset,
	};
	
//ռλ���������Զ�����Ⱦģʽ��CALL�����༭�˵�
void PreISetMenuDummy(void)
	{
	IntEditHandler(&PreChargeCurrentEdit);
	}
	
const MenuConfigDef PreChargeISetMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&PreISetMenuDummy, //��Ⱦ����
	NULL, //��������
	//�������ò˵�����Ҫ�ñ������
	"Ԥ����������",
	NULL,
	NULL, 
	NULL,
	//�����ʱ���ʼ���˵��༭
	&IntEditInitHandler,
	NULL
	};
