#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"

const BoolListEntryDef DisParam[6]=
	{
		{
		"放电系统总开关",
		true,
		&CfgData.OutputConfig.IsEnableOutput,
		false,
		true
		},
		{
		"PD协议",
		true,
		&CfgData.OutputConfig.IsEnablePDOut,
		false,
		false
		},		
		{
		"SCP(华为25W)",
		true,
		&CfgData.OutputConfig.IsEnableSCPOut,
		false,
		false
		},
		{
		"QC,AFC等老协议",
		true,
		&CfgData.OutputConfig.IsEnableDPDMOut,
		false,
		false
		},		
		{
		"高功率SCP(100W)",
		true,
		&CfgData.OutputConfig.IsEnableHSCPOut,
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

	
const BoolListEntryDef DisParamNoHSCP[5]=
	{
		{
		"放电系统总开关",
		true,
		&CfgData.OutputConfig.IsEnableOutput,
		false,
		true
		},
		{
		"PD协议",
		true,
		&CfgData.OutputConfig.IsEnablePDOut,
		false,
		false
		},		
		{
		"SCP(华为25W)",
		true,
		&CfgData.OutputConfig.IsEnableSCPOut,
		false,
		false
		},
		{
		"QC,AFC等老协议",
		true,
		&CfgData.OutputConfig.IsEnableDPDMOut,
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
	
void LeaveDisMgmtMenu(void)
	{
	SwitchingMenu(&SetMainMenu);
	}

	
const MenuConfigDef DisChgCfgMenuNoHSCP=
	{
	MenuType_BoolListSetup,
	//布尔类的入口
	DisParamNoHSCP,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"放电系统配置",
	NULL,
	NULL,
	&LeaveDisMgmtMenu, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};
	
const MenuConfigDef DisChgCfgMenu=
	{
	MenuType_BoolListSetup,
	//布尔类的入口
	DisParam,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"放电系统配置",
	NULL,
	NULL,
	&LeaveDisMgmtMenu, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};	
