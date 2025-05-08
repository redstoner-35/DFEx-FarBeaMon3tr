#ifndef _INA226_
#define _INA226_

//宏定义
#define BusVoltLSB 1.25 //总线电压的1LSB数值，单位mV
#define MaxmiumCurrent 15 //预期的最大电流（15A）
#define CurrentLSB (double)MaxmiumCurrent/PMBUS_2NPowCalc(15) //LSB=总线最大电流除以2的15次方
#define PowerLSB (float)25*CurrentLSB //功率读数的LSB，为25倍的电流LSB

//内部包含
#include <stdbool.h>

typedef enum
	{
	INA226_AvgCount_1,
  INA226_AvgCount_4,
  INA226_AvgCount_16,
  INA226_AvgCount_64,
  INA226_AvgCount_128,
  INA226_AvgCount_256,
  INA226_AvgCount_512,
  INA226_AvgCount_1024   
	}INA226AvgCountDef;

typedef enum
	{
	INA226_Conv_140uS,
  INA226_Conv_204US,
  INA226_Conv_332US,
  INA226_Conv_588US,
  INA226_Conv_1100US,
  INA226_Conv_2116US,
  INA226_Conv_4156US,
  INA226_Conv_8244US,
	}INA226ConvTimeDef;

//模式枚举
typedef enum
{
INA226_PowerDown,
INA226_Trig_BCurrent,
INA226_Trig_BVoltage,
INA226_Trig_Both,	
INA226_ADC_Off,
INA226_Cont_BCurrent,
INA226_Cont_BVoltage,
INA226_Cont_Both	
}INA226ModeDef;

//初始化函数输出
typedef enum
{
A226_Init_OK=0,
A226_Error_SMBUS_NACK=1,
A226_Error_CalibrationReg=2,
A226_Error_ProgramCalReg=3,
A226_Error_ProgramReg=4,
A226_Error_SetAlertCfg=5,
A226_Error_NotGenuineDevice=6
}INA226InitStatDef;

//报警配置
typedef enum
{
A226_AlertDisable=0, //关闭所有警报
A226_EnableOCP=0x8000,  //开启电流过高报警[SOL=1]
A226_EnableUCP=0x4000,  //开启电流过低报警[SUL=1]
A226_EnableOVP=0x2000,  //开启电压过高报警[BOL=1]
A226_EnableUVP=0x1000,  //开启电压过低报警[BUL=1]
A226_EnableOPP=0x800, //开启过功率报警[BUL=0]
}INA226AlertDef;

//获取参数用的结构体
typedef struct
{
float BusVolt;
float BusCurrent;
float BusPower;
}INADoutSreDef;

//初始化226用的结构体
typedef struct
{
float ShuntValue;//检流电阻的阻值，单位为毫欧（mR）
INA226ModeDef ConvMode;//转换模式
INA226ConvTimeDef VBUSConvTime;
INA226ConvTimeDef IBUSConvTime;	  //总线电压电流的采样时间
INA226AvgCountDef AvgCount; //平均时间	
bool IsEnableAlertLatch; //是否开启报警引脚的锁存
bool IsAlertPinInverted; //是否反向报警输出的极性
INA226AlertDef AlertConfig; //告警配置
}INAinitStrdef;

//函数
INA226InitStatDef INA226_INIT(INAinitStrdef * INAConf);
bool INA226_GetBusInformation(INADoutSreDef *INADout);
bool INA226_SetAlertRegister(unsigned int Value);
bool INA226_QueueIfGaugeCanReady(void);

#endif
