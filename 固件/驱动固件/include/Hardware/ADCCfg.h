#ifndef ADC
#define ADC

#include "stdbool.h"

//结构体
typedef struct
	{
  int Systemp; //系统温度
	float OutputVoltage; //DCDC输出电压(V)
	float BatteryVoltage; //等效单节电池电压(V)
	float RawBattVolt; //原始的电池电压(V)
	float MCUVDD; //单片机的VDD
	float FBInjectVolt; //FB注入运放的输出电压(用于判断是否恒流)
	bool IsNTCOK; //NTC是否OK
	}ADCResultStrDef;

//ADC基准电压和特殊基准通道定义
#define ADCVREF 2.00 //ADC片内基准LDO的电压
#define ADC_INTVREFCh 31 //ADC连通到片内带隙基准的特殊通道定义	
#define ADCBGVREF 1.20 //ADC特殊通道带隙基准的电压	
	
//ADC寄存器操作宏定义	
#define ADC_StartConv() ADCON0|=0x02 //ADC启动转换
#define ADC_GetIfStillConv()	ADCON0&0x02  //检查ADC是否仍然在转换需要继续等
#define ADC_ReadConvResult()	(ADRESL|(ADRESH<<8)) //读取ADC转换的寄存器结果
#define ADC_EnableCmd() ADCON1|=0x80  //使能ADC IP
#define ADC_DisableCmd() ADCON1&=0x7F  //关闭ADC IP	
#define ADC_SetVREFReg(IsVDD) ADCLDO=(!IsVDD?0xA0:0x00) //设置基准
#define ADC_IsUsingIVREF() ADCLDO&0x80 //检测ADC是否在使用片内基准	
#define ADC_CheckIfChInvalid(Ch) (Ch<0||(Ch>22&&Ch<ADC_INTVREFCh)) //检查通道参数是否合法	
	
//ADC引擎操作宏定义
#define EnableADCAsync() IsNotAllowAsync=false
#define DisableADCAsync() IsNotAllowAsync=true

//ADC外部采集的参数配置
#define VoutUpperResK 680
#define VoutLowerResK 56 //输出检测分压的上下拉电阻
#define VBattUpperResK 680
#define VBattLowerResK 100 //电池检测分压的上下拉电阻
#define NTCUpperResValueK 330 //NTC热敏电阻的上拉阻值

//绝对值宏
#define fabs(x) x>0?x:x*-1  //浮点求绝对值

//外部ADC数据引用
extern ADCResultStrDef Data;
extern xdata bool IsNotAllowAsync; //是否启用异步转换

//外部函数
void ADC_Init(void);
void ADC_DeInit(void);
void SystemTelemHandler(void);
void BatteryTelemHandler(void);

#endif