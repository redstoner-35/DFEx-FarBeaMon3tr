#include "GUI.h"
#include "Config.h"

void BackFromTset(void)
	{
	SwitchingMenu(&SetMainMenu);
	}

//���ò���
const intEditMenuCfg TProtEdit=
	{
	&CfgData.OverHeatLockTemp, //����Դ
	80,
	105, //80-105��
	1, //LSB=1��
	"��", //���϶�
	"����",
	"����",
  &BackFromTset,
	};
	
//ռλ���������Զ�����Ⱦģʽ��CALL�����༭�˵�
void TSetMenuDummy(void)
	{
	IntEditHandler(&TProtEdit);
	}
	
const MenuConfigDef TSetMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&TSetMenuDummy, //��Ⱦ����
	NULL, //��������
	//�������ò˵�����Ҫ�ñ������
	"���ȱ����¶�����",
	NULL,
	NULL, 
	NULL,
	//�����ʱ���ʼ���˵��༭
	&IntEditInitHandler,
	NULL
	};
