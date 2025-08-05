#ifndef _AUXPSU_
#define _AUXPSU_

#include <stdbool.h>

//定义
typedef enum
	{
	AUXPSU_ALLOFF,
	AUXPSU_Passthrough,
	AUXPSU_Buck,
	}AUXPSUStateDef;

//外部参考
extern bool IsCPortTriggerOK; //标志位，C口诱骗5.1K电阻存在检测	
	
//函数
void AUXPSU_Mgmt(void);   //系统正常工作阶段切换
void AUXPSU_SwitchToPassThrough(void);
bool AUXPSU_SetTypeCFVoutState(bool State);
void AUXPSU_DetectIfCPortTriggerPresent(void); //检测C口控制的5.1K硬件是否正常
	
#endif
