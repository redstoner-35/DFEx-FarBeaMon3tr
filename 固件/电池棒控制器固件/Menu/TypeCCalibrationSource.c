#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"

const EnumEditEntryDef TypeCDSCfg[3]=
	{
		{
		"IP2366(低精度)",
	  true,
		0,
		false,
		},
		{
		"INA226(高精度)",
	  true,
		1,
		false,
		},
		{ //占位符
		"",
	  false,
		100,
		true
		}
	};

int ReadTCSourceEnumValue(void)
	{		
	//返回充电功率的enum值	
		return CfgData.EnableHPGauge?1:0;
	}
	
void FedTCSourceEnumValue(int Input)
	{
  //设置对应功率值
	if(Input)CfgData.EnableHPGauge=true;
	else CfgData.EnableHPGauge=false;
  //返回到对应设置菜单
  SwitchingMenu(&TypeCGaugeSetMenu); //处于退出状态,按下ESC后回到Type-C校准菜单
	}

const MenuConfigDef TypeCCgaugeDSourceMenu=
	{
	MenuType_EnumSetup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	TypeCDSCfg,
  &ReadTCSourceEnumValue,
  &FedTCSourceEnumValue,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"TypeC功率数据源",
	NULL,
	NULL,
	NULL, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};	




