#include "GUI.h"
#include "Config.h"

//回到设置菜单的函数声明
void ReturnFromPPSIset(void);

//配置参数
const intEditMenuCfg PDO9VCurrentEdit=
	{
	&CfgData.FixedPDOCfg.PDO9VICCMAX, //数据源
	500,
	3000, //0.5到3A
	10, //LSB=10mA
	"mA", //毫安
	"",
	"",
  &ReturnFromPPSIset,
	};
	
const intEditMenuCfg PDO12VCurrentEdit=
	{
	&CfgData.FixedPDOCfg.PDO12VICCMAX, //数据源
	500,
	3000, //0.5到3A
	10, //LSB=10mA
	"mA", //毫安
	"",
	"",
  &ReturnFromPPSIset,
	};
	
const intEditMenuCfg PDO15VCurrentEdit=
	{
	&CfgData.FixedPDOCfg.PDO15VICCMAX, //数据源
	500,
	3000, //0.5到3A
	10, //LSB=10mA
	"mA", //毫安
	"",
	"",
  &ReturnFromPPSIset,
	};

const intEditMenuCfg PDO20VCurrentEdit=
	{
	&CfgData.FixedPDOCfg.PDO20VICCMAX, //数据源
	1000,
	4000, //1到4A
	10, //LSB=10mA
	"mA", //毫安
	"",
	"",
  &ReturnFromPPSIset,
	};	
	
const intEditMenuCfg ExtendedPDO20VCurrentEdit=
	{
	&CfgData.FixedPDOCfg.PDO20VICCMAX, //数据源
	1000,
	7000, //1到7A
	50, //LSB=50mA
	"mA", //毫安
	"",
	"",
  &ReturnFromPPSIset,
	};	
	
//9V PDO电流设置菜单	
void FPDO9VIsetDummy(void)
	{
	IntEditHandler(&PDO9VCurrentEdit);
	}	

const MenuConfigDef PDO9VIsetMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&FPDO9VIsetDummy, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"9V PDO电流设置",
	NULL,
	NULL, 
	NULL,
	//进入的时候初始化菜单编辑，退出的时候检查数值
	&IntEditInitHandler,
	NULL
	};

//12V PDO电流设置菜单	
void FPDO12VIsetDummy(void)
	{
	IntEditHandler(&PDO12VCurrentEdit);
	}	

const MenuConfigDef PDO12VIsetMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&FPDO12VIsetDummy, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"12V PDO电流设置",
	NULL,
	NULL, 
	NULL,
	//进入的时候初始化菜单编辑，退出的时候检查数值
	&IntEditInitHandler,
	NULL
	};

//15V PDO电流设置菜单	
void FPDO15VIsetDummy(void)
	{
	IntEditHandler(&PDO15VCurrentEdit);
	}	

const MenuConfigDef PDO15VIsetMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&FPDO15VIsetDummy, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"15V PDO电流设置",
	NULL,
	NULL, 
	NULL,
	//进入的时候初始化菜单编辑，退出的时候检查数值
	&IntEditInitHandler,
	NULL
	};	
	
//20V PDO电流设置菜单	
void FPDO20VIsetDummy(void)
	{
	//根据固件配置使用不同的PDO设置
	if(CurrentIP2366FW->IsExtendPDOCapable)IntEditHandler(&ExtendedPDO20VCurrentEdit);
	else IntEditHandler(&PDO20VCurrentEdit);
	}	
	
void FPDO20VExitCheck(void)
	{
	extern bool IsSupportExterndPDO;
	int buf,LSBCount;
	//公版固件检查数值限制PDO在1000-4000mA
	if(!CurrentIP2366FW->IsExtendPDOCapable)
		{
		buf=CfgData.FixedPDOCfg.PDO20VICCMAX;
		if(buf<1000)buf=1000;
		if(buf>4000)buf=4000;
		CfgData.FixedPDOCfg.PDO20VICCMAX=buf;
		}
	//非公版固件检查数值限制PDO在1000-7000mA且为50mA的倍数
	else
		{
		buf=CfgData.FixedPDOCfg.PDO20VICCMAX;
		if(buf<1000)buf=1000;
		if(buf>7000)buf=7000;	
		//强制取整为50mA per LSB
		LSBCount=buf/50;
		CfgData.FixedPDOCfg.PDO20VICCMAX=LSBCount*50;
		}
	}
	
void FPDO20VEnterCheck(void)
	{
	extern bool IsSupportExterndPDO;
	int buf,LSBCount;
	//开启50mA per LSB模式后，强制20V PDO电流设置为50mA的倍数
	if(CurrentIP2366FW->IsExtendPDOCapable)
		{
		//限制数值
		buf=CfgData.FixedPDOCfg.PDO20VICCMAX;
		if(buf<1000)buf=1000;
		if(buf>7000)buf=7000;	
		//强制取整为50mA per LSB
		LSBCount=buf/50;
		CfgData.FixedPDOCfg.PDO20VICCMAX=LSBCount*50;
		}
	IntEditInitHandler();
	}

const MenuConfigDef PDO20VIsetMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&FPDO20VIsetDummy, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"20V PDO电流设置",
	NULL,
	NULL, 
	NULL,
	//进入的时候初始化菜单编辑，退出的时候检查数值
	&FPDO20VEnterCheck,
	&FPDO20VExitCheck
	};	
