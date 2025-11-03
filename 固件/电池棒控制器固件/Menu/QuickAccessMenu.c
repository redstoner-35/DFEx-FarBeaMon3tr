#include "GUI.h"
#include "Config.h"
#include "delay.h"
#include "LogSystem.h"
#include "ADC.h"
#include "IP2366_REG.h"

//内部变量
bool IsEnableTempChargeOnly=false;
static bool TempChargeConfig=false;
static bool IsTempEnableStorageMode=false;
static bool ClearOCFlag=false;
bool IsEnterDischargeMode=false;

const BoolListEntryDef QuickAccessParam[5]=
	{
		{
		"仅充电模式",
		true,
		&TempChargeConfig,
		false,
		false
		},
		{
		"存储模式",
		true,
		&IsTempEnableStorageMode,
		false,
		false
		},
		{
		"仅放电模式",
		true,
		&IsEnterDischargeMode,
		false,
		false
		},
		{
		"解除故障保护",
		true,
		&ClearOCFlag,
		false,
		false			
		},
		{ //占位符
		"",
		false,
		&AlwaysFalse,
		true,
		false
		}		
	};

//内部函数，判断临时充电使能本来的状态
static bool GetTempChargeModeStatus(void)
	{
	//仅充电模式开启
	if(IsEnableTempChargeOnly)return true;
	//仅充电模式bit复位，但是当前DCDC处于关闭状态,标记为开启
	if(!DCDCOutputBit)return true;
	//其余状态默认关闭
	return false;
	}	

//进入快捷菜单前根据当前设置应用按钮配置	
void EnterQuickMenu(void)	
	{
	ClearOCFlag=false;
	IsEnterDischargeMode=false;
	TempChargeConfig=GetTempChargeModeStatus();
	IsTempEnableStorageMode=StorageMode==StorageMode_OFF?false:true;
	}

//菜单配置
void IP2366_ReInitBasedOnConfig(void);
void IP2366_SetIBatLIMBaseOnSysCfg(void);
		
void ReturnFromQuickAccMenu(void)
	{
	extern ChipStatDef CState;
	bool IsNeedToReConfig=false;
	bool IsNeedToClearOCF;
	extern bool IsCPortConnected;
	bool CurrentStorMode=StorageMode==StorageMode_OFF?false:true;
	//进行临时充电操作的比对
	if(TempChargeConfig!=GetTempChargeModeStatus())
		{
		//按钮为关闭状态，执行关闭仅充电的判断
		if(!TempChargeConfig)
			{
			IsEnableTempChargeOnly=false;
			//配置系统内DCDC输出为关闭模式，使能输出bit临时启用充电输出
			if(!CfgData.OutputConfig.IsEnableOutput)DCDCOutputBit=true;
			}
		//按钮为开启状态，执行开启仅充电的判断
		else
			{			
			IsEnableTempChargeOnly=true;
			//如果配置系统内的DCDC输出bit为关闭状态但是输出开启，则除能输出bit
			if(!CfgData.OutputConfig.IsEnableOutput&&DCDCOutputBit)DCDCOutputBit=false;
			}
		//模式状态发生变更，需要重新初始化IC
		IsNeedToReConfig=true;
		}
	//进行存储模式的比对
	if(IsTempEnableStorageMode!=CurrentStorMode)
		{
		//临时关闭存储模式
		if(!IsTempEnableStorageMode)StorageMode=StorageMode_OFF;
		//重新打开存储模式
		else StorageMode=CfgData.StorageModeINROM;
		//模式状态发生变更，需要重新初始化IC
		IsNeedToReConfig=true;
		}
	//检查配置是否发生变化，如果发生变化，则重新初始化芯片应用设置
	if(IsNeedToReConfig)
		{
		IP2366_ReInitBasedOnConfig(); //设置芯片配置
		IP2366_SetIBatLIMBaseOnSysCfg(); //设置动态限流
		}	
	//进行故障保护清除操作
	if(CState.VSysState!=VSys_State_Normal)IsNeedToClearOCF=true;
	else if(CState.VBusState==VBUS_OverVolt)IsNeedToClearOCF=true;
	else IsNeedToClearOCF=false;
	if(ClearOCFlag&&IsNeedToClearOCF)IP2366_ClearOCFlag();
	//打开仅放电模式，直接导航到适配器模拟
	if(IsEnterDischargeMode)SwitchingMenu(&AdapterEmuMenu);
	//如果C口连接则弹出提示让用户重新插拔C口	
	else if((IsNeedToClearOCF||IsNeedToReConfig)&&IP2366_GetIfCPortConnected())
		{
		IsCPortConnected=true;
		SwitchingMenu(&InfoUserRemoveCCableMenu);
		}
	//返回主菜单
	else
		{
		ClearScreen(); //清屏
		SwitchingMenu(&MainMenu);
		}
	//退出之前需要reset index
	ClearMenuIndex();
	}
	
//实现仅充电和仅放电互斥
bool IP2366_IsEnableDischarge(float VBatRaw);	
	
void QuickAccesSelProc(void)
	{
	//清除OC Flag和仅放电模式互斥	
	if(ClearOCFlag&&IsEnterDischargeMode)IsEnterDischargeMode=false;
	//仅充电和仅放电互斥
	if(TempChargeConfig&&IsEnterDischargeMode)IsEnterDischargeMode=false;
	//配置系统内关闭存储模式后存储模式开关不允许打开
	if(CfgData.StorageModeINROM==StorageMode_OFF&&IsTempEnableStorageMode)IsTempEnableStorageMode=false;
	//系统当前处于安全充电模式，或者开启存储模式且电池电压过低时禁止开启仅放电
	if(IsBootFromVBUS)IsEnterDischargeMode=false;
	if(IsTempEnableStorageMode&&!IP2366_IsEnableDischarge(ADCO.Vbatt))IsEnterDischargeMode=false;
	}

const MenuConfigDef QuickAccessMenu=
	{
	MenuType_BoolListSetup,
	//布尔类的入口
	QuickAccessParam,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//特殊渲染的处理
	&QuickAccesSelProc, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"快捷菜单",
	NULL,
	NULL,
	&ReturnFromQuickAccMenu, 
	//进入和退出构造函数没有事情要做
	&EnterQuickMenu,
	NULL
	};
