#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"

const EnumEditEntryDef PowerCfg[7]=
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
		{
		"100W(烫烫烫)",
	  true,
		Power_100W,
		false,
		},	
		{
		"140W(垃圾电池别用)",
	  true,
		Power_140W,
		false,
		},	
		{ //占位符
		"",
	  false,
		100,
		true
		}
	};

int ReadPWREnumValue(void)
	{
	//判断参数并修正非法参数
	if(CfgData.MaxVPD==PDMaxIN_20V&&CfgData.InputConfig.ChargePower==Power_140W)	
		CfgData.InputConfig.ChargePower=Power_100W;		
	//返回充电功率的enum值	
	return (int)CfgData.InputConfig.ChargePower;
	}
	
void FedPWREnumValue(int Input)
	{
	//返回enum值
	CfgData.InputConfig.ChargePower=(ChargePowerDef)Input;
	//判断参数并修正非法参数
	if(CfgData.MaxVPD==PDMaxIN_20V&&CfgData.InputConfig.ChargePower==Power_140W)	
		CfgData.InputConfig.ChargePower=Power_100W;	
  //返回到对应设置菜单
	if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
	else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
	}

const MenuConfigDef PowerSetMenu=
	{
	MenuType_EnumSetup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	PowerCfg,
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

