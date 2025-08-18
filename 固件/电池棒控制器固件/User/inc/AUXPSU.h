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
bool AUXPSU_SetIPDState(bool State);	//设置强制下拉C口强迫适配器输出的取电电阻的状态
bool AUXPSU_ConnectTCtoIP2366(void); 	//给磁保持继电器发送指令让CC线切换到IP2366
bool AUXPSU_ConnectTCtoIPD(void);  	//给磁保持继电器发送指令让CC线切换到下拉强制取电
void AUXPSU_Mgmt(void);   //系统正常工作阶段切换
void AUXPSU_SwitchToPassThrough(void);
bool AUXPSU_SetTypeCFVoutState(bool State);
void AUXPSU_DetectIfCPortTriggerPresent(void); //检测C口控制的5.1K硬件是否正常
void ForceEnableAdvPM(void);	//强制启用电源管理（进入安全模式用）
	
#endif
