#include "Config.h"
#include "Key.h"
#include "GUI.h"
#include <string.h>
#include <stdio.h>
#include "ADC.h"

//再充电电压字符
static char VRechargeMsg[3][21]={0};
static const char ShowVRProc[]="%.2fV(%.3fV/Cell)";

const EnumEditEntryDef RechargeCfg[5]=
	{
		{
		"关闭再充电功能",
	  true,
		Recharge_Disable,
		false,
		},
		{
		&VRechargeMsg[0][0],
	  false,
		Recharge_0V05,
		false,
		},
		{
		&VRechargeMsg[1][0],
	  false,
		Recharge_0V1,
		false,
		},	
		{
		&VRechargeMsg[2][0],
	  false,
		Recharge_0V2,
		false,
		},		
		{ //占位符
		"",
	  false,
		100,
		true
		}
	};

int ReadRechargeEnumValue(void)
	{
	float buf;
	int i;
	//进行计算	
	for(i=0;i<3;i++)memset(VRechargeMsg[i],0,21);
	buf=((float)CfgData.InputConfig.FullVoltage)/1000;
	snprintf(VRechargeMsg[0],21,ShowVRProc,BattCellCount*(buf-0.05),buf-0.05);
	snprintf(VRechargeMsg[1],21,ShowVRProc,BattCellCount*(buf-0.1),buf-0.1);
	snprintf(VRechargeMsg[2],21,ShowVRProc,BattCellCount*(buf-0.2),buf-0.2);
	//返回充电功率的enum值
	return (int)CfgData.VRecharge;
	}
	
void FedRechargeEnumValue(int Input)
	{
	CfgData.VRecharge=(ReChargeConfig)Input;
	SwitchingMenu(&ChgSysSetMenu);
	}

const MenuConfigDef RechargeSetMenu=
	{
	MenuType_EnumSetup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	RechargeCfg,
  &ReadRechargeEnumValue,
  &FedRechargeEnumValue,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"再充电阈值设置",
	NULL,
	NULL,
	NULL, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};
