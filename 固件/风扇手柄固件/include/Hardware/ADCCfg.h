#ifndef ADC
#define ADC

#include "stdbool.h"

//结构体
typedef struct
	{
	float RawBattVolt;   //未经过计算的原始值
	float BatteryVoltage; //计算出来的等效电池电压(V)
	float MCUVDD; //单片机的VDD
	}ADCResultStrDef;


//ADC配置宏定义
#define ADCAIN14_VDD   (3 << 0)  //AIN14 = VDD 
#define ADCAIN14_4V    (2 << 0)  //AIN14 = 4.0V 
#define ADCAIN14_3V    (1 << 0)  //AIN14 = 3.0V 
#define ADCAIN14_2V    (0 << 0)  //AIN14 = 2.0V 
#define ADCInRefVDD    (1 << 2)  //internal reference from VDD 
#define ADCExHighRef   (1 << 7)  //high reference from AVREFH/P2.0 
#define ADCSpeedDiv16  (0 << 4)  //ADC clock = fosc/16 
#define ADCSpeedDiv8   (1 << 4)  //ADC clock = fosc/8 
#define ADCSpeedDiv1   (2 << 4)  //ADC clock = fosc/1 
#define ADCSpeedDiv2   (3 << 4)  //ADC clock = fosc/2 
#define ADCChannelEn   (1 << 6)  //enable ADC channel 
#define SelAIN5         (5 << 0)  //select ADC channel 5 
#define ADCStart        (1 << 6)  //start ADC conversion 
#define ADCEn         (1 << 7)  //enable ADC 
#define EADC         (1 << 1)  //enable ADC interrupt 
#define ClearEOC        0xDF; 

//ADC配置
#define ADCAverageCount 6

//外部ADC数据引用
extern ADCResultStrDef Data;

//函数
void ADC_Init(void);
void ADC_DeInit(void);
void SystemTelemHandler(void);
void BatteryTelemHandler(void);

#endif