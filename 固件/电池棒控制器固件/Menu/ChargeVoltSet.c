#include "GUI.h"
#include "Config.h"

//回到设置菜单
void ReturnFromVset(void)
	{
	SwitchingMenu(&ChgSysSetMenu);
	}
	
//配置参数
const intEditMenuCfg VFullEdit=
	{
	&CfgData.InputConfig.FullVoltage, //数据源
	3600,
	4230, //3.6-4.23V
	10, //LSB=10mA
	"mV", //毫伏
	"寿命",
	"续航",
  &ReturnFromVset,
	};
	
//占位函数，在自定义渲染模式下CALL整数编辑菜单
void VSetMenuDummy(void)
	{
	IntEditHandler(&VFullEdit);
	}
	
const MenuConfigDef ChgVSetMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&VSetMenuDummy, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"恒压充电电压设置",
	NULL,
	NULL, 
	NULL,
	//进入的时候初始化菜单编辑
	&IntEditInitHandler,
	NULL
	};
