#include "GUI.h"
#include "Config.h"

//回到设置菜单
void ReturnFromPreIset(void)
	{
	SwitchingMenu(&ChgSysSetMenu);
	}
	
//配置参数
const intEditMenuCfg PreChargeCurrentEdit=
	{
	&CfgData.InputConfig.PreChargeCurrent, //数据源
	100,
	2000, //100-2000mA
	50, //LSB=50mA
	"mA", //毫安
	"寿命",
	"速度",
  &ReturnFromPreIset,
	};
	
//占位函数，在自定义渲染模式下CALL整数编辑菜单
void PreISetMenuDummy(void)
	{
	IntEditHandler(&PreChargeCurrentEdit);
	}
	
const MenuConfigDef PreChargeISetMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&PreISetMenuDummy, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"预充电电流设置",
	NULL,
	NULL, 
	NULL,
	//进入的时候初始化菜单编辑
	&IntEditInitHandler,
	NULL
	};
