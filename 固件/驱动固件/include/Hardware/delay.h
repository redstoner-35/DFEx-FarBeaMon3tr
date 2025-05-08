#ifndef Delay
#define Delay

//宏定义
//#define EnableMicroSecDelay //是否启用微秒延时
//#define EnableHBCheck //是否开启心跳检查

//系统8Hz心跳定时初始化
extern volatile bit SysHFBitFlag;
void SetSystemHBTimer(bit IsEnable);

//检查心跳定时器是否启动
#ifdef EnableHBCheck
void CheckIfHBTIMIsReady(void);
#endif

//延时初始化部分和较长的延时
void delay_init();
void delay_ms(int ms);
void delay_sec(int sec);

//微秒级别短延时
#ifdef EnableMicroSecDelay
void delay_us(int us);
#endif

#endif
