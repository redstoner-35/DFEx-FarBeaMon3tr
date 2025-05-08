#ifndef _ADC_
#define _ADC_

#include "stdbool.h"

//转换输出结构体
typedef struct
	{
	bool IsNTCOK; //NTC是否正常
	float Systemp; //系统温度
  float Vbatt; //电池电压
	float Ibatt; //电池电流
	float IVREF; //电流检测部分的VREF测试
	}ADCOutTypeDef;

//ADC参数配置
#define VREF 2.50 //基准电压
#define ADCAvg 5  //ADC平均次数
#define ADCConvTimeOut 2000 //ADC转换超时时间(单位2mS)
#define MinimumCurrentFactor 0.05 //日志系统不统计的电流	
	
//温度电压和电流监测配置	
#define SenseAmpGain 50 //检流放大器的增益，单位为(V/V)	
#define SenseShuntmOhm 1.0 //检流电阻阻值(mR)	
#define VsenseUpRes 100 //
#define VsenseLowRes 10 //电池电压测量的分压电阻阻值(K)	
#define NTCUpperResValueK	10 //NTC热敏电阻上拉阻值(K)	
#define NTCT0 25 //NTC标准阻值标定温度
#define NTCT0ResK 15 //NTC在标定温度下的阻值(K)
#define NTCBValue 3450 //NTC B值
#define NTCTRIM -3 //温度修正值(℃)
	
#if (BATTCOUNT == 4)	
	
#define BattCellCount 4
	
#elif (BATTCOUNT == 3)

#define BattCellCount 3

#elif (BATTCOUNT == 2)

#define BattCellCount 2

#else
  #error "Undefined Battery Cell Count"
#endif
	
/*下面的自动Define负责处理ADC的IO，ADC通道定义，不允许修改！*/
#define ISenseOut_IOP STRCAT2(AFIO_PIN_,ISenseOut_IOPN)
#define ISenseREF_IOP STRCAT2(AFIO_PIN_,ISenseREF_IOPN) 
#define TempVBatt_IOP STRCAT2(AFIO_PIN_,TempVBatt_IOPN) 

#define _ISenseOut_Ch STRCAT2(ADC_CH_,ISenseOut_IOPN)
#define _ISenseREF_Ch STRCAT2(ADC_CH_,ISenseREF_IOPN)
#define _TempVBatt_Ch STRCAT2(ADC_CH_,TempVBatt_IOPN)

//外部参考
extern ADCOutTypeDef ADCO;

//函数
bool ADC_GetResult(void);
void InternalADC_Init(void); //ADC初始化

#endif
