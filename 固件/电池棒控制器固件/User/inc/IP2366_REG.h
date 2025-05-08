#ifndef _2366REG_
#define _2366REG_

#include <stdbool.h>

typedef enum
	{
	//可读写寄存器
	REG_SYSCTL0=0,
	REG_SYSCTL2=2,
	REG_SYSCTL3=3,
	REG_SYSCTL6=6,
	REG_SYSCTL8=8,
	REG_SYSCTL9=9,
	REG_SYSCTL10=0x0A,
	REG_SYSCTL11=0x0B,
	REG_SYSCTL12=0x0C,
	REG_SELECT_PDO=0x0D,
	REG_TYPEC_CTL8=0x22,
	REG_TYPEC_CTL9=0x23,
	REG_TYPEC_CTL10=0x24,
	REG_TYPEC_CTL11=0x25,
	REG_TYPEC_CTL12=0x26,
	REG_TYPEC_CTL13=0x27,
	REG_TYPEC_CTL14=0x28,	
	REG_TYPEC_CTL17=0x2B,
	REG_TYPEC_CTL18=0x2C,
	REG_TYPEC_CTL23=0x29,
	REG_TYPEC_CTL24=0x2A,
  //只读寄存器
	REG_STATE_CTL0=0x31,
  REG_STATE_CTL1=0x32,
	REG_STATE_CTL2=0x33,
	REG_STATE_CTL3=0x38,
  REG_TYPEC_STATE=0x34,
	REG_RECEIVED_PDO=0x35,
	REG_VBAT_LSB=0x50,
	REG_VBAT_MSB=0x51,
  REG_VSYS_LSB=0X52,
	REG_VSYS_MSB=0X53,
	REG_IBAT_LSB=0X6E,
	REG_IBAT_MSB=0X6F,
	REG_ISYS_LSB=0X70,
	REG_ISYS_MSB=0X71,
	REG_PSYS_LSB=0x74,
	REG_PSYS_MSB=0x75
	}IP2366REGDef;

typedef enum
	{
	Power_30W,
	Power_45W,
	Power_60W,
	Power_65W,
	Power_100W,
	Power_140W
	}ChargePowerDef;

typedef enum
	{
	IStop_50mA=1,
	IStop_100mA=2,
	IStop_150mA=3,
	IStop_200mA=4,
	IStop_250mA=5,
	IStop_300mA=6,
	IStop_350mA=7,
	IStop_400mA=8,
	IStop_450mA=9,
	IStop_500mA=10
	}IStopConfig;	 //停充电流设置
	
typedef enum
	{
	Recharge_Disable=0, //关闭再充电
	Recharge_0V05=1, //电池电压低于浮充0.05V时再充电
	Recharge_0V1=2, //电池电压低于浮充0.1V时再充电
	Recharge_0V2=3 //电池电压低于浮充0.2V时再充电
	}ReChargeConfig; //再充电配置
	
	
typedef enum
 {
 QuickCharge_PD,
 QuickCharge_HV, //其余高压快充
 QuickCharge_HC, //低电压高电流快充
 QuickCharge_None, //无快充
 QuickCharge_PlaceHolder, //QC枚举里面的占位符，其实没用只是用来复位GUI
 }QuickChargeStateDef;	
 
typedef enum
 {
 Batt_StandBy,
 Batt_PreChage,
 Batt_CCCharge,
 Batt_CVCharge,
 Batt_ChgWait,
 Batt_ChgDone,
 Batt_ChgError,
 Batt_discharging,
 }BatteryStateDef;	
 
typedef enum
	{
	RecvPDO_None, //没有PDO
	RecvPDO_5V, //收到5V
	RecvPDO_9V, //9V
	RecvPDO_12V, //12V
	RecvPDO_15V, //15V
	RecvPDO_20V, //20V
	}RecvPDODef;	
 
typedef struct
	{
	bool EnablePPS2;
	bool EnablePPS1;
	bool Enable20V;
	bool Enable15V;
	bool Enable12V;
	bool Enable9V;
	}PDOBroadcastDef;	
	
typedef enum
 {
 PD_5VMode=1,
 PD_7VMode=2,
 PD_9VMode=3,
 PD_12VMode=4,
 PD_15VMode=5,
 PD_20VMode=6,
 PD_28VMode=7
 }PDStateDef;	
	
typedef struct
	{
	bool IsEnableCharger; //是否开启充电
	int ChargeCurrent; //恒流充电电流设置(mA)
	int PreChargeCurrent; //涓流充电电流设置(mA)
	int FullVoltage; //电池满电电压
	ChargePowerDef ChargePower; //最大输出功率
	}IP2366InputDef;	
	
