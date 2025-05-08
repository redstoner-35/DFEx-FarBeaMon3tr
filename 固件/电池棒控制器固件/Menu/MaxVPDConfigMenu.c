#include "Config.h"
#include "Key.h"
#include "GUI.h"

const EnumEditEntryDef MaxVPDCfg[3]=
	{
		{
		"PD3.0(20V)",
	  false,
		PDMaxIN_20V,
		false,
		},
		{
		"PD3.1-EPR(28V)",
	  false,
		PDMaxIN_28V,
		false,
		},
		{ //占位符
		"",
	  false,
		100,
		true
		}
	};

int ReadMaxVPDEnumValue(void)
	{
	//返回充电功率的enum值
	return (int)CfgData.MaxVPD;
	}
	
void FedMaxVPDEnumValue(int Input)
	{
	extern bool IsEnable17AMode;
	MaximumPDVoltageDef buf;
	//收取结果	
	if(IsEnable17AMode)buf=(MaximumPDVoltageDef)Input;
	else buf=PDMaxIN_20V; //普通固件禁止超充
	//启用超充模式	
	if(CfgData.MaxVPD!=buf&&buf==PDMaxIN_28V)
		{
		CfgData.InputConfig.ChargeCurrent=IP2366_ICCMAX;
		CfgData.OverHeatLockTemp=100; //超充发热巨大需要调高温度
		CfgData.InputConfig.ChargePower=Power_140W;
    CfgData.MaxVPD=PDMaxIN_28V;
		WriteConfiguration(&CfgUnion,false);	
		}
	//更新结果
	CfgData.MaxVPD=buf;
	//如果配置为20V Max则关闭EPR
	if(CfgData.MaxVPD==PDMaxIN_20V&&CfgData.InputConfig.ChargePower==Power_140W)	
		{
		CfgData.OverHeatLockTemp=90; //普通模式调低温度
		if(CfgData.InputConfig.ChargeCurrent>9700)CfgData.InputConfig.ChargeCurrent=9700;
		CfgData.InputConfig.ChargePower=Power_100W;
		WriteConfiguration(&CfgUnion,false);
		}		
	SwitchingMenu(&ChgSysSetMenu);
	}

const MenuConfigDef MaxVPDMenu=
	{
	MenuType_EnumSetup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	MaxVPDCfg,
  &ReadMaxVPDEnumValue,
  &FedMaxVPDEnumValue,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"最高充放电压设置",
	NULL,
	NULL,
	NULL, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};
