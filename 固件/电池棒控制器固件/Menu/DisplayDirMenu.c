#include "LCD_Init.h"
#include "Config.h"
#include "GUI.h"

//函数声明
void SetScreenDirection(void);

const EnumEditEntryDef DirCfg[3]=
	{
		{
		"正常方向显示",
	  true,
		LCDDisplay_Hori_Invert,
		false,
		},
		{
		"反转方向显示",
	  true,
		LCDDisplay_Hori_Normal,
		false,
		},
		{ //占位符
		"",
	  false,
		100,
		true
		}
	};
	
int ReadDisplayEnumValue(void)
	{
	//返回充电功率的enum值
	return (int)CfgData.DisplayDir;
	}
	
void FedDisplayEnumValue(int Input)
	{
	CfgData.DisplayDir=(LCDDisplayDirDef)Input;
	SetScreenDirection(); //重新初始化屏幕
	if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
	else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
	}

const MenuConfigDef DisPlayDirMenu=
	{
	MenuType_EnumSetup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	DirCfg,
  &ReadDisplayEnumValue,
  &FedDisplayEnumValue,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"显示方向配置",
	NULL,
	NULL,
	NULL, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};
