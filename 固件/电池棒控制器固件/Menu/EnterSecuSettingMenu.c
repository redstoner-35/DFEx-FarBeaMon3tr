#include "GUI.h"

//进入安全设置处理
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
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&VerifyPassWhenSecuEnter, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"管理员安全验证",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数没有事情要做
	&PasswordEnterInit,
	NULL
	};	
