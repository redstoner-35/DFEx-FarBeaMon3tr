#include "GUI.h"
#include "Config.h"

//回到设置菜单
void ReturnToTCCalMenu(void)
	{
	SwitchingMenu(&TypeCGaugeSetMenu);
	}
	
//配置参数
const intEditMenuCfg VoltageCal=
	{
	&CfgData.TypeCVoltageCal, //数据源
	500,
	1500, //对应50%-150%原始值
	1, //LSB=0.1%
	"  ", 
	"负偏",
	"正偏",
  &ReturnToTCCalMenu,
	};
	
const intEditMenuCfg AmpCal=
	{
	&CfgData.TypeCAmpereCal, //数据源
	500,
	1500, //对应50%-150%原始值
	1, //LSB=0.1%
	"  ", 
	"负偏",
	"正偏",
  &ReturnToTCCalMenu,
	};	
	
//占位函数，在自定义渲染模式下CALL整数编辑菜单
void VCALMenuDummy(void)
	{
	IntEditHandler(&VoltageCal);
	}
	
void ICALMenuDummy(void)
	{
	IntEditHandler(&AmpCal);
	}
	
//菜单输入
const MenuConfigDef TypeICALMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&ICALMenuDummy, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"TypeC电流校准",
	NULL,
	NULL, 
	NULL,
	//进入的时候初始化菜单编辑
	&IntEditInitHandler,
	NULL
	};
	
//菜单输入
const MenuConfigDef TypeCVCALMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&VCALMenuDummy, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"TypeC电压校准",
	NULL,
	NULL, 
	NULL,
	//进入的时候初始化菜单编辑
	&IntEditInitHandler,
	NULL
	};
