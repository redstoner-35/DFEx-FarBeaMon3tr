#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"

const EnumEditEntryDef PowerCfgNoEPR[5]=
	{
		{
		"30W(12V2.5A)",
	  false,
		Power_30W,
		false,
		},
		{
		"45W(15V3A)",
	  false,
		Power_45W,
		false,
		},
		{
		"60W(20V3A)",
	  false,
		Power_60W,
		false,
		},	
		{
		"65W(20V3.25A)",
	  false,
		Power_65W,
		false,
		},		
		{ //占位符
		"",
	  false,
		100,
		true
		}
	};

//函数声明
int ReadPWREnumValue(void);
void FedPWREnumValue(int Input);

const MenuConfigDef PowerSetMenuNoEPR=
	{
	MenuType_EnumSetup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	PowerCfgNoEPR,
  &ReadPWREnumValue,
  &FedPWREnumValue,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"充放电功率配置",
	NULL,
	NULL,
	NULL, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};	
