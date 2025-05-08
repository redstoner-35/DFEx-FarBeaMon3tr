#include "Config.h"
#include "Key.h"
#include "GUI.h"

const EnumEditEntryDef IStopCfg[8]=
	{
		{
		"200mA",
	  false,
		IStop_200mA,
		false,
		},		
		{
		"250mA",
	  false,
		IStop_250mA,
		false,
		},		
		{
		"300mA",
	  false,
		IStop_300mA,
		false,
		},	
		{
		"350mA",
	  false,
		IStop_350mA,
		false,
		},		
		{
		"400mA",
	  false,
		IStop_400mA,
		false,
		},	
		{
		"450mA",
	  false,
		IStop_450mA,
		false,
		},		
		{
		"0.5A(不推荐)",
	  true,
		IStop_500mA,
		false,
		},			
		{ //占位符
		"",
	  false,
		100,
		true
		}
	};

int ReadIStopEnumValue(void)
	{
	//返回充电功率的enum值
	return (int)CfgData.IStop;
	}
	
void FedIStopEnumValue(int Input)
	{	
	extern bool IsEnable17AMode;
	CfgData.IStop=(IStopConfig)Input;
	if(IsEnable17AMode)
		{
		//2.5mR爆充固件因为停充电流机制导致100和150mA充不满，需要强制设置到200mA
		if(CfgData.IStop==IStop_100mA||CfgData.IStop==IStop_150mA)CfgData.IStop=IStop_200mA;
		}
	SwitchingMenu(&ChgSysSetMenu);
	}

const MenuConfigDef IstopSetMenu=
	{
	MenuType_EnumSetup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	IStopCfg,
  &ReadIStopEnumValue,
  &FedIStopEnumValue,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"停充电流设置",
	NULL,
	NULL,
	NULL, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};
