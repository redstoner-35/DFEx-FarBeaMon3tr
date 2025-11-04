#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"

const EnumEditEntryDef PreChargeEndCfg[6]=
	{
		{
		"2800mV(2.8V)",
	  false,
		VLow_2V8,
		false,
		},
		{
		"2900mV(2.9V)",
	  false,
		VLow_2V9,
		false,
		},
		{
		"3000mV(3.0V)",
	  false,
		VLow_3V0,
		false,
		},
		{
		"3100mV(3.1V)",
	  false,
		VLow_3V1,
		false,
		},
		{
		"3200mV(3.2V)",
	  false,
		VLow_3V2,
		false,
		},		
		{ //占位符
		"",
	  false,
		100,
		true
		}
	};

int ReadPreChargeEnumValue(void)
	{
	//非法的参数值，返回2V8
  if(CfgData.PreChargeEndVoltage<VLow_2V8)return (int)VLow_2V8;	
	//返回充电功率的enum值
	return (int)CfgData.PreChargeEndVoltage;
	}
	
void FedPreChargeEnumValue(int Input)
	{
	CfgData.PreChargeEndVoltage=(VBatLowDef)Input;
	if(CfgData.PreChargeEndVoltage<VLow_2V8)
		CfgData.PreChargeEndVoltage=VLow_2V8; //涓流结束电压不允许设置低于2.8V
	SwitchingMenu(&ChgSysSetMenu);
	}

const MenuConfigDef PreChargeEndSetMenu=
	{
	MenuType_EnumSetup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	PreChargeEndCfg,
  &ReadPreChargeEnumValue,
  &FedPreChargeEnumValue,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"预充电结束电压设置",
	NULL,
	NULL,
	NULL, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};	

