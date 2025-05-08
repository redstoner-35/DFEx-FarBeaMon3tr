#include "GUI.h"
#include "Config.h"

const BoolListEntryDef SecuParam[10]=
	{
		{
		"高级菜单直接访问",
		true,
		&CfgData.EnableAdvAccess,
		false,
		false
		},
		{
		"使能过热降额功能",
		true,
		&CfgData.EnableThermalStepdown,
		false,
		false
		},
		{
		"使能充电系统配置",
		true,
		&CfgData.EnableChargeConfig,
		false,
		false
		},		
		{
		"使能放电系统配置",
		true,
		&CfgData.EnableDischargeConfig,
		false,
		false
		},		
		{
		"使能低压保护配置",
		true,
		&CfgData.EnableLVProtectConfig,
		false,
		false
		},	
		{
		"使能充放功率配置",
		true,
		&CfgData.EnableChargPowerConfig,
		false,
		false
		},	
		{
		"使能输出广播配置",
		true,
		&CfgData.EnablePDOConfig,
		false,
		false
		},	
		{
		"使能过热保护配置",
		true,
		&CfgData.EnableOTPConfig,
		false,
		false
		},	
		{
		"使能高精度功率计",
		true,
		&CfgData.EnableHPGauge,
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
void LeaveDisMgmtMenu(void);

const MenuConfigDef SecuCfgMenu=
	{
	MenuType_BoolListSetup,
	//布尔类的入口
	SecuParam,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"系统安全设置",
	NULL,
	NULL,
	&LeaveDisMgmtMenu, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};	
