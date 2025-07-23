#include "GUI.h"
#include "Config.h"

const EnumEditEntryDef SleepCfg[3]=
	{
		{
		"电池棒模式",
	  true,
		System_Sleep_Deep,
		false,
		},
		{
		"充电宝模式(即插即用)",
	  true,
		System_Sleep_Normal,
		false,
		},
		{ //占位符
		"",
	  false,
		100,
		true
		}
	};

	
int ReadDeepSleepEnumVal(void)
	{
	return (int)CfgData.SleepCfg;
	}	
	
void FedDeepSleepEnumVal(int Input)	
	{
	CfgData.SleepCfg=(SystemSleepStateDef)Input;
	//关闭输出模式的话不允许系统进入浅度睡眠
	if(!CfgData.OutputConfig.IsEnableOutput)CfgData.SleepCfg=System_Sleep_Deep;
	SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
	}
	
//函数声明
int ReadPWREnumValue(void);
void FedPWREnumValue(int Input);

const MenuConfigDef SleepCfgMenu=
	{
	MenuType_EnumSetup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	SleepCfg,
  &ReadDeepSleepEnumVal,
  &FedDeepSleepEnumVal,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"系统休眠配置",
	NULL,
	NULL,
	NULL, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};	
