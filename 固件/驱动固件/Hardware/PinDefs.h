/************************************************************************************/
/** \file PinDefs.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个头文件为底层的硬件定义文件，定义了整个工程内所有外设对应的硬件PIN和
模拟输入ADC通道的映射关系。

**	History: 
				2025年12月26日 10:05 新增电池电压检测引脚和输出检测引脚的GPIO Port SFR声明以便于
				                     ADC驱动直接对相关引脚进行设置。
														 
				2025年12月20日 Initial Release
**	
/************************************************************************************/
#ifndef PINDEFS
#define PINDEFS
/************************************************************************************/
/* Include files */
/************************************************************************************/
#include "GPIOCfg.h"

/************************************************************************************
这个文件包含工程内所有用到的IO引脚的引脚位置描述，已经按照引脚功能类型进行了分组，如果
您计划将该固件用于其他驱动，您可以根据硬件调整引脚定义。
************************************************************************************/

/******************** 以下是主DCDC和辅助DCDC以及输出控制的引脚 *********************/
#define AUXENIOP GPIO_PORT_2
#define AUXENIOG 2
#define AUXENIOx GPIO_PIN_6	//辅助6.0V使能输出(P2.6)


#define BOOSTRUNIOP GPIO_PORT_3
#define BOOSTRUNIOG 3
#define BOOSTRUNIOx GPIO_PIN_2	//主Boost运行使能输出(P3.2)


#define LEDNegMOSIOP GPIO_PORT_2
#define LEDNegMOSIOG 2
#define LEDNegMOSIOx GPIO_PIN_5		//LED负极MOSFET(P2.5)


/******************* 以下是负责CC/CV整定电压生成的PWMDAC的引脚 *********************/
#define PWMDACENIOP GPIO_PORT_2
#define PWMDACENIOG 2
#define PWMDACENIOx GPIO_PIN_3 	//恒流(CC)整定PWMDAC使能(P2.3)

#define PWMDACIOP GPIO_PORT_2
#define PWMDACIOG 2
#define PWMDACIOx GPIO_PIN_2		//恒流(CC)整定PWMDAC输出(P2.2)

#define PreChargeDACIOP GPIO_PORT_2
#define PreChargeDACIOG 0
#define PreChargeDACIOx GPIO_PIN_2		//恒压预充(CV)整定PWMDAC输出(P0.2)


/************************************************************************************
以下是系统的模拟输入引脚，包括电池电压测量、输出电压测量、运放恒流状态反馈、负责温度
测量的NTC输入以及NTC控制
************************************************************************************/
#define NTCInputIOG 0
#define NTCInputIOx GPIO_PIN_5 
#define NTCInputAIN 5						//NTC输入(P0.5,AN5)

#define VOUTFBIOP GPIO_PORT_3
#define VOUTFBIOG 3
#define VOUTFBIOx GPIO_PIN_1
#define VOUTFBAIN 13						//输出电压反馈引脚(P3.1,AN13)


#define OPFBIOG 0
#define OPFBIOx GPIO_PIN_1
#define OPFBAIN 1								//运放恒流状态反馈引脚(P0.1,AN1)

#define VBATInputIOP GPIO_PORT_3
#define VBATInputIOG 3
#define VBATInputIOx GPIO_PIN_0
#define VBATInputAIN 22					//电池电压检测引脚(P3.0,AN22)


#define NTCENIOP GPIO_PORT_0
#define NTCENIOG 0
#define NTCENIOx GPIO_PIN_3			//NTC检测使能引脚(P0.3)


/****************** 以下是负责按键小板部分的引脚(指示灯和按键) ********************/
#define SideKeyGPIOP GPIO_PORT_0
#define SideKeyGPIOG 0
#define SideKeyGPIOx GPIO_PIN_0		//侧按按键(P0.0)


#define RedLEDIOP GPIO_PORT_1
#define RedLEDIOG 1
#define RedLEDIOx GPIO_PIN_4		//红色指示灯(P1.4)	


#define GreenLEDIOP GPIO_PORT_1
#define GreenLEDIOG 1
#define GreenLEDIOx GPIO_PIN_3		//绿色指示灯(P1.3)



/********************* 以下是其他特殊功能的引脚或者保留引脚 ***********************/

#define SYSHBLEDIOP GPIO_PORT_0
#define SYSHBLEDIOG 0
#define SYSHBLEDIOx GPIO_PIN_4			//心跳LED(P0.4)






#endif /* PINDEFS */

/*************************  End Of File  ***********************/
