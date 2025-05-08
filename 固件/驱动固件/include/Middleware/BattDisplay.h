#ifndef _BattDs_
#define _BattDs_

//电池检测配置
#define VBattAvgCount 40 //等效单节电池电压数据的平均次数(用于内部逻辑的低压保护,电量显示和电量不足跳档)
#define LowVoltStrobeGap 15 //触发低电压提示之后每隔多久闪一次

//电池电压平均计算结构体声明
typedef struct
	{
	int Min;
  int Max;
	long AvgBuf;
	unsigned char Count;
	}AverageCalcDef;	

//状态变量enum
typedef enum
	{
	Battery_Plenty, //电池电量充足
	Battery_Mid, //电池电量较为充足
	Battery_Low, //电池电量不足
	Battery_VeryLow //电池电量严重不足
	}BattStatusDef;

typedef enum
	{
  BattVdis_Waiting, //等待显示阶段
	BattVdis_PrepareDis, //准备显示
	BattVdis_DelayBeforeDisplay, //延迟一段时间
	BattVdis_Show10V, //显示十位
	BattVdis_Gap10to1V, //十位和个位之间的等待
	BattVdis_Show1V, //显示个位
	BattVdis_Gap1to0_1V, //个位和十分位之间的等待
	BattVdis_Show0_1V, //显示小数点后一位(0.1V)
	BattVdis_WaitShowChargeLvl, //等待一段时间后显示当前电量
	BattVdis_ShowChargeLvl, //显示电池电量的等待
	}BattVshowFSMDef; //电池电量显示处理


//外部参考
extern bit IsBatteryAlert; //电池低电警告发生
extern bit IsBatteryFault; //电池低电量故障发生
extern BattStatusDef BattState; //电池状态
extern xdata float Battery; //滤波之后的电池电压
extern xdata BattVshowFSMDef VshowFSMState; //状态机状态	
	
//函数
void BattDisplayTIM(void); //电池电量显示函数处理
void TriggerVshowDisplay(void); //启动电池电压显示
void DisplayVBattAtStart(void);
void BatteryTelemHandler(void);  //电池测量和指示灯控制
bit LowPowerStrobe(void); //低电量提示闪
	
#endif
