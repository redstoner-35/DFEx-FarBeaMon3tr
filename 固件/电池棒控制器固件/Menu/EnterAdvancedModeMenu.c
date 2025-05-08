#include "GUI.h"

void EnterAdvModeProc(void)
	{
	SwitchingMenu(&SetMainMenu);
	IsEnableAdvancedMode=true; //高级模式开启	
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
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&VerifyPassWhenAdvMode, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"进入高级模式",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数没有事情要做
	&PasswordEnterInit,
	NULL
	};	