typedef enum
	{
	VSys_State_Normal,
	VSys_State_OCP,
	VSys_State_Short
	}VSysStateDef;	
	
typedef enum
	{
	VBUS_NoPower,
	VBUS_Normal,
	VBUS_OverVolt,
	}VbusVoltStateDef;	
	
typedef struct
	{
	VbusVoltStateDef VBusState;
	VSysStateDef VSysState;
	}ChipStatDef;	
	
typedef enum
	{
	TypeC_Disconnect=0,
	TypeC_UFP=2,
	TypeC_DFP=1,
	TypeC_DRP=3
	}TypeCRoleDef;	

typedef enum
	{
	VLow_2V5,
	VLow_2V6,
	VLow_2V7,
	VLow_2V8,
	VLow_2V9,
	VLow_3V0,
	VLow_3V1,
	VLow_3V2,
	}VBatLowDef;
	
typedef struct
	{
	float VBUSVolt;
	float VBUSCurrent;
  QuickChargeStateDef QuickChargeState;
	PDStateDef PDState;	 
	bool IsTypeCConnected;
	}IP2366VBUSStateDef;	
	
typedef struct
	{
	bool IsEnableOutput;  //开启对外输出功能
	bool IsEnablePDOut; //是否开启PD快充
	bool IsEnableDPDMOut; //是否开启高压快充协议
	bool IsEnableSCPOut; //是否开启SCP模式
	bool IsEnableHSCPOut; //开启高压SCP
	}IP2366OutConfigDef;

//电流回读参数配置
#define BusCurrentCalFactor 0.985	//IP2366读回来的电流修正值
	
//峰值电流参数配置
#if (BATTCOUNT == 4)	

#define IP2366_ICCMAX 13000 //四串版本是13000mA
	
#elif (BATTCOUNT == 3 || BATTCOUNT == 2)
	
#define IP2366_ICCMAX 16000 //2-3串版本是16000mA
	
#else
	
#error "Invaild Battery Count Settings!"	
	
#endif
	
//特殊数值运算函数
bool IP2366_QueryCurrentStateIsACC(BatteryStateDef IN); //查询电池状态是否需要库仑计统计	
	
//函数
bool IP2366_GetVRecharge(float *Vrecharge); //获取再充电电压
bool IP2366_getCurrentChargeParam(int *Istop,float *Vstop); //获取当前芯片的充电参数(浮充电压和停充电流等)
bool IP2366_GetCurrentPeakCurrent(int *Result); //获取峰值电流
bool IP2366_SetOTPSign(void); //设置重载检测标记
bool IP2366_DetectIfChipReset(bool *IsReset); //检查芯片是否复位
bool IP2366_SetVLowVolt(VBatLowDef Vlow); //设置低压保护
bool IP2366_GetFirmwareTimeStamp(char TimeStamp[5]);	//获取时间戳
bool IP2366_SetInputState(IP2366InputDef * Cfg); //设置输入状态
bool IP2366_SetOutputState(IP2366OutConfigDef * CFG); //设置输出状态	
bool IP2366_GetRecvPDO(RecvPDODef *PDOResult);	//获取输入的PDO状态
bool IP2366_GetVBUSState(IP2366VBUSStateDef * State); //获取VBUS状态
bool IP2366_GetChargerState(BatteryStateDef *State);	//获取充电状态
bool IP2366_DetectIfPresent(void);	//监测IP2366是否存在
bool IP2366_SetTypeCRole(TypeCRoleDef Role); //设置TypeC的模式
bool IP2366_GetIfInputConnected(void);	//获取IP2366是否为输入连接
void IP2366_SetICCMax(int TargetCurrent); //反复设置iset寄存器直到电流一样为止
bool IP2366_ReadChipState(ChipStatDef *State);	//获取一部分芯片的信息
bool IP2366_SetPDOBroadCast(PDOBroadcastDef *PDOCfg);	//设置PDO广播
bool IP2366_EnableDCDC(bool IsEnableCharger,bool IsEnableDischarge); //配置充电器和放电	
bool IP2366_SetReChargeParam(ReChargeConfig Vrecharge,IStopConfig IStop); //设置再充电电流	
bool IP2366_UpdataChargePower(ChargePowerDef Power);	//更新充电功率
bool IP2366_UpdateFullVoltage(int Volt); //更新充电电压(传入的单位为mV)
void IP2366_ClearOCFlag(void); //清除OC Flag

#endif
