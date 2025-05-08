#ifndef _ADAsync_
#define _ADAsync_

//ADC异步引所需的枚举值
typedef enum
	{
	ADC_SubmitQueue, //提交转换队列	
  ADC_SubmitChFromQueue, //向ADC转换线程提交队列内的任务
	ADC_WaitMissionDone, //等待任务完成
	ADC_ConvertComplete //转换完毕	
	}ADCAsyncStateDef; //ADC异步转换状态机处理

typedef struct
	{
	long avgbuf;
	int Count;
	char Ch;
	bool IsMissionProcessing; //是否正在处理任务
	}ADCConvertTemp;

//ADC异步引擎配置
#define ADCConvertQueueDepth 5 //ADC转换任务队列深度	
#define ADCAverageCount 10 //ADC对于每个转换任务的平均次数	

//ADC转换引擎的队列配置
#include "Pindefs.h"
#include "ADCCfg.h"	
	
code char ADCChQueue[ADCConvertQueueDepth]=
	{
	ADC_INTVREFCh, //先转换VREF
	NTCInputAIN,//然后转换温度	
	OPFBAIN, //FB注入恒流运放的输出电压
	VBATInputAIN, //电池电压
	VOUTFBAIN //最后转换输出电压
	};
	
#endif
