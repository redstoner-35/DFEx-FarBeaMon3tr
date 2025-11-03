#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"

//标志位，是否发生sink功率的变动
bool IsSinkPowerChanged=false;

//外部动态构建充电功率的系统
extern EnumEditEntryDef DynamicPowerCfg[7];
void PrepareDynamicPowerCfg(void);
void ClampChargePower(void);

int ReadSinkPowerEnumValue(void)
	{
	ChargePowerDef MaxPower;
	//复位标志位
	IsSinkPowerChanged=false;
	//如果当前的充放电功率设置值超过芯片所容许的上限，则clamp到已知的值
	MaxPower=CurrentIP2366FW->MaxCapableChgPower;
	if(MaxPower>Power_65W&&!CurrentIP2366FW->IsHyperChargeCapable)MaxPower=Power_65W;
	if(MaxPower>Power_100W&&CfgData.MaxVPD==PDMaxIN_20V)MaxPower=Power_100W;
	//超过上限时返回合法的最大值
  if(CfgData.MaxSnkPower>MaxPower)return (int)MaxPower;		
	//返回充电功率的enum值	
	return (int)CfgData.MaxSnkPower;
	}	
	
void FedSinkPowerEnumValue(int Input)
	{
	//返回enum值
	if(CfgData.MaxSnkPower!=(ChargePowerDef)Input)IsSinkPowerChanged=true; //Sink功率发生更改，执行判断
	CfgData.MaxSnkPower=(ChargePowerDef)Input;
	//判断参数并修正非法参数
	if(CfgData.MaxSnkPower>CurrentIP2366FW->MaxCapableChgPower)
		CfgData.MaxSnkPower=CurrentIP2366FW->MaxCapableChgPower; //充电功率超过芯片报告的极限，进行恢复
	if(CfgData.MaxVPD==PDMaxIN_20V&&CfgData.MaxSnkPower==Power_140W)	
		CfgData.MaxSnkPower=Power_100W;	
  //返回到对应设置菜单
	SwitchingMenu(&ChgSysSetMenu);
	}
	
const MenuConfigDef SinkPowerSetMenu=
	{
	MenuType_EnumSetup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	DynamicPowerCfg,
  &ReadSinkPowerEnumValue,
  &FedSinkPowerEnumValue,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"输入最大功率设置",
	NULL,
	NULL,
	NULL, 
	//进入和退出构造函数没有事情要做
	&PrepareDynamicPowerCfg,
	&ClampChargePower
	};	
