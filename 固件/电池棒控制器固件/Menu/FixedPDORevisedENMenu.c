#include "GUI.h"
#include "Config.h"

const BoolListEntryDef FixedPDOEditParam[7]=
	{
		{
		"使能9VPDO电流修改",
		true,
		&CfgData.FixedPDOCfg.IsEnable9VPDOSet,
		false,
		false
		},
		{
		"使能12VPDO电流修改",
		true,
		&CfgData.FixedPDOCfg.IsEnable12VPDOSet,
		false,
		false
		},
		{
		"使能15VPDO电流修改",
		true,
		&CfgData.FixedPDOCfg.IsEnable15VPDOSet,
		false,
		false
		},
		{
		"使能20VPDO电流修改",
		true,
		&CfgData.FixedPDOCfg.IsEnable20VPDOSet,
		false,
		false
		},
		{
		"使能PPS1电流修改",
		true,
		&CfgData.PPSConfig.IsEnablePPS1Set,
		false,
		false
		},
		{
		"使能PPS2电流修改",
		true,
		&CfgData.PPSConfig.IsEnablePPS2Set,
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
	
//安全菜单配置
void ReturnFromPPSIset(void);

const MenuConfigDef FixedPDOICCModifyCfgMenu=
	{
	MenuType_BoolListSetup,
	//布尔类的入口
	FixedPDOEditParam,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"PDO电流修改设置",
	NULL,
	NULL,
	&ReturnFromPPSIset, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};
