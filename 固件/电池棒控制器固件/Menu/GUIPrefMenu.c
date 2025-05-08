#include "GUI.h"
#include "Config.h"


const BoolListEntryDef GUIPrefParam[3]=
	{
		{
		"使能快速自检",
		true,
		&CfgData.EnableFastBoot,
		false,
		false
		},
		{
		"老花眼模式",
		true,
		&CfgData.EnableLargeMenu,
		false,
		false
		},		
		{ //占位符
		"",
		false,
		&AlwaysFalse,
		true,
		false
		}		
	};

void BackFromGUIPref(void)
	{
  //返回到对应设置菜单
	if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
	else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
	}
	
const MenuConfigDef GUIPrefMenu=
	{
	MenuType_BoolListSetup,
	//布尔类的入口
	GUIPrefParam,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"GUI首选项设置",
	NULL,
	NULL,
	&BackFromGUIPref, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};	
