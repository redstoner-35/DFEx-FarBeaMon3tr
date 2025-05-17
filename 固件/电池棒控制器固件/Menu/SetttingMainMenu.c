#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"
#include "CapTest.h"
#include "LogSystem.h"
#include "delay.h"
#include "ADC.h"
#include "BalanceMgmt.h"

//变量
bool IsEnablePowerOFF=false;
bool IsEnableAdapEmu=false;
bool IsPDOCanConfig=false;
bool IsEnableLVConfig=false;
bool EnableOneShotAct=false;
bool EnableManuBal=false;

//函数声明
void ShutSysOFF(void);
void IP2366_ReInitBasedOnConfig(void);
void IP2366_SetIBatLIMBaseOnSysCfg(void);

//检查是否可以关闭系统
void UpdateIfSysCanOFF(void)
	{
	BatteryStateDef State;
	//Type-C断开连接才允许关机
	IP2366_GetChargerState(&State);
	IsEnablePowerOFF=State==Batt_StandBy?true:false;
	if(!CfgData.OutputConfig.IsEnableOutput)IsEnableAdapEmu=false;
	if(State==Batt_StandBy)IsEnableAdapEmu=true;
	else if(State==Batt_discharging)IsEnableAdapEmu=true;
	else IsEnableAdapEmu=false;
	//检查PDO设置是否开启
	if(!CfgData.EnablePDOConfig)IsPDOCanConfig=false;
	else if(!CfgData.OutputConfig.IsEnableOutput)IsPDOCanConfig=false;
	else if(!CfgData.OutputConfig.IsEnablePDOut)IsPDOCanConfig=false;
	else IsPDOCanConfig=true;
	//设置是否能配置输出
	if(!CfgData.EnableLVProtectConfig)IsEnableLVConfig=false;
	else if(!CfgData.OutputConfig.IsEnableOutput)IsEnableLVConfig=false;
	else IsEnableLVConfig=true;
	//启用一次性测容
	EnableOneShotAct=CfgData.InstantCTest==InstantCTest_NotTriggered?true:false;
	//设置是否启用手动均衡	
	if(ADCO.Vbatt<10.1||!IsEnablePowerOFF)EnableManuBal=false;
	else EnableManuBal=true;
	}
	
//关闭系统
void ManuallyShutSystemOFF(void)
	{
	WriteConfiguration(&CfgUnion,false);
	IsEnableAdvancedMode=false;
	Balance_ForceDiasble();
	delay_ms(100);
	ShutSysOFF();
	}

//回到主菜单
void ReturnToMainMenu(void)
	{
	//检查配置是否发生变化，如果发生变化，则重新初始化芯片应用设置
	if(!CheckIfConfigIsSame())
		{
		IP2366_ReInitBasedOnConfig(); //设置芯片配置
		IP2366_SetIBatLIMBaseOnSysCfg(); //设置动态限流
		}
	//回去之前首先保存配置，然后退出	
	WriteConfiguration(&CfgUnion,false);
	IsEnableAdvancedMode=false;
	ClearScreen(); //清屏
	SwitchingMenu(&MainMenu);
	}
	
//进入功率设置菜单
void EnterPSet(void)
	{
	//根据配置选择进哪个菜单
	if(CfgData.MaxVPD==PDMaxIN_20V)SwitchingMenu(&PowerSetMenuNoEPR);
	else SwitchingMenu(&PowerSetMenu);
	}	
	
//进入功率设置菜单
void EnterLVSet(void)
	{
	SwitchingMenu(&LVSetMenu);
	}		

//进入放电系统配置
void EnterDisMgmt(void)
	{
	extern bool IsEnableHSCPMode;
	//仅特殊固件
	if(IsEnableHSCPMode)SwitchingMenu(&DisChgCfgMenu);
	else SwitchingMenu(&DisChgCfgMenuNoHSCP);
	}
//进入充电管理
void EnterChgMgmt(void)
	{
	SwitchingMenu(&SafeAlmMenu);
	}	

//进入容量测试
void EnterChargeTest(void)
	{
	SwitchingMenu(&CapTestMenu);
	}
