#ifndef _CFG_
#define _CFG_

#include "IP2366_REG.h"
#include "LCD_Init.h"

typedef enum
	{
	AutoSave_Disabled, //自动存盘关闭
	AutoSave_Enabled	 //自动存盘开启
	}AutoSaveCfgDef; //自动保存配置

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
	StorageMode_OFF, //存储模式禁用
	StorageMode_3V6, //存储模式启用，电池最大电压3.6V
	StorageMode_3V7, //存储模式启用，电池最大电压3.7V
	StorageMode_3V8 //存储模式启用，电池最大电压3.8V
	}StorageModeDef;	
	
typedef enum
	{
	Balance_Diasbled, //永久关闭主动均衡
	Balance_ChgDisOnly, //仅充放电时启用
	Balance_ChgOnly, //仅在充电时启用
	Balance_AlwaysEnabled, //均衡永远开启
	}BalanceModeDef;	
	
typedef enum
	{
	System_Sleep_Deep, //系统开启深度睡眠模式（最低待机功耗但是无法检测到纯受电设备的插入）
	System_Sleep_Normal //系统关闭深度睡眠模式（待机功耗增加但是可以检测到纯受电设备的插入）
	}SystemSleepStateDef;	
	
//系统配置
typedef struct
	{
	IP2366InputDef InputConfig;
	IP2366OutConfigDef OutputConfig;
	VBatLowDef Vlow;
	PDOBroadcastDef PDOCFG;
	//输入系统配置
	VBatLowDef PreChargeEndVoltage;     //预充电结束电压（电池电压高于该数值后，系统由低电流涓流充电转为恒流充电）
	bool EnableSmartSinkPower;          //智能自充功率功能（开启后系统将根据开启充电时的系统温度自动配置合适的自充功率）
  bool EnableAutoPowerSave;           //自动省电模式，在充满电20秒后让适配器关闭快充，进入5V模式		
	ChargePowerDef MaxSnkPower;         //最大的输入sink功率
	IP2366SinkProtocolDef SinkConfig;
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
	bool EnableTCCalibration; //开启TypeC功率计修正值配置
	bool EnableExtendedBalance; //在均衡开启的时候，是否使能自动补充均衡
	//TypeC功率修正数据配置	
	int TypeCAmpereCalCharge;  //充电时的修正值
	int TypeCVoltageCal;  
	int TypeCAmpereCal; //Type-C的电压和电流修正值，LSB=0.1% 1000=原始值的100%
	//过热保护配置		
	int OverHeatLockTemp; //过热保护时间
	//GUI和显示配置
  SystemSleepStateDef SleepCfg; //系统睡眠配置
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
  //PPS输出电流设置
	IP2366PPSPDOSetDef PPSConfig;
	//Fixed PDO电流配置
	IP2366FixPDOSetDef FixedPDOCfg;
  //存储模式和自动存盘配置
	AutoSaveCfgDef AutoSaveCfg;
	StorageModeDef StorageModeINROM;
  //本地电池和温度测量校准设置
  int SystemTempCalFactor;     //系统温度测量校准系数
	int BatteryVoltageCalFactor;
	int BatteryCurrentCalFactor; //电池电压电流校准系数
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
extern bool DCDCOutputBit; //输出bit
extern StorageModeDef StorageMode; //存储模式缓存
#define CfgFileSize (sizeof(CfgUnion)*2)	
#define CfgData CfgUnion.ROMImage.Data.Data
#define CfgChecksum CfgUnion.ROMImage.CRCResult

//函数
void TryToSaveConfigDuringPost(char Present); //尝试保存配置
bool CheckIfConfigOK(bool IsBackup);  //检查指定的配置文件状态
void SyncUnResetThings(CfgUnionDef *IN);  //检测出厂设置时同步不需要更改的内容
bool CheckIfConfigIsSame(void); //配置相同
bool ReadConfiguration(CfgUnionDef *Out,bool IsUsingBackup);
bool WriteConfiguration(CfgUnionDef *IN,bool ForceUpdate); //读写数据
void RestoreFactoryWithoutSomeSettings(void);//设置里面恢复出厂但是不重置校准配置
void LoadConfig(void); //开机时读取配置
void LoadDefaultConfig(CfgUnionDef *IN,bool IsFactoryOverride); //加载默认配置到某个地方
unsigned int CalcROMCRC32(CfgUnionDef *IN); //计算CRC32

#endif
