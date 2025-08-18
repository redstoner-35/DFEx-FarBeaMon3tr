#ifndef _LVProt_
#define _LvProt_

#include "stdbool.h"

//参数配置
#define BatteryAlertDelay 10 //电池警报延迟	
#define BatteryFaultDelay 2 //电池故障强制跳档/关机的延迟

//函数
void BatteryLowAlertProcess(bool IsNeedToShutOff,ModeIdxDef ModeJump); //普通挡位的警报函数
void BattAlertTIMHandler(void); //电池低电量报警处理函数

#endif
