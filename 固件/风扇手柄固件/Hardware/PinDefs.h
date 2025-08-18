#ifndef PINDEFS
#define PINDEFS

//内部包含
#include "GPIOCfg.h"

/************************************************************************************
系统未使用IO Map
P0.2 P1.3-1.4 P2.6 P3.0-3.2
************************************************************************************/
#define P0UnusedPIN 0x01<<2
#define P1UnusedPIN ((0x01<<3)|(0x01<<4))
#define P2UnusedPIN 0x01<<6
#define P3UnusedPIN (0x01|(0x01<<1)|(0x01<<2))

/************************************************************************************
以下是系统的常规GPIO引脚，用于控制外部外设切换功能以及PIN Strap读入
************************************************************************************/

#define PinStrapIOP GPIO_PORT_0
#define PinStrapIOG 0
#define PinStrapIOx GPIO_PIN_0 //读取系统风扇配置的PIN Strap(P0.0)

#define DCDCENIOP GPIO_PORT_0
#define DCDCENIOG 0
#define DCDCENIOx GPIO_PIN_5 //风扇升压DCDC电源的使能引脚(P0.5)

#define FANPWRENIOP GPIO_PORT_2
#define FANPWRENIOG 2
#define FANPWRENIOx GPIO_PIN_3 //风扇电源输出的使能引脚(P2.3)


/************************************************************************************
以下是系统的PWM输出引脚，用于对外输出PWM控制风扇速度，DCDC输出电压等等
************************************************************************************/

#define CVDACIOP GPIO_PORT_0
#define CVDACIOG 0
#define CVDACIOx GPIO_PIN_4   //负责控制系统的输出电压调整的PWM引脚（P0.4）

#define FanPWMIOP GPIO_PORT_0
#define FanPWMIOG 0
#define FanPWMIOx GPIO_PIN_3  //负责控制4PIN PWM风扇的PWM输出引脚（P0.3）

/************************************************************************************
以下是系统的模拟输入引脚，例如电池电压测量等
************************************************************************************/

#define VBATInputIOG 2
#define VBATInputIOx GPIO_PIN_5
#define VBATInputAIN 11					//电池电压检测引脚(P2.5,AN11)

/****************** 以下是负责按键小板部分的引脚(指示灯和按键) ********************/
#define SideKeyGPIOP GPIO_PORT_2
#define SideKeyGPIOG 2
#define SideKeyGPIOx GPIO_PIN_2 	//侧按按键(P2.2)


#define RedLEDIOP GPIO_PORT_2
#define RedLEDIOG 2
#define RedLEDIOx GPIO_PIN_4		//红色指示灯(P2.4)	


#define GreenLEDIOP GPIO_PORT_0
#define GreenLEDIOG 0
#define GreenLEDIOx GPIO_PIN_1		//绿色指示灯(P0.1)

#endif
