#include "GUI.h"
#include "Config.h"

//函数声明
void ReturnToTCCalMenu(void);
void LoadBattCalibrationData(void);

//配置参数
const intEditMenuCfg SysTempCal=
	{
	&CfgData.SystemTempCalFactor, //数据源
	-200,
	200, //施加-20摄氏度到20摄氏度的偏移量修正温度检测差异
	1, 	 //LSB=0.1℃
	"  ", 
	"负偏",
	"正偏",
  &ReturnToTCCalMenu,
	};
	
	//占位函数，在自定义渲染模式下CALL整数编辑菜单
void SysTCALMenuDummy(void)
	{
	IntEditHandler(&SysTempCal);
	}
	
//菜单输入
const MenuConfigDef SysTCALMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&SysTCALMenuDummy, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"系统温度校准",
	NULL,
	NULL, 
	NULL,
	//进入的时候初始化菜单编辑
	&IntEditInitHandler,
	&LoadBattCalibrationData
	};
