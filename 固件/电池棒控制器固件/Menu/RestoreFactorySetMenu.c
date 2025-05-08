#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"
#include "CapTest.h"

//决定是否可以读取
bool IsEnableFactoryReset=false;

//计算是否需要恢复出厂
void CalcIfNeedToReset(void)
	{
	unsigned int CurrentCRC;
	CfgUnionDef buf;
	//计算现在配置的CRC
	CurrentCRC=CalcROMCRC32(&CfgUnion);
	LoadDefaultConfig(&buf);
	if(CurrentCRC!=CalcROMCRC32(&buf))IsEnableFactoryReset=true;
	else IsEnableFactoryReset=false;
	}

//回到主菜单
void ReturnFromRSTMenu(void)
	{
	SwitchingMenu(&SetMainMenu);
	}

void ResetCTest(void)
	{
	SwitchingMenu(&ResetCTestMenu);
	}	

void ResetSysCfg(void)
	{
	SwitchingMenu(&ResetSysConfigtMenu);
	}	
	
void ResetColumGauge(void)
	{
	SwitchingMenu(&ResetColMenu);
	}	
	
//菜单项参数
const SetupMenuSelDef RSTSetup[4]=
	{
		{
		"重置系统设置",
		false,
		&IsEnableFactoryReset,
		&ResetSysCfg,
		},
		{
		"重置测容系统数据",
		false,
		&LastCData.IsDataValid,
		&ResetCTest,
		},	
		{
		"重置库仑计数据",
		false,
		&AlwaysTrue,
		&ResetColumGauge,
		},	
		{
		"\0",
		true,
		&AlwaysTrue,
		NULL,
		}		
	};
	
//菜单主参数	
const MenuConfigDef RSTMainMenu=
	{
	MenuType_Setup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"重置系统设置",
	RSTSetup,
	NULL,
	&ReturnFromRSTMenu, 
	//进入的时候需要计算下当前配置是不是出厂设置
	&CalcIfNeedToReset,
	NULL
	};	
