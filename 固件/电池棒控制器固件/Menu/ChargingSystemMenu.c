#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"

//进入恒流充电电流设置
void EnterIChargeSet(void)
	{
	SwitchingMenu(&IChargeSetMenu);
	}
//进入预充电电流设置
void EnterPreChargeIset(void)
	{
	SwitchingMenu(&PreChargeISetMenu);
	}	
//进入预充电电压设置
void EnterChgVsetMenu(void)
	{
	SwitchingMenu(&ChgVSetMenu);
	}
//进入再充电配置
void EnterRechargeSetMenu(void)
	{
	SwitchingMenu(&RechargeSetMenu);
	}	
	
void EnterIstopMenu(void)
	{
  //根据固件模式进行配置选择对应的处理
	if(CurrentIP2366FW->IsHyperChargeCapable)SwitchingMenu(&IstopSetMenu);
	else SwitchingMenu(&IstopStdSetMenu);
	}

void EnterMaxVPDMenu(void)
	{
	SwitchingMenu(&MaxVPDMenu);
	}	
	
//菜单项参数
static bool EnableMaxVPDConfig=false;

//进行VPD使能配置
void SetEnableMaxVPDConfig(void)
	{
	if(!CurrentIP2366FW->IsHyperChargeCapable)EnableMaxVPDConfig=false;
	else if(CurrentIP2366FW->MaxCapableChgPower!=Power_140W)EnableMaxVPDConfig=false;
	else if(CurrentIP2366FW->IP2366ICCMAX>9700)EnableMaxVPDConfig=true;
	else EnableMaxVPDConfig=false;
	}	
	
const SetupMenuSelDef ChargeSystemSetup[7]=
	{
		{
		"电池峰值电流设置",
		false,
		&AlwaysTrue,
		&EnterIChargeSet
		},
		{
		"预充电电流设置",
		false,
		&AlwaysTrue,
		&EnterPreChargeIset
		},
		{
		"恒压充电电压设置",
		false,
		&AlwaysTrue,
		&EnterChgVsetMenu
		},
		{
		"再充电阈值设置",
		false,
		&AlwaysTrue,		
		&EnterRechargeSetMenu
		},
		{
		"停充电流设置",
		false,
		&AlwaysTrue,		
		&EnterIstopMenu
		},
		{
		"最高充放电压设置",
		false,
		&EnableMaxVPDConfig,		
		&EnterMaxVPDMenu
		},
		{
		"\0",
		true,
		&AlwaysTrue,
		NULL,
		}
	};

void ReturnToMainSetMenu(void)
	{
	SwitchingMenu(&SetMainMenu);
	}
	
const MenuConfigDef ChgSysSetMenu=
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
	"充电系统配置",
	ChargeSystemSetup,
	NULL,
	&ReturnToMainSetMenu, 
	//进入和退出构造函数没有事情要做
	SetEnableMaxVPDConfig,
	NULL
	};	
