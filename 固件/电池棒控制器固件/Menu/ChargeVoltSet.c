#include "GUI.h"
#include "Config.h"

//�ص����ò˵�
void ReturnFromVset(void)
	{
	SwitchingMenu(&ChgSysSetMenu);
	}
	
//���ò���
const intEditMenuCfg VFullEdit=
	{
	&CfgData.InputConfig.FullVoltage, //����Դ
	3600,
	4230, //3.6-4.23V
	10, //LSB=10mA
	"mV", //����
	"����",
	"����",
  &ReturnFromVset,
	};
	
//ռλ���������Զ�����Ⱦģʽ��CALL�����༭�˵�
void VSetMenuDummy(void)
	{
	IntEditHandler(&VFullEdit);
	}
	
const MenuConfigDef ChgVSetMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&VSetMenuDummy, //��Ⱦ����
	NULL, //��������
	//�������ò˵�����Ҫ�ñ������
	"��ѹ����ѹ����",
	NULL,
	NULL, 
	NULL,
	//�����ʱ���ʼ���˵��༭
	&IntEditInitHandler,
	NULL
	};
