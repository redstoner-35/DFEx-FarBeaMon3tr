#ifndef _CFG_
#define _CFG_

#include "IP2366_REG.h"
#include "LCD_Init.h"

typedef enum
	{
	InstantCTest_NotTriggered, //下次上电直接进容量测试未启动
	InstantCTest_Armed, //下次直接进容量测试已激活但是未动作
	InstantCTest_EnteredOK //瞬时容量测试顺利进入
	}InstantCTestDef;

typedef enum
	{
	PDMaxIN_20V,
	PDMaxIN_28V
	}MaximumPDVoltageDef;	
	
typedef enum
	{
	Balance_Diasbled, //永久关闭主动均衡
	Balance_ChgDisOnly, //仅充放电时启用
	Balance_ChgOnly, //仅在充电时启用
	Balance_AlwaysEnabled, //均衡永远开启
	}BalanceModeDef;	
	
//系统配置
typedef struct
	{
	IP2366InputDef InputConfig;
	IP2366OutConfigDef OutputConfig;
	VBatLowDef Vlow;
	PDOBroadcastDef PDOCFG;
	//安全设置
	bool EnableHPGauge; //开启高精度GTypeC输入计量
  bool EnableAdvAccess; //开启开发者菜单的一键访问		
	bool EnableChargeConfig; //开启充电配置
  bool EnableDischargeConfig; //开启放电配置
  bool EnableChargPowerConfig; //开启充电功率配置
  bool EnablePDOConfig; //开启PDO设置的配置		
	bool EnableLVProtectConfig; //开启低压保护配置
	bool EnableOTPConfig; //打开过热保护配置
	bool EnableThermalStepdown; //开启过热后自动掉功率的机制
	//过热保护配置
	int OverHeatLockTemp; //过热保护时间
	//GUI和显示配置
	bool EnableLargeMenu; //启用大菜单
	bool EnableFastBoot; //启用快速启动
	LCDDisplayDirDef DisplayDir;
	//容量测试配置	
	InstantCTestDef InstantCTest;
	//再充电参数配置
	ReChargeConfig VRecharge;
	IStopConfig IStop;
  //最大PD充电输入配置
	MaximumPDVoltageDef MaxVPD;
  //均衡系统模式配置
  BalanceModeDef BalanceMode;
	}SystemCfgDef;

typedef union
	{
	SystemCfgDef Data;
	char ByteBuf[sizeof(SystemCfgDef)];
	}CfgDataStorDef;	
	
typedef struct
	{	
	unsigned int CRCResult;
	CfgDataStorDef Data;
	}CfgROMImageDef;	

typedef union
	{
	CfgROMImageDef ROMImage;
	char ByteBuf[sizeof(CfgROMImageDef)];
	}CfgUnionDef;

//外部参考
extern CfgUnionDef CfgUnion;
#define CfgFileSize sizeof(CfgUnion)	
#define CfgData CfgUnion.ROMImage.Data.Data
#define CfgChecksum CfgUnion.ROMImage.CRCResult

//函数
bool CheckIfConfigIsSame(void); //配置相同
bool ReadConfiguration(CfgUnionDef *Out);
bool WriteConfiguration(CfgUnionDef *IN,bool ForceUpdate); //读写数据
void RestoreDefaultConfig(void); //重置出厂
void LoadConfig(void); //开机时读取配置
void LoadDefaultConfig(CfgUnionDef *IN); //加载默认配置到某个地方
unsigned int CalcROMCRC32(CfgUnionDef *IN); //计算CRC32

#endif
