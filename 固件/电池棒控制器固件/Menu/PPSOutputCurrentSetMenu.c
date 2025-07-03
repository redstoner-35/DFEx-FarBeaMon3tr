#include "GUI.h"
#include "Config.h"

//回到设置菜单
void ReturnFromPPSIset(void)
	{
	SwitchingMenu(&PDOutputSetMenu);
	}
	
//配置参数
const intEditMenuCfg PPS1CurrentEdit=
	{
	&CfgData.PPSConfig.PPS1Current, //数据源
	1500,
	6350, //PPS1可调范围是1500-6350mA
	50, //LSB=50mA
	"mA", //毫安
	"",
	"",
  &ReturnFromPPSIset,
	};
	
const intEditMenuCfg PPS2CurrentEdit=
	{
	&CfgData.PPSConfig.PPS2Current, //数据源
	2000,
	6350, //PPS2可调范围是1500到6350mA
	50, //LSB=50mA
	"mA", //毫安
	"",
	"",
  &ReturnFromPPSIset,
	};	
	
	
//检查数值是否合法
void CheckPPSIsetIsOK(void)
	{
	extern bool IsEnable17AMode;
	//非公版固件允许设置
	if(IsEnable17AMode)return;
	//复位PPS电流
	if(CfgData.PPSConfig.PPS2Current>3000)CfgData.PPSConfig.PPS2Current=3000;
	if(CfgData.PPSConfig.PPS1Current>3000)CfgData.PPSConfig.PPS1Current=3000;
	}	
	
//PPS1电流设置菜单	
void PPS1IsetDummy(void)
	{
	IntEditHandler(&PPS1CurrentEdit);
	}	

const MenuConfigDef PPS1IsetMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&PPS1IsetDummy, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"PPS1电流设置",
	NULL,
	NULL, 
	NULL,
	//进入的时候初始化菜单编辑，退出的时候检查数值
	&IntEditInitHandler,
	&CheckPPSIsetIsOK
	};
	
//PPS1电流设置菜单	
void PPS2IsetDummy(void)
	{
	IntEditHandler(&PPS2CurrentEdit);
	}	

const MenuConfigDef PPS2IsetMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&PPS2IsetDummy, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"PPS2电流设置",
	NULL,
	NULL, 
	NULL,
	//进入的时候初始化菜单编辑，退出的时候检查数值
	&IntEditInitHandler,
	&CheckPPSIsetIsOK
	};
