#include "GUI.h"

void EnterAdvModeProc(void)
	{
	SwitchingMenu(&SetMainMenu);
	IsEnableAdvancedMode=true; //�߼�ģʽ����	
	}

void BackToEasySetup(void)	
	{
	SwitchingMenu(&EasySetMainMenu);
	}
PasswordInputDef EntAdvModeVerify=
	{
	"\x8F\xE8\xB3\x93",
	&EnterAdvModeProc,
	&BackToEasySetup,
	};
	
void VerifyPassWhenAdvMode(void)
	{
	PassWordMenuRender(&EntAdvModeVerify);
	}

const MenuConfigDef EnterAdvancedMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&VerifyPassWhenAdvMode, //��Ⱦ����
	NULL, //��������
	//�������ò˵�����Ҫ�ñ������
	"����߼�ģʽ",
	NULL,
	NULL, 
	NULL,
	//������˳����캯��û������Ҫ��
	&PasswordEnterInit,
	NULL
	};	
