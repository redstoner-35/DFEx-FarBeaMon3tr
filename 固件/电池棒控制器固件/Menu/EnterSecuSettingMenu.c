#include "GUI.h"

//���밲ȫ���ô���
void EnterSecuProc(void)
	{
	SwitchingMenu(&SecuCfgMenu);
	}

void LeaveDisMgmtMenu(void);
	
PasswordInputDef EntSecuVerify=
	{
	"\x8F\xE8\xB3\x93",
	&EnterSecuProc,
	&LeaveDisMgmtMenu,
	};
	
void VerifyPassWhenSecuEnter(void)
	{
	PassWordMenuRender(&EntSecuVerify);
	}

const MenuConfigDef EnterSecuMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&VerifyPassWhenSecuEnter, //��Ⱦ����
	NULL, //��������
	//�������ò˵�����Ҫ�ñ������
	"����Ա��ȫ��֤",
	NULL,
	NULL, 
	NULL,
	//������˳����캯��û������Ҫ��
	&PasswordEnterInit,
	NULL
	};	
