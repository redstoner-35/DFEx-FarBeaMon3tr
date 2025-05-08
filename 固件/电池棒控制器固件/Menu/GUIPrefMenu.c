#include "GUI.h"
#include "Config.h"


const BoolListEntryDef GUIPrefParam[3]=
	{
		{
		"ʹ�ܿ����Լ�",
		true,
		&CfgData.EnableFastBoot,
		false,
		false
		},
		{
		"�ϻ���ģʽ",
		true,
		&CfgData.EnableLargeMenu,
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

void BackFromGUIPref(void)
	{
  //���ص���Ӧ���ò˵�
	if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
	else SwitchingMenu(&SetMainMenu); //�����˳�״̬,����ESC��ص����˵�
	}
	
const MenuConfigDef GUIPrefMenu=
	{
	MenuType_BoolListSetup,
	//����������
	GUIPrefParam,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//������Ⱦ�Ĵ���
	NULL, //��Ⱦ����
	NULL, //��������
	//�����ò˵�
	"GUI��ѡ������",
	NULL,
	NULL,
	&BackFromGUIPref, 
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};	
