#include "GUI.h"
#include "Config.h"
#include <string.h>

//回到设置菜单
void ReturnFromIset(void)
	{
	SwitchingMenu(&ChgSysSetMenu);
	}

//内部变量
static intEditMenuCfg IBatPeakEdit;	
static const char IBatPeakEditMaxStr[]={"激进\0"};	
static const char IBatPeakEditMinStr[]={"保守\0"};	
static const char IBatUnitStr[]={"mA\0"};	
	
//检查数值是否合法
void CheckILimitIsOK(void)
	{
  if(!CurrentIP2366FW->IsHyperChargeCapable)CfgData.InputConfig.ChargeCurrent=9700; //不支持超充模式，9700 Max
	if(CfgData.InputConfig.ChargeCurrent>CurrentIP2366FW->IP2366ICCMAX)
		CfgData.InputConfig.ChargeCurrent=CurrentIP2366FW->IP2366ICCMAX; //不允许填入的电流超过9700
	}	
	
//占位函数，在自定义渲染模式下CALL整数编辑菜单
void ISetMenuDummy(void)
	{
  IntEditHandler(&IBatPeakEdit);
	}
	
//初始化充电峰值电流函数
void ChargePowerDispInit(void)
	{
	//准备数值编辑结构体
	IBatPeakEdit.MaxName=(char *)IBatPeakEditMaxStr;
	if(!CurrentIP2366FW->IsHyperChargeCapable)IBatPeakEdit.max=9700;
	else IBatPeakEdit.max=CurrentIP2366FW->IP2366ICCMAX; //指定max值为芯片ICCMAX
	IBatPeakEdit.min=3000;
	IBatPeakEdit.MinName=(char *)IBatPeakEditMinStr;
	IBatPeakEdit.Source=&CfgData.InputConfig.ChargeCurrent;	
	IBatPeakEdit.Step=100;	
	IBatPeakEdit.ThingsToDoWhenExit=&ReturnFromIset;
	IBatPeakEdit.Unit=(char *)IBatUnitStr;
	//初始化编辑器
	IntEditInitHandler();
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
	&ChargePowerDispInit,
	&CheckILimitIsOK
	};
