#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"
#include "CapTest.h"
#include "LogSystem.h"


//变量
extern bool IsEnablePowerOFF;
extern bool IsEnableAdapEmu;
extern bool EnableOneShotAct;
extern bool EnableManuBal;

//函数声明
void EnterChargeTest(void);
void ManuallyShutSystemOFF(void);
void EnterCTHistory(void);
void ViewColData(void);
void ResetColumGauge(void);
void EnterAbout(void);
void EnterAdapterEmu(void);
void SendTCResetCommand(void);
void SetDisplayDir(void);
void ActOneShotTest(void);
void EnterPSet(void);
void EnterGUIPref(void);
void ViewChipState(void);
void EnterManuBal(void);

//进入高级模式之前输密码
void EnterAdvMode(void)
	{
	SwitchingMenu(&EnterAdvancedMenu);
	}

//菜单项参数
const SetupMenuSelDef EasySetup[19]=
	{	
		{
		"一键充电测容",
		false,
		&AlwaysTrue,
		&EnterChargeTest
		},
		{
		"充放电功率配置",
		false,
		&CfgData.EnableChargPowerConfig,
		&EnterPSet
		},
		{
		"激活一次性测容",
		false,
		&EnableOneShotAct,
		&ActOneShotTest
		},
		{
		"查看历史测容数据",
		false,
		&LastCData.IsDataValid,
		&EnterCTHistory
		},
		{
		"查看库仑计历史数据",
		false,
		&LogHeader.IsRunlogHasContent,
		&ViewColData,
		},
		{
		"重置库仑计数据",
		false,
		&LogHeader.IsRunlogHasContent,
		&ResetColumGauge,
		},		
		{
		"适配器模拟",
		false,
		&IsEnableAdapEmu,			
		&EnterAdapterEmu
		},
		{
		"手动激活均衡",
		false,
		&EnableManuBal,
		&EnterManuBal,
		},
		{
		"Type-C链路重置",
		false,
		&AlwaysTrue,		
		&SendTCResetCommand
		},
		{
		"显示方向设置",
		false,
		&AlwaysTrue,			
		&SetDisplayDir
		},
		{
		"GUI首选项设置",
		false,
		&AlwaysTrue,			
		&EnterGUIPref
		},
		{
		"查看芯片状态",
		false,
		&AlwaysTrue,
		&ViewChipState,
		},
		{
		"关闭系统",
		false,
		&IsEnablePowerOFF,
		&ManuallyShutSystemOFF,
		},		
		{
		"进入高级模式",
		false,
		&AlwaysTrue,
		&EnterAdvMode,
		},		
		{
		"关于",
		false,
		&AlwaysTrue,
		&EnterAbout,
		},
		{
		"\0",
		true,
		&AlwaysTrue,
		NULL,
		}
	};

void BackFromEsetupToMainMenu(void)
	{
	ClearScreen(); //清屏
	WriteConfiguration(&CfgUnion,false); //写入配置
	SwitchingMenu(&MainMenu);
	}	
	
const MenuConfigDef EasySetMainMenu=
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
	"系统设置",
	EasySetup,
	NULL,
	&BackFromEsetupToMainMenu, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};
