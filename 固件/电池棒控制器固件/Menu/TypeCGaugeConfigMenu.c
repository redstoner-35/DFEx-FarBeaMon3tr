#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"

//进入功率计电压电流校准的地方
void EnterTCICAL(void)
	{
	SwitchingMenu(&TypeICALMenu);
	}
	
void EnterTCVCAL(void)
	{
	SwitchingMenu(&TypeCVCALMenu);
	}

void EnterDsourceSel(void)
	{
	SwitchingMenu(&TypeCCgaugeDSourceMenu);
	}	
	
//菜单项参数
const SetupMenuSelDef TCGaugeSetup[4]=
	{
		{
		"TypeC功率数据源",
		false,
		&AlwaysTrue,
		&EnterDsourceSel
		},
		{
		"功率计电压校准",
		false,
		&AlwaysTrue,
		&EnterTCVCAL
		},
		{
		"功率计电流校准",
		false,
		&AlwaysTrue,
		&EnterTCICAL
		},
		{
		"\0",
		true,
		&AlwaysTrue,
		NULL,
		}
	};

//外部函数声明
void ReturnToMainSetMenu(void);
	
const MenuConfigDef TypeCGaugeSetMenu=
	{
	MenuType_Setup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"TypeC功率计配置",
	TCGaugeSetup,
	NULL,
	&ReturnToMainSetMenu, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};	
