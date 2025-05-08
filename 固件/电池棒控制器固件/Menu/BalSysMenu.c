#include "Config.h"
#include "Key.h"
#include "GUI.h"

const EnumEditEntryDef BALModeCfg[5]=
	{
		{
		"始终开启",
	  true,
		Balance_AlwaysEnabled,
		false,
		},
		{
		"仅充电时开启",
	  true,
		Balance_ChgOnly,
		false,
		},
		{
		"仅充放电时开启",
	  true,
		Balance_ChgDisOnly,
		false,
		},
		{
		"始终关闭(不推荐)",
	  true,
		Balance_Diasbled,
		false,
		},
		{ //占位符
		"",
	  false,
		100,
		true
		}
	};
int ReadBalEnumValue(void)
	{
	//返回均衡系统的enum值
	return (int)CfgData.BalanceMode;
	}
	
void FedBalEnumValue(int Input)
	{
	CfgData.BalanceMode=(BalanceModeDef)Input;
	SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
	}

const MenuConfigDef BalSysSetMenu=
	{
	MenuType_EnumSetup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	BALModeCfg,
  &ReadBalEnumValue,
  &FedBalEnumValue,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"主动均衡设置",
	NULL,
	NULL,
	NULL, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};
