#include "GUI.h"
#include "Config.h"

//回到设置菜单
void ReturnFromIset(void)
	{
	SwitchingMenu(&ChgSysSetMenu);
	}
	
//配置参数
const intEditMenuCfg ChargeCurrentEdit=
	{
	&CfgData.InputConfig.ChargeCurrent, //数据源
	3000,
	9700, //3000-9700mA(公版芯片)
	100, //LSB=100mA
	"mA", //毫安
	"保守",
	"激进",
  &ReturnFromIset,
	};
	
const intEditMenuCfg ChargeCurrentEditBeastMode=
	{
	&CfgData.InputConfig.ChargeCurrent, //数据源
	3000,
	IP2366_ICCMAX, //3000芯片所允许的最高电流(非公版芯片野兽模式)
	100, //LSB=100mA
	"mA", //毫安
	"保守",
	"激进",
  &ReturnFromIset,
	};	
	
	
//检查数值是否合法
void CheckILimitIsOK(void)
	{
	extern bool IsEnable17AMode;
	if((!IsEnable17AMode||CfgData.MaxVPD==PDMaxIN_20V)&&CfgData.InputConfig.ChargeCurrent>9700)
		CfgData.InputConfig.ChargeCurrent=9700; //公版板子和固件条件下芯片最大只能到9.7A峰值
	}	
	
//占位函数，在自定义渲染模式下CALL整数编辑菜单
void ISetMenuDummy(void)
	{
	extern bool IsEnable17AMode;
	if(!IsEnable17AMode||CfgData.MaxVPD==PDMaxIN_20V)IntEditHandler(&ChargeCurrentEdit);
	else IntEditHandler(&ChargeCurrentEditBeastMode);
	}
	
const MenuConfigDef IChargeSetMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&ISetMenuDummy, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"电池峰值电流设置",
	NULL,
	NULL, 
	NULL,
	//进入的时候初始化菜单编辑，退出的时候检查数值
	&IntEditInitHandler,
	&CheckILimitIsOK
	};
