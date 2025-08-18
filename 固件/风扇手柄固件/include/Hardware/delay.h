#ifndef Delay
#define Delay

//宏定义
//#define EnableMicroSecDelay //是否启用微秒延时
//#define EnableHBCheck //是否开启心跳检查
#define UseUnifiedSystemTimeBase   //启用统一的系统初始化函数

//系统心跳定时除能和flag
extern volatile bit SysHFBitFlag;
#define DisableSysHBTIM() T2CON=0x00;IE&=~0x20; //禁用系统心跳定时器，直接关闭定时器并除能中断

//检查心跳定时器是否启动
#ifdef EnableHBCheck
void CheckIfHBTIMIsReady(void);
#endif



//启动系统心跳计时器和延时计时器的初始化
#ifdef UseUnifiedSystemTimeBase
  
	//使用统一初始化函数
	void StartSystemTimeBase(void);

#else

	//使用独立初始化函数
	void EnableSysHBTIM(void);
	void delay_init();

#endif

//较长的延时
void delay_ms(int ms);
void delay_sec(int sec);

//微秒级别短延时
#ifdef EnableMicroSecDelay
void delay_us(int us);
#endif

#endif
