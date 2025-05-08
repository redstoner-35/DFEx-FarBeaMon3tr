#ifndef _GPIOCFG_
#define _GPIOCFG_

//GPIO定义
#define GPIO_PORT_0 P0
#define GPIO_PORT_1 P1
#define GPIO_PORT_2 P2
#define GPIO_PORT_3 P3
#define GPIOMask(x) 1<<x

//系统未使用的IO进行配置
void UnusedIOConfig(void);

#endif
