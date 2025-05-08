#ifndef Delay
#define Delay

#include "ht32.h"

//systick延时函数
void delay_init(void);
void delay_ms(u16 ms);
void delay_Second(u8 sec);
void delay_us(u16 us);

//系统GPTM0 4Hz心跳定时器函数
void EnableHBTimer(void);
void DisableHBTimer(void);
void CheckHBTimerIsEnabled(void);
void CheckIfHBTIMStart(void);

#endif
