#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"
#include "CapTest.h"

//决定是否可以读取
bool IsEnableFactoryReset=false;
bool IsEnableUndoChanges=false;

//计算是否需要恢复出厂
void CalcIfNeedToReset(void)
	{
	unsigned int CurrentCRC;
	extern bool UsingBackupConfig;
	CfgUnionDef buf;
	//计算现在配置的CRC
	CurrentCRC=CalcROMCRC32(&CfgUnion);
	LoadDefaultConfig(&buf,false);
	SyncUnResetThings(&buf);
	if(CurrentCRC!=CalcROMCRC32(&buf))IsEnableFactoryReset=true;
	else IsEnableFactoryReset=false;
	//从ROM内读取配置
	ReadConfiguration(&buf,UsingBackupConfig);
	if(CurrentCRC!=CalcROMCRC32(&buf))IsEnableUndoChanges=true;
	else IsEnableUndoChanges=false;
	
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
	
void UpdateBCfg(void)
	{
	SwitchingMenu(&PSWDVerifyBeforeUpdateBCFGMenu);
	}	
	
void LoadBCfg(void)
	{
	SwitchingMenu(&PSWDVerifyBeforeRestoreBCFGMenu);
	}
	
void UndoChanges(void)
	{
	SwitchingMenu(&DiscardCurrentPendingChangesMenu);
	}	
	
void SaveChanges(void)
	{
	SwitchingMenu(&SaveSystemSettingMenu);
	}	
	
void AutoSaveConfig(void)
	{
	SwitchingMenu(&AutoSaveCfgMenu);
	}
	
void DataBaseCheck(void)
	{
	SwitchingMenu(&DatabaseCheckMenu);
	}
	
//菜单项参数
const SetupMenuSelDef RSTSetup[10]=
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
		"自动存盘设置",
		false,
		&AlwaysTrue,		
		&AutoSaveConfig
		},
		{
		"撤销当前更改",
		false,
		&IsEnableUndoChanges,		
		&UndoChanges
		},
		{
		"保存当前更改",
		false,
		&IsEnableUndoChanges,		
		&SaveChanges
		},
		{
		"更新备用配置",
		false,
		&AlwaysTrue,
		&UpdateBCfg,
		},
		{
		"恢复备用配置",
		false,
		&AlwaysTrue,
		&LoadBCfg,
		},	
		{
		"日志数据库校验",
		false,
		&AlwaysTrue,
		&DataBaseCheck		
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
	"配置文件管理",
	RSTSetup,
	NULL,
	&ReturnFromRSTMenu, 
	//进入的时候需要计算下当前配置是不是出厂设置
	&CalcIfNeedToReset,
	NULL
	};	
