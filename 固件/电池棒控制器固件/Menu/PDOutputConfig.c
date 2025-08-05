#include "GUI.h"
#include "Config.h"

//外部变量声明
extern bool IsEnable17AMode;

//内部变量，决定PPS1和2的电流是否能调节
static bool IsEnablePPS1Set=false;
static bool IsEnablePPS2Set=false;
static bool IsEnable9VSet=false;
static bool IsEnable12VSet=false;
static bool IsEnable15VSet=false;
static bool IsEnable20VSet=false;

//判断PPS1和PPS2是否能进行处理的函数
void CheckIfPPS1And2CanAdjust(void)
	{
	//芯片为公版不允许调节
	if(!CurrentIP2366FW->IsExtendPDOCapable||!CfgData.OutputConfig.IsEnablePDOut)
		{
		IsEnablePPS1Set=false;
		IsEnablePPS2Set=false;
		}
	//根据PPS是否启用来决定是否允许调节
	else
		{
		if(!CfgData.PPSConfig.IsEnablePPS1Set)IsEnablePPS1Set=false;
		else IsEnablePPS1Set=CfgData.PDOCFG.EnablePPS1;
		if(!CfgData.PPSConfig.IsEnablePPS2Set)IsEnablePPS2Set=false;
		else IsEnablePPS2Set=CfgData.PDOCFG.EnablePPS2;
		}
	//关闭PD输出，所有选项除能
	if(!CfgData.OutputConfig.IsEnablePDOut)
		{	
		IsEnable9VSet=false;
		IsEnable12VSet=false;
		IsEnable15VSet=false;
		IsEnable20VSet=false;
		}
	//根据PDO是否使能进行设置
	else
		{
		//9V PDO
		if(!CfgData.PDOCFG.Enable9V)IsEnable9VSet=false;
		else IsEnable9VSet=CfgData.FixedPDOCfg.IsEnable9VPDOSet;
		//12V PDO
		if(!CfgData.PDOCFG.Enable12V)IsEnable12VSet=false;
		else IsEnable12VSet=CfgData.FixedPDOCfg.IsEnable12VPDOSet;		
		//15V PDO
		if(CfgData.InputConfig.ChargePower==Power_30W)IsEnable15VSet=false;
		else if(!CfgData.PDOCFG.Enable15V)IsEnable15VSet=false;
		else IsEnable15VSet=CfgData.FixedPDOCfg.IsEnable15VPDOSet;	
		//20V PDO
		if(!CfgData.PDOCFG.Enable20V)IsEnable20VSet=false;
		else if(CfgData.InputConfig.ChargePower==Power_45W)IsEnable20VSet=false;
		else if(CfgData.InputConfig.ChargePower==Power_30W)IsEnable20VSet=false;	
		else IsEnable20VSet=CfgData.FixedPDOCfg.IsEnable20VPDOSet;	
		}
	}

//进入菜单函数
void EnterPPS1Set(void)
	{
	SwitchingMenu(&PPS1IsetMenu);
	}

void EnterPPS2Set(void)
	{
	SwitchingMenu(&PPS2IsetMenu);
	}
	
void EnterPDOSet(void)
	{
	SwitchingMenu(&PDOCfgMenu);
	}
	
void EnterFixPDOICCMODEN(void)
	{
	SwitchingMenu(&FixedPDOICCModifyCfgMenu);
	}
	
void EnterFPDO9VSet(void)
	{
	SwitchingMenu(&PDO9VIsetMenu);
	}

void EnterFPDO12VSet(void)
	{
	SwitchingMenu(&PDO12VIsetMenu);
	}	

void EnterFPDO15VSet(void)
	{
	SwitchingMenu(&PDO15VIsetMenu);
	}	
	
void EnterFPDO20VSet(void)
	{
	SwitchingMenu(&PDO20VIsetMenu);
	}
	
//菜单项参数
const SetupMenuSelDef PDOSetupSubMenu[9]=
	{
		{
		"PDO广播设置",
		false,
		&AlwaysTrue,
		&EnterPDOSet
		},
		{
		"PDO电流修改设置",
		false,
		&AlwaysTrue,
		&EnterFixPDOICCMODEN	
		},
		{
		"PPS1电流设置",
		false,
		&IsEnablePPS1Set,
		&EnterPPS1Set
		},
		{
		"PPS2电流设置",
		false,
		&IsEnablePPS2Set,
		&EnterPPS2Set
		},
		{
		"9V PDO电流设置",
		false,
		&IsEnable9VSet,
		&EnterFPDO9VSet
		},
		{
		"12V PDO电流设置",
		false,
		&IsEnable12VSet,
		&EnterFPDO12VSet
		},
		{
		"15V PDO电流设置",
		false,
		&IsEnable15VSet,
		&EnterFPDO15VSet
		},
		{
		"20V PDO电流设置",
		false,
		&IsEnable20VSet,
		&EnterFPDO20VSet
		},
		{
		"\0",
		true,
		&AlwaysTrue,
		NULL,
		}
	};

void ReturnToMainSetMenu(void);
	
const MenuConfigDef PDOutputSetMenu=
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
	"PD输出配置",
	PDOSetupSubMenu,
	NULL,
	&ReturnToMainSetMenu, 
	//进入时根据设置更新是否允许设置
	&CheckIfPPS1And2CanAdjust,
	NULL
	};
