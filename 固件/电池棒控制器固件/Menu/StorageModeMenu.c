#include "GUI.h"
#include "Config.h"

const EnumEditEntryDef StorageModeCfg[5]=
	{
		{
		"关闭存储模式",
	  true,
		StorageMode_OFF,
		false,
		},
		{
		"开启,限压3.6V",
	  true,
		StorageMode_3V6,
		false,
		},
		{
		"开启,限压3.7V",
	  true,
		StorageMode_3V7,
		false,
		},	
		{
		"开启,限压3.8V",
	  true,
		StorageMode_3V8,
		false,
		},		
		{ //占位符
		"",
	  false,
		100,
		true
		}
	};

int ReadStorageModeEnumValue(void)
	{
	//返回充电功率的enum值	
	return (int)CfgData.StorageModeINROM;
	}
	
void FedStorageModeEnumValue(int Input)
	{
	//返回enum值
	CfgData.StorageModeINROM=(StorageModeDef)Input;
	StorageMode=CfgData.StorageModeINROM; //更新临时存储值
  //返回到对应设置菜单
	if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
	else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
	}

const MenuConfigDef StorageModeSetMenu=
	{
	MenuType_EnumSetup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	StorageModeCfg,
  &ReadStorageModeEnumValue,
  &FedStorageModeEnumValue,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"长期存储模式设置",
	NULL,
	NULL,
	NULL, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};	

