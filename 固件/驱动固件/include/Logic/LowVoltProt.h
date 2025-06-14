#ifndef _LVProt_
#define _LvProt_

#include "stdbool.h"

//参数配置
#define BatteryMaximumTurboVdroop 1.2  //极亮启动过程中，电池最大允许的和运行前的压差(V)
#define BatteryAlertDelay 10 //电池警报延迟	
#define BatteryFaultDelay 2 //电池故障强制跳档/关机的延迟
#define TurboILIMTryCDTime 4 //每次极亮尝试下调电流的冷却时间（单位是1/8秒）

//外部引用
extern xdata int TurboILIM; //极亮电流限制
extern xdata float BeforeRawBattVolt; //极亮前电压的采样

//函数
void BatteryLowAlertProcess(bool IsNeedToShutOff,ModeIdxDef ModeJump); //普通挡位的警报函数
void RampLowVoltHandler(void); //无极调光的专属处理
void BattAlertTIMHandler(void); //电池低电量报警处理函数
void TurboLVILIMProcess(void); //极亮专属的电流运行值的功能
void RampRestoreLVProtToMax(void); //每次开机进入无级模式时尝试恢复限流
void CalcTurboILIM(void); //计算极亮电流挡位的限流值

#endif
