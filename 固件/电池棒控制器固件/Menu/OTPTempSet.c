#include "GUI.h"
#include "Config.h"

void BackFromTset(void)
	{
	SwitchingMenu(&SetMainMenu);
	}

//配置参数
const intEditMenuCfg TProtEdit=
	{
	&CfgData.OverHeatLockTemp, //数据源
	80,
	105, //80-105度
	1, //LSB=1度
	"℃", //摄氏度
	"保守",
	"激进",
  &BackFromTset,
	};
	
//占位函数，在自定义渲染模式下CALL整数编辑菜单
void TSetMenuDummy(void)
	{
	IntEditHandler(&TProtEdit);
	}
	
const MenuConfigDef TSetMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&TSetMenuDummy, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"过热保护温度设置",
	NULL,
	NULL, 
	NULL,
	//进入的时候初始化菜单编辑
	&IntEditInitHandler,
	NULL
	};