//进入恢复出厂设置
void EnterResetFactory(void)
	{
	SwitchingMenu(&RSTMainMenu);
	}	
//进入测容历史查看
void EnterCTHistory(void)
	{
	SwitchingMenu(&CapTestHisMenu);
	}	
//查看库仑计历史数据
void ViewColData(void)
	{
	SwitchingMenu(&ColHisMenu);
	}
	
void ViewChipState(void)
	{
	SwitchingMenu(&ChipStatMenu);
	}

void EnterPDOConfig(void)	
	{
	SwitchingMenu(&PDOCfgMenu);
	}

void EnterSecuCfg(void)
	{
	SwitchingMenu(&EnterSecuMenu);
	}	
	
void EnterAbout(void)
	{
	SwitchingMenu(&AboutMenu);
	}	
void EnterTset(void)
	{
	SwitchingMenu(&TSetMenu);
	}
void EnterAdapterEmu(void)
	{
	SwitchingMenu(&AdapterEmuMenu);
	}

void SendTCResetCommand(void)
	{
  SwitchingMenu(&TCResetMenu);
	}

void SetDisplayDir(void)
	{
	SwitchingMenu(&DisPlayDirMenu);
	}	
	
void ActOneShotTest(void)
	{
	SwitchingMenu(&ActOneShotCTestMenu);
	}
	
void EnterGUIPref(void)
	{
	SwitchingMenu(&GUIPrefMenu);
	}	
	
void EnterBalCfg(void)
	{
	SwitchingMenu(&BalSysSetMenu);
	}	
	
void EnterManuBal(void)
	{
	SwitchingMenu(&BALTestMenu);
	}
	
void EnterTypeCGaugeConfig(void)
	{
	SwitchingMenu(&TypeCGaugeSetMenu);
	}	
	
//菜单项参数
const SetupMenuSelDef MainSetup[23]=
	{
		{
		"系统安全设置",
		false,
		&AlwaysTrue,
		&EnterSecuCfg
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
		"过热保护温度设置",
		false,
		&CfgData.EnableOTPConfig,
		&EnterTset
		},
		{
		"充放电功率配置",
		false,
		&CfgData.EnableChargPowerConfig,
		&EnterPSet
		},
		{
		"主动均衡设置",
		false,
		&AlwaysTrue,
		&EnterBalCfg
		},
		{
		"放电系统配置",
		false,
		&CfgData.EnableDischargeConfig,
		&EnterDisMgmt
		},
		{
		"PDO广播配置",
		false,
		&IsPDOCanConfig,
		&EnterPDOConfig
		},
		{
		"充电系统配置",
		false,
		&CfgData.EnableChargeConfig,
		&EnterChgMgmt
		},
		{
		"放电低压保护配置",
		false,
		&IsEnableLVConfig,
		&EnterLVSet,
		},	
		{
		"查看历史测容数据",
		false,
		&LastCData.IsDataValid,
		&EnterCTHistory
		},
		{
		"适配器模拟",
		false,
		&IsEnableAdapEmu,			
		&EnterAdapterEmu
		},
		{
		"激活一次性测容",
		false,
		&EnableOneShotAct,
		&ActOneShotTest
		},
		{
		"一键充电测容",
		false,
		&AlwaysTrue,
		&EnterChargeTest
		},
		{
		"查看库仑计历史数据",
		false,
		&LogHeader.IsRunlogHasContent,
		&ViewColData,
		},
		{
		"手动激活均衡",
		false,
		&EnableManuBal,
		&EnterManuBal,
		},
		{
		"TypeC功率计配置",
		false,
		&CfgData.EnableTCCalibration,
		&EnterTypeCGaugeConfig
		},
		{
		"查看芯片状态",
		false,
		&AlwaysTrue,
		&ViewChipState,
		},
		{
		"恢复出厂设置",
		false,
		&AlwaysTrue,
		&EnterResetFactory,
		},		
		{
		"关闭系统",
		false,
		&IsEnablePowerOFF,
		&ManuallyShutSystemOFF,
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

const MenuConfigDef SetMainMenu=
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
	"系统设置（开发者）",
	MainSetup,
	NULL,
	&ReturnToMainMenu, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};	
