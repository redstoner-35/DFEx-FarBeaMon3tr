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
static bool EnableAutoBal=false;

//函数声明
bool UpdateSinkPower(bool IsForceUpdate);
void ShutSysOFF(void);
void IP2366_ReInitBasedOnConfig(void);
void IP2366_SetIBatLIMBaseOnSysCfg(void);

//检查是否可以关闭系统
void UpdateIfSysCanOFF(void)
	{
	BatteryStateDef State;
	extern bool CurrentStorDisState;
	//Type-C断开连接才允许关机
	IP2366_GetChargerState(&State);
	IsEnablePowerOFF=State==Batt_StandBy?true:false;
	//设置适配器模拟功能是否使能
	if(!DCDCOutputBit)IsEnableAdapEmu=false;
	else if(IsBootFromVBUS)IsEnableAdapEmu=false;     //系统处于安全boot模式，禁止适配器模拟运行
	else if(State==Batt_StandBy)IsEnableAdapEmu=true;
	else if(State==Batt_discharging)IsEnableAdapEmu=true;
	else IsEnableAdapEmu=false;
	//检查PDO设置是否开启
	if(!CfgData.EnablePDOConfig)IsPDOCanConfig=false;
	else if(!CfgData.OutputConfig.IsEnablePDOut)IsPDOCanConfig=false;
	else if(StorageMode!=StorageMode_OFF&&CurrentStorDisState)IsPDOCanConfig=true;  //开启存储模式且系统正在放电时，启动检测
	else if(!DCDCOutputBit)IsPDOCanConfig=false; 
	else IsPDOCanConfig=true;
	//设置是否能配置输出
	if(!CfgData.EnableLVProtectConfig)IsEnableLVConfig=false;
	else if(!DCDCOutputBit)IsEnableLVConfig=false;
	else IsEnableLVConfig=true;
	//启用一次性测容
	if(StorageMode!=StorageMode_OFF)EnableOneShotAct=false;  //存储模式下开启时不允许激活一次性测容
	else EnableOneShotAct=CfgData.InstantCTest==InstantCTest_NotTriggered?true:false;
	//设置是否启用手动均衡	
	if(ADCO.Vbatt<10.1||!IsEnablePowerOFF)EnableManuBal=false;
	else EnableManuBal=true;
  //设置是否启用自动均衡
	if(State==Batt_StandBy||State==Batt_discharging)EnableAutoBal=false;
	else EnableAutoBal=true;
	}
	
//关闭系统
void ManuallyShutSystemOFF(void)
	{
	extern bool IsConfigSaved;
	//配置发生更改，重新初始化
	if(IsConfigSaved||!CheckIfConfigIsSame())
		{
		IP2366_ReInitBasedOnConfig(); //设置芯片配置
		IP2366_SetIBatLIMBaseOnSysCfg(); //设置动态限流
		IsConfigSaved=false;
		}
	//关机之前首先保存配置
	if(CfgData.AutoSaveCfg==AutoSave_Enabled||!CfgData.EnableAdvAccess)WriteConfiguration(&CfgUnion,false);
	IsEnableAdvancedMode=false;
	Balance_ForceDiasble();
	delay_ms(100);
	ShutSysOFF();
	}

//回到主菜单
void ReturnToMainMenu(void)
	{
	bool IsConfigModified=false;
	extern bool IsConfigSaved;
	extern bool IsSinkPowerChanged;
	extern bool IsCPortConnected;
	//检查配置是否发生变化，如果发生变化，则重新初始化芯片应用设置
	if(IsConfigSaved||!CheckIfConfigIsSame())
		{
		IsConfigModified=true;
		IP2366_ReInitBasedOnConfig(); //设置芯片配置
		if(IsSinkPowerChanged)UpdateSinkPower(true);           //退出菜单时强制更新Sink功率
		IP2366_SetIBatLIMBaseOnSysCfg(); //设置动态限流
		}
	//回去之前首先保存配置，然后退出	
	if(CfgData.AutoSaveCfg==AutoSave_Enabled||!CfgData.EnableAdvAccess)WriteConfiguration(&CfgUnion,false);
	IsEnableAdvancedMode=false;
	if(IsConfigModified&&IP2366_GetIfCPortConnected())
		{
		//配置发生更改，重新初始化后弹出拔掉C口的提示
		IsCPortConnected=true;
		SwitchingMenu(&InfoUserRemoveCCableMenu);
		}
	else
		{		
		//配置没有发生变更，不用弹出提示直接回主菜单
		ClearScreen(); 
		SwitchingMenu(&MainMenu);
		}
	//清除flag
	IsSinkPowerChanged=false;
	IsConfigSaved=false;
	}
	
//进入功率设置菜单
void EnterPSet(void)
	{
	SwitchingMenu(&PowerSetMenu);
	}	
	
//进入功率设置菜单
void EnterLVSet(void)
	{
	SwitchingMenu(&LVSetMenu);
	}		

//进入休眠配置
void EnterSleepCfg(void)
	{
	SwitchingMenu(&SleepCfgMenu);
	}	
	
//进入放电系统配置
void EnterDisMgmt(void)
	{
	//特殊固件下解锁HSCP支持
	if(CurrentIP2366FW->IsHSCPCapable)SwitchingMenu(&DisChgCfgMenu);
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

void EnterPDOutCfg(void)	
	{
	SwitchingMenu(&PDOutputSetMenu);
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
	
void EnterStorageModePref(void)
	{
	SwitchingMenu(&StorageModeSetMenu);
	}

void EnterAutoExtBalMenu(void)
	{
	SwitchingMenu(&AutoBALMenu);
	}	
	
void EnterQueryPDOListMenu(void)
	{
	SwitchingMenu(&QueryPDOListMenu);
	}
	
//菜单项参数
const SetupMenuSelDef MainSetup[27]=
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
		"长期存储模式设置",
		false,		
		&AlwaysTrue,
		&EnterStorageModePref
		},
		{
		"系统休眠配置",
		false,
		&AlwaysTrue,
		&EnterSleepCfg
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
		"PD输出配置",
		false,
		&IsPDOCanConfig,
		&EnterPDOutCfg
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
		"激活自动均衡补电",
		false,		
		&EnableAutoBal,
		&EnterAutoExtBalMenu
		},
		{
		"测量系统配置",
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
		"Sink PDO列表查询",
		false,
		&AlwaysTrue,
		&EnterQueryPDOListMenu
		},
		{
		"配置文件管理",
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
