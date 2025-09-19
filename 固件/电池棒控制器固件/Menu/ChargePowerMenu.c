#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"

const char EmptyStr[]="\0";
const EnumEditEntryDef PowerCfg[7]=
	{
		{
		"30W",
	  false,
		Power_30W,
		false,
		},
		{
		"45W(20V2.25A)",
	  false,
		Power_45W,
		false,
		},
		{
		"60W(20V3A)",
	  false,
		Power_60W,
		false,
		},	
		{
		"65W(20V3.25A)",
	  false,
		Power_65W,
		false,
		},		
		{
		"100W(烫烫烫)",
	  true,
		Power_100W,
		false,
		},	
		{
		"140W(垃圾电池别用)",
	  true,
		Power_140W,
		false,
		},	
		{ //占位符
		"",
	  false,
		100,
		true
		}
	};

//内部动态菜单结构体
EnumEditEntryDef DynamicPowerCfg[7];
	
void PrepareDynamicPowerCfg(void)
	{
	int i;
	ChargePowerDef MaxPower;
	//构建最大充电功率
	MaxPower=CurrentIP2366FW->MaxCapableChgPower;
	if(MaxPower>Power_65W&&!CurrentIP2366FW->IsHyperChargeCapable)MaxPower=Power_65W;
	if(MaxPower>Power_100W&&CfgData.MaxVPD==PDMaxIN_20V)MaxPower=Power_100W;
	//开始构建菜单数据	
	for(i=0;i<7;i++)
		{
		DynamicPowerCfg[i].IsChinese=true;
		if(PowerCfg[i].EnumValue>MaxPower)
			{
			//当前菜单项所指定的充电功率超过芯片上限，设置为placeholder
			DynamicPowerCfg[i].EnumValue=100;
			DynamicPowerCfg[i].IsPlaceHolder=true;
			DynamicPowerCfg[i].IsChinese=false;
			DynamicPowerCfg[i].SelName=(char *)EmptyStr;
			}
		else
			{
			//当前菜单项所指的充电功率在芯片上限内，进行构建
			DynamicPowerCfg[i].EnumValue=PowerCfg[i].EnumValue;
			DynamicPowerCfg[i].IsPlaceHolder=false;
			DynamicPowerCfg[i].IsChinese=PowerCfg[i].IsChinese;
			DynamicPowerCfg[i].SelName=PowerCfg[i].SelName;
			}
		}
	}	
	
int ReadPWREnumValue(void)
	{
	ChargePowerDef MaxPower;
	//如果当前的充放电功率设置值超过芯片所容许的上限，则clamp到已知的值
	MaxPower=CurrentIP2366FW->MaxCapableChgPower;
	if(MaxPower>Power_65W&&!CurrentIP2366FW->IsHyperChargeCapable)MaxPower=Power_65W;
	if(MaxPower>Power_100W&&CfgData.MaxVPD==PDMaxIN_20V)MaxPower=Power_100W;
	//超过上限时返回最大值
  if(CfgData.InputConfig.ChargePower>MaxPower)return (int)MaxPower;		
	//返回充电功率的enum值	
	return (int)CfgData.InputConfig.ChargePower;
	}
	
void FedPWREnumValue(int Input)
	{
	//返回enum值
	CfgData.InputConfig.ChargePower=(ChargePowerDef)Input;
	//判断参数并修正非法参数
	if(CfgData.InputConfig.ChargePower>CurrentIP2366FW->MaxCapableChgPower)
		CfgData.InputConfig.ChargePower=CurrentIP2366FW->MaxCapableChgPower; //充电功率超过芯片报告的极限，进行恢复
	if(CfgData.MaxVPD==PDMaxIN_20V&&CfgData.InputConfig.ChargePower==Power_140W)	
		CfgData.InputConfig.ChargePower=Power_100W;	
	//支持充放电功率单独设置的固件，且没有打开高级管理员配置，则同步充放电配置
  if(CurrentIP2366FW->ExtendedTCSetting&&(!CfgData.EnableAdvAccess||!CfgData.EnableChargeConfig))
		{
		//同步充放电设置
		CfgData.MaxSnkPower=CfgData.InputConfig.ChargePower;
		}
  //返回到对应设置菜单
	if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
	else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
	}

//退出配置菜单时Clamp充放电功率
void ClampChargePower(void)
	{
	ChargePowerDef MaxPower;
	//如果当前的充放电功率设置值超过芯片所容许的上限，则clamp到已知的值
	MaxPower=CurrentIP2366FW->MaxCapableChgPower;
	if(MaxPower>Power_65W&&!CurrentIP2366FW->IsHyperChargeCapable)MaxPower=Power_65W;
	if(MaxPower>Power_100W&&CfgData.MaxVPD==PDMaxIN_20V)MaxPower=Power_100W;
	//限制充放功率
	if(CfgData.InputConfig.ChargePower>MaxPower)CfgData.InputConfig.ChargePower=MaxPower;
	if(!CurrentIP2366FW->ExtendedTCSetting)CfgData.MaxSnkPower=Power_140W;
	else if(CfgData.MaxSnkPower>MaxPower)CfgData.MaxSnkPower=MaxPower;
	}	
	
const MenuConfigDef PowerSetMenu=
	{
	MenuType_EnumSetup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	DynamicPowerCfg,
  &ReadPWREnumValue,
  &FedPWREnumValue,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"充放电功率配置",
	NULL,
	NULL,
	NULL, 
	//进入和退出构造函数没有事情要做
	&PrepareDynamicPowerCfg,
	&ClampChargePower
	};	

