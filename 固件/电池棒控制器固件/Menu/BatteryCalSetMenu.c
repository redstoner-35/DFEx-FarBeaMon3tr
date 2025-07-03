#include "GUI.h"
#include "Config.h"

//回到设置菜单函数声明
void ReturnToTCCalMenu(void);
	
//配置参数
const intEditMenuCfg BattVoltageCal=
	{
	&CfgData.BatteryVoltageCalFactor, //数据源
	800,
	1200, //对应80%-120%原始值
	1, //LSB=0.1%
	"  ", 
	"负偏",
	"正偏",
  &ReturnToTCCalMenu,
	};
	
const intEditMenuCfg BattCurrentCal=
	{
	&CfgData.BatteryCurrentCalFactor, //数据源
	800,
	1200, //对应80%-120%原始值
	1, //LSB=0.1%
	"  ", 
	"负偏",
	"正偏",
  &ReturnToTCCalMenu,
	};	
	
//占位函数，在自定义渲染模式下CALL整数编辑菜单
void BattVCALMenuDummy(void)
	{
	IntEditHandler(&BattVoltageCal);
	}
	
void BattICALMenuDummy(void)
	{
	IntEditHandler(&BattCurrentCal);
	}
	
//菜单输入
const MenuConfigDef BattICALMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&BattICALMenuDummy, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"电池电流校准",
	NULL,
	NULL, 
	NULL,
	//进入的时候初始化菜单编辑
	&IntEditInitHandler,
	NULL
	};
	
//菜单输入
const MenuConfigDef BattVCALMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&BattVCALMenuDummy, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"电池电压校准",
	NULL,
	NULL, 
	NULL,
	//进入的时候初始化菜单编辑
	&IntEditInitHandler,
	NULL
	};
