#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"
#include <string.h>

//字符串
static char PDO20VMsg[16]={0};

const BoolListEntryDef PDOParam[7]=
	{
		{
		"PPS1",
		false,
		&CfgData.PDOCFG.EnablePPS1,
		false,
		false
		},
   	{
		"PPS2",
		false,
		&CfgData.PDOCFG.EnablePPS2,
		false,
		false
		},
		{
		PDO20VMsg,
		true,
		&CfgData.PDOCFG.Enable20V,
		false,
		false
		},
		{
		"15V",
		false,
		&CfgData.PDOCFG.Enable15V,
		false,
		false
		},
		{
		"12V PDO",
		false,
		&CfgData.PDOCFG.Enable12V,
		false,
		false
		},
		{
		"9V PDO",
		false,
		&CfgData.PDOCFG.Enable9V,
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

//进入PDO广播配置前需要准备一下字符串	
void PreparePDO20VStr(void)
	{
	memset(PDO20VMsg,0x00,sizeof(PDO20VMsg));
	if(CfgData.MaxVPD==PDMaxIN_20V)strncpy(PDO20VMsg,"20V",sizeof(PDO20VMsg));
  else strncpy(PDO20VMsg,"20V和28V EPR",sizeof(PDO20VMsg));
	}
	
//函数声明
void LeaveDisMgmtMenu(void);

const MenuConfigDef PDOCfgMenu=
	{
	MenuType_BoolListSetup,
	//布尔类的入口
	PDOParam,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"PDO广播配置",
	NULL,
	NULL,
	&LeaveDisMgmtMenu, 
	//进入和退出构造函数
	&PreparePDO20VStr,  //进入前需要复制一下字符串根据PDO模式决定是否有EPR相关的提示
	NULL
	};	
