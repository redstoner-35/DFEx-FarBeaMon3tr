#include "Config.h"
#include "Key.h"
#include "GUI.h"

const EnumEditEntryDef IStopStdCfg[11]=
	{
		{
		"50mA(不推荐)",
	  true,
		IStop_50mA,
		false,
		},	
		{
		"100mA",
	  false,
		IStop_100mA,
		false,
		},	
		{
		"150mA",
	  false,
		IStop_150mA,
		false,
		},	
		{
		"200mA",
	  false,
		IStop_200mA,
		false,
		},		
		{
		"250mA",
	  false,
		IStop_250mA,
		false,
		},		
		{
		"300mA",
	  false,
		IStop_300mA,
		false,
		},	
		{
		"350mA",
	  false,
		IStop_350mA,
		false,
		},		
		{
		"400mA",
	  false,
		IStop_400mA,
		false,
		},	
		{
		"450mA",
	  false,
		IStop_450mA,
		false,
		},		
		{
		"0.5A(不推荐)",
	  true,
		IStop_500mA,
		false,
		},			
		{ //占位符
		"",
	  false,
		100,
		true
		}
	};
	
//电流enum读取声明
int ReadIStopEnumValue(void);
void FedIStopEnumValue(int Input);

const MenuConfigDef IstopStdSetMenu=
	{
	MenuType_EnumSetup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	IStopStdCfg,
  &ReadIStopEnumValue,
  &FedIStopEnumValue,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"停充电流设置",
	NULL,
	NULL,
	NULL, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};	
