#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"

//函数
void ReturnFromVset(void);

//配置结构体
const BoolListEntryDef LegacySinkCfgParam[4]=
	{
		{
		"PD2.0/3.0协议",
		true,
		&CfgData.SinkConfig.EnableSinkPD,
		false,
		false
		},		
		{
		"华为SCP/HSCP",
		true,
		&CfgData.SinkConfig.EnableSinkSCP,
		false,
		false
		},
		{
		"QC,AFC等老协议",
		true,
		&CfgData.SinkConfig.EnableSinkDPDM,
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

const BoolListEntryDef SinkCfgParam[6]=
	{	
		{
		"PD2.0/3.0协议",
		true,
		&CfgData.SinkConfig.EnableSinkPD,
		false,
		false
		},		
		{
		"华为SCP/HSCP",
		true,
		&CfgData.SinkConfig.EnableSinkSCP,
		false,
		false
		},
		{
		"QC,AFC等老协议",
		true,
		&CfgData.SinkConfig.EnableSinkDPDM,
		false,
		false
		},
		{
		"自动省电功能",
		true,
		&CfgData.EnableAutoPowerSave,
		false,
		false
		},
		{
		"自适应输入功率",
		true,
		&CfgData.EnableSmartSinkPower,
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

//检查Sink配置
void CheckEPRSinkConfig(void)
	{
	//如果2366固件不支持高级Sink配置，则始终禁用超级省电
	if(!CurrentIP2366FW->ExtendedTCSetting)CfgData.EnableAutoPowerSave=false;
	}
	
const MenuConfigDef LegacySinkProtocolMenu=
	{
	MenuType_BoolListSetup,
	//布尔类的入口
	LegacySinkCfgParam,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"输入快充协议设置",
	NULL,
	NULL,
	&ReturnFromVset, 
	//进入和退出构造函数没有事情要做
	NULL,
	&CheckEPRSinkConfig
	};
	
const MenuConfigDef SinkProtocolMenu=
	{
	MenuType_BoolListSetup,
	//布尔类的入口
	SinkCfgParam,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"Sink系统高级设置",
	NULL,
	NULL,
	&ReturnFromVset, 
	//进入和退出构造函数没有事情要做
	NULL,
	&CheckEPRSinkConfig
	};
