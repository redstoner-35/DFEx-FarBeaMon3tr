/*******************************************************************************
* Copyright (C) 2019 China Micro Semiconductor Limited Company. All Rights Reserved.
*
* This software is owned and published by:
* CMS LLC, No 2609-10, Taurus Plaza, TaoyuanRoad, NanshanDistrict, Shenzhen, China.
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software contains source code for use with CMS
* components. This software is licensed by CMS to be adapted only
* for use in systems utilizing CMS components. CMS shall not be
* responsible for misuse or illegal use of this software for devices not
* supported herein. CMS is providing this software "AS IS" and will
* not be responsible for issues arising from incorrect user implementation
* of the software.
*
* This software may be replicated in part or whole for the licensed use,
* with the restriction that this Disclaimer and Copyright notice must be
* included with each copy of this software, whether used in part or whole,
* at all times.
*/

/*****************************************************************************/
/** \file gpio.h
**
** 
**
** History:
** 
*****************************************************************************/
#ifndef __GPIO_H__
#define __GPIO_H__
/*****************************************************************************/
/* Include files */
/*****************************************************************************/
#include "cms8s6990.h"
/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define') */
/*****************************************************************************/
/*----------------------------------------------------------------------------
 **GPIO引脚
 ---------------------------------------------------------------------------*/
#define  GPIO_PIN_0_MSK				(0x01)		/* GPIO Pin 0 mask */
#define  GPIO_PIN_1_MSK				(0x02)		/* GPIO Pin 1 mask */
#define  GPIO_PIN_2_MSK				(0x04)		/* GPIO Pin 2 mask */
#define  GPIO_PIN_3_MSK				(0x08)		/* GPIO Pin 3 mask */
#define  GPIO_PIN_4_MSK				(0x10)		/* GPIO Pin 4 mask */
#define  GPIO_PIN_5_MSK				(0x20)		/* GPIO Pin 5 mask */
#define  GPIO_PIN_6_MSK				(0x40)		/* GPIO Pin 6 mask */
#define  GPIO_PIN_7_MSK				(0x80)		/* GPIO Pin 7 mask */

#define  GPIO_PIN_0					(0x00)		/* GPIO Pin 0 number*/
#define  GPIO_PIN_1					(0x01)		/* GPIO Pin 1 number */
#define  GPIO_PIN_2					(0x02)		/* GPIO Pin 2 number */
#define  GPIO_PIN_3					(0x03)		/* GPIO Pin 3 number */
#define  GPIO_PIN_4					(0x04)		/* GPIO Pin 4 number */
#define  GPIO_PIN_5					(0x05)		/* GPIO Pin 5 number */
#define  GPIO_PIN_6					(0x06)		/* GPIO Pin 6 number */
#define  GPIO_PIN_7					(0x07)		/* GPIO Pin 7 number */

#define  GPIO0						(0x00)
#define  GPIO1						(0x01)
#define  GPIO2						(0x02)
#define  GPIO3						(0x03)

#define  GPIO_P00					(0x00)
#define  GPIO_P01					(0x01)
#define  GPIO_P02					(0x02)
#define  GPIO_P03					(0x03)
#define  GPIO_P04					(0x04)
#define  GPIO_P05					(0x05)
#define  GPIO_P13					(0x13)
#define  GPIO_P14					(0x14)
#define  GPIO_P15					(0x15)
#define  GPIO_P16					(0x16)
#define  GPIO_P17					(0x17)
#define  GPIO_P21					(0x21)
#define  GPIO_P22					(0x22)
#define  GPIO_P23					(0x23)
#define  GPIO_P24					(0x24)
#define  GPIO_P25					(0x25)
#define  GPIO_P26					(0x26)
#define  GPIO_P30					(0x30)
#define  GPIO_P31					(0x31)
#define  GPIO_P32					(0x32)
#define  GPIO_P35					(0x35)
#define  GPIO_P36					(0x36)

/*----------------------------------------------------------------------------
 **GPIO 引脚模式设置
---------------------------------------------------------------------------*/
typedef enum
	{
	GPIO_Input_Floating, //输入浮空
	GPIO_IPU, //输入带上拉
	GPIO_IPD, //输入带下拉
	GPIO_Out_PP, //推挽输出
	GPIO_Out_OD //三态输入输出
	}GPIOModeDef;

typedef enum
	{
	GPIO_Slow_Slew,
	GPIO_Fast_Slew //
	}GPIOSlewRateDef;

typedef enum
	{
	GPIO_Low_Current,
	GPIO_High_Current, //GPIO电流配置
	}GPIOCurrentDef;
	
typedef struct
	{
	GPIOModeDef Mode;
	GPIOSlewRateDef Slew;
	GPIOCurrentDef DRVCurrent;
	}GPIOCfgDef;
/*----------------------------------------------------------------------------
 **GPIO 引脚中断模式
---------------------------------------------------------------------------*/
#define GPIO_Int_Disable 0x00 //关闭中断
#define GPIO_Int_Rising 0x01 //上升沿触发
#define GPIO_Int_Falling 0x02 //下降沿触发
#define GPIO_Int_DualEdge 0x03 //双边沿触发

/*----------------------------------------------------------------------------
 ** IO引脚复用模式
---------------------------------------------------------------------------*/
#define GPIO_AF_GPIO 0x00 //(配置为GPIO)
#define GPIO_AF_Analog 0x01 //(配置为模拟输入)
#define GPIO_AF_CC0 0x04 //(定时器2比较输出通道0)
#define GPIO_AF_CC1 0x05 //(定时器2比较输出通道1)
#define GPIO_AF_CC2 0x06 //(定时器2比较输出通道2)
#define GPIO_AF_CC3 0x07 //(定时器2比较输出通道3)
#define GPIO_AF_U0TXD 0x08 //(串口0发送)
#define GPIO_AF_U0RXD 0x09 //(串口0接收)
#define GPIO_AF_U1TXD 0x0A //(串口1发送)
#define GPIO_AF_U1RXD 0x0B //(串口1接收)
#define GPIO_AF_SCL 0x0C //(内部IIC SCL)
#define GPIO_AF_SDA 0x0D //(内部IIC SDA)
#define GPIO_AF_SPINSS 0x0E //(SPI从机模式片选信号)
#define GPIO_AF_SPICLK 0x0F //(SPI时钟信号)
#define GPIO_AF_SPIMOSI 0x10 //(SPI主到从数据链路)
#define GPIO_AF_SPIMISO 0x11 //(SPI从到主数据链路)
#define GPIO_AF_PWMCH0 0x12 //(PWM通道0)
#define GPIO_AF_PWMCH1 0x13 //(PWM通道1)
#define GPIO_AF_PWMCH2 0x14 //(PWM通道2)
#define GPIO_AF_PWMCH3 0x15 //(PWM通道3)
#define GPIO_AF_PWMCH4 0x16 //(PWM通道4)
#define GPIO_AF_PWMCH5 0x17 //(PWM通道5)
#define GPIO_AF_BEEP 0x18 //(蜂鸣器驱动输出)
#define GPIO_AF_ACMP0_O 0x1A //(模拟比较器比较输出0)
#define GPIO_AF_ACMP1_O 0x1B //(模拟比较器比较输出1)

/******************************************************************************
 ** \brief	 GPIO_SetMUXMode
 **			 设置GPIO复用功能的相应模式
 ** \param [in] Port  ： GPIO0、GPIO1、GPIO2、GPIO3
 **			    PinNum:  GPIO_PIN_0~GPIO_PIN_7 (0~7)		 
 **			    Mode: GPIO_AF_GPIO(配置为GPIO)
 **			          GPIO_AF_Analog(配置为模拟输入)
 **			          GPIO_AF_CC0(定时器2比较输出通道0)
 **			          GPIO_AF_CC1(定时器2比较输出通道1)
 **			          GPIO_AF_CC2(定时器2比较输出通道2)
 **			          GPIO_AF_CC3(定时器2比较输出通道3)
 **			          GPIO_AF_U0TXD(串口0发送)
 **			          GPIO_AF_U0RXD(串口0接收)
 **			          GPIO_AF_U1TXD(串口1发送)
 **			          GPIO_AF_U1RXD(串口1接收)
 **			          GPIO_AF_SCL(内部IIC SCL)
 **			          GPIO_AF_SDA(内部IIC SDA)
 **			          GPIO_AF_SPINSS(SPI从机模式片选信号)
 **			          GPIO_AF_SPICLK(SPI时钟信号)
 **			          GPIO_AF_SPIMOSI(SPI主到从数据链路)
 **			          GPIO_AF_SPIMISO(SPI从到主数据链路)
 **			          GPIO_AF_PWMCH0(PWM通道0)
 **			          GPIO_AF_PWMCH1(PWM通道1)
 **			          GPIO_AF_PWMCH2(PWM通道2)
 **			          GPIO_AF_PWMCH3(PWM通道3)
 **			          GPIO_AF_PWMCH4(PWM通道4)
 **			          GPIO_AF_PWMCH5(PWM通道5)
 **			          GPIO_AF_BEEP(蜂鸣器驱动输出)
 **			          GPIO_AF_ACMP0_O(模拟比较器比较输出0)
 **			          GPIO_AF_ACMP1_O(模拟比较器比较输出1)
 ** \return  无
 ** \note  
 **  (1)P0的PinNum输入值范围：GPIO_PIN_0~GPIO_PIN_5
 **  (2)P1的PinNum输入值范围：GPIO_PIN_3~GPIO_PIN_7
 **  (3)P2的PinNum输入值范围：GPIO_PIN_1~GPIO_PIN_6
 **  (4)P3的PinNum输入值范围：GPIO_PIN_0~GPIO_PIN_2、GPIO_PIN_5、GPIO_PIN_6
 ******************************************************************************/
void GPIO_SetMUXMode(uint8_t Port, uint8_t PinNum, uint8_t Mode);

/********************************************************************************
 ** \brief	 GPIO_SET_PS_MODE
 **			 设置端口输入功能分配
 ** \param [in]  Ps_mode；PS_INT0、PS_INT1..... PS_FB
 **				 gpio:	GPIO_P00 .... GPIO_P36 
 ** \return  none
********************************************************************************/
#define  GPIO_SET_PS_MODE(Ps_mode,gpio)		(Ps_mode = gpio)	

/********************************************************************************
 ** \brief	 GPIO_ENABLE_OUTPUT
 **			 使能GPIO为推挽输出模式
 ** \param [in] PortTRIS ：方向寄存器 P0TRIS 、P1TRIS 、P2TRIS 、P3TRIS 
 **            PinNum 
 ** \return  none
 ******************************************************************************/
#define	 GPIO_ENABLE_OUTPUT(PortTRIS, PinNum)	(PortTRIS |= (1<<PinNum))

/********************************************************************************
 ** \brief	 GPIO_ENABLE_OUTPUT
 **			 使能GPIO为三态输入模式
 ** \param [in] PorttRIS ：方向寄存器 P0TRIS 、P1TRIS 、P2TRIS 、P3TRIS 
 **            PinNum 
 ** \return  none
 ******************************************************************************/
#define  GPIO_ENABLE_INPUT(PortTRIS, PinNum)	(PortTRIS &= ~(1<<PinNum))

/********************************************************************************
 ** \brief	 GPIO_ENABLE_OD
 **			 GPIO开漏功能开启
 ** \param [in] PorttOD  :开漏功能寄存器：P0OD、P1OD、P2OD、P3OD
 **            PinNum  : 0~7
 ** \return  none
 ******************************************************************************/
#define  GPIO_ENABLE_OD(PortOD, PinNum)			(PortOD |= (1<<PinNum))
/********************************************************************************
 ** \brief	 GPIO_DISABLE_OD
 **			 GPIO开漏功能关闭
 ** \param [in] PorttOD  :开漏功能寄存器：P0OD、P1OD、P2OD、P3OD
 **            PinNum  : 0~7
 ** \return  none
 ******************************************************************************/
#define  GPIO_DISABLE_OD(PortOD, PinNum)		(PortOD &= ~(1<<PinNum))
/********************************************************************************
 ** \brief	 GPIO_ENABLE_UP
 **			 GPIO上拉功能开启
 ** \param [in] PorttUP  :上拉功能寄存器：P0UP、P1UP、P2UP、P3UP
 **            PinNum  : 0~7
 ** \return  none
 ******************************************************************************/
#define  GPIO_ENABLE_UP(PortUP, PinNum)			(PortUP |= (1<<PinNum))
/********************************************************************************
 ** \brief	 GPIO_DISABLE_UP
 **			 GPIO上拉功能关闭
 ** \param [in] PorttUP  :上拉功能寄存器：P0UP、P1UP、P2UP、P3UP
 **            PinNum  : 0~7
 ** \return  none
 ******************************************************************************/
#define  GPIO_DISABLE_UP(PortUP, PinNum)		(PortUP &= ~(1<<PinNum))
/********************************************************************************
 ** \brief	 GPIO_ENABLE_RD
 **			 GPIO下拉功能开启
 ** \param [in] PorttRD  :下拉功能寄存器：P0RD、P1RD、P2RD、P3RD
 **            PinNum  : 0~7
 ** \return  none
 ******************************************************************************/
#define  GPIO_ENABLE_RD(PortRD, PinNum)			(PortRD |= (1<<PinNum))
/********************************************************************************
 ** \brief	 GPIO_DISABLE_RD
 **			 GPIO下拉功能关闭
 ** \param [in] PorttRD  :下拉功能寄存器：P0RD、P1RD、P2RD、P3RD
 **            PinNum  : 0~7
 ** \return  none
 ******************************************************************************/
#define  GPIO_DISABLE_RD(PortRD, PinNum)		(PortRD &= ~(1<<PinNum))
/********************************************************************************
 ** \brief	 GPIO_ENABLE_LOW_CURRENT
 **			 GPIO弱电流驱动功能开启
 ** \param [in] PorttDR  :驱动功能寄存器：P0DR、P1DR、P2DR、P3DR
 **            PinNum  : 0~7
 ** \return  none
 ******************************************************************************/
#define  GPIO_ENABLE_LOW_CURRENT(PortDR, PinNum)	(PortDR |= (1<<PinNum))
/********************************************************************************
 ** \brief	 GPIO_ENABLE_LOW_CURRENT
 **			 GPIO弱电流驱动功能关闭
 ** \param [in] PorttDR  :驱动功能寄存器：P0DR、P1DR、P2DR、P3DR
 **            PinNum  : 0~7
 ** \return  none
 ******************************************************************************/
#define  GPIO_DISABLE_LOW_CURRENT(PortDR, PinNum)	(PortDR &= ~(1<<PinNum))

/********************************************************************************
 ** \brief	 GPIO_ENABLE_LOW_CURRENT
 **			 GPIO慢斜率功能开启
 ** \param [in] PorttSR  :斜率控制功能寄存器：P0SR、P1SR、P2SR、P3SR
 **            PinNum  : 0~7
 ** \return  none
 ******************************************************************************/
#define  GPIO_ENABLE_SLOW_SLOPE(PortSR, PinNum)      (PortSR |= (1<<PinNum))
/********************************************************************************
 ** \brief	 GPIO_ENABLE_LOW_CURRENT
 **			 GPIO慢斜率功能关闭
 ** \param [in] PorttSR  :斜率控制功能寄存器：P0SR、P1SR、P2SR、P3SR
 **            PinNum  : 0~7
 ** \return  none
 ******************************************************************************/
#define  GPIO_DISABLE_SLOW_SLOPE(PortSR, PinNum)     (PortSR &= ~(1<<PinNum))


/*----------------------------------------------------------------------------
 **GPIO 引脚中断触发模式
/*---------------------------------------------------------------------------*/
#define  GPIO_INT_RISING	(0x01)			/*上升沿触发中断*/
#define  GPIO_INT_FALLING	(0x02)			/*下降沿触发中断*/
#define  GPIO_INT_BOTH_EDGE	(0x03)			/*上升、下降沿均触发中断*/

/********************************************************************************
 ** \brief	 GPIO_SET_INT_MODE
 **			 设置IO口的外部中断功能
 ** \param [in] PorttEICFG ：IO口的外部中断模式配置寄存器
 **            PinIntMode ：IO口的复用功能 
 ** \return  none
 ******************************************************************************/
#define  GPIO_SET_INT_MODE(PortEICFG, PinIntMode)	(PortEICFG = PinIntMode)

/********************************************************************************
 ** \brief	 GPIO_ENABLE_INT_MODE
 **			使能IO口的外部中断功能
 ** \param [in] PorttEXTIE ：IO口的外部中断寄存器
 **            PinMSK：GPIO_PIN_0_MSK .. GPIO_PIN_7_MSK
 ** \return  none
 ******************************************************************************/
#define  GPIO_ENABLE_INT_MODE(PortEXTIE,PinMSK)		(PortEXTIE |= PinMSK)	

/********************************************************************************
 ** \brief	 GPIO_DISABLE_INT_MODE
 **			关闭IO口的外部中断功能
 ** \param [in] PorttEXTIE ：IO口的外部中断寄存器
 **            PinMSK: GPIO_PIN_0_MSK .. GPIO_PIN_7_MSK
 ** \return  none
 ******************************************************************************/
#define  GPIO_DISABLE_INT_MODE(PortEXTIE,PinMSK)	(PortEXTIE &= ~PinMSK)	

/*****************************************************************************/
/* Global type definitions ('typedef') */
/*****************************************************************************/



/*****************************************************************************/
/* Global variable declarations ('extern', definition in C source) */
/*****************************************************************************/


/*****************************************************************************/
/* Global function prototypes ('extern', definition in C source) */
/*****************************************************************************/
/****************************************************************************
 ** \brief	 GPIO_ConfigGPIOMode
 **			 配置GPIO模式
 ** \param [in] Port  ： GPIO0、GPIO1、GPIO2、GPIO3
 **			   PinMSK: GPIO_PIN_0_MSK ~ GPIO_PIN_7_MSK	
 **        Cfg : 配置结构体
 **         |-Mode:	GPIO_Input_Floating(输入浮空)
 ** 				| 			GPIO_IPU(输入带上拉)
 **					|				GPIO_IPD(输入带下拉)
 **					|				GPIO_Out_PP(推挽输出)
 **					| 			GPIO_Out_OD(开漏输出)
 **					|
 **					|-Slew: GPIO_Slow_Slew(慢斜率)  GPIO_Fast_Slew(块斜率)
 **					|-DRVCurrent: GPIO_Low_Current(低电流)  GPIO_High_Current(高电流) 
 ** \return  none
 ** \note   
 ***************************************************************************/
void GPIO_ConfigGPIOMode( uint8_t Port, uint8_t PinMSK,GPIOCfgDef *Cfg);

 /********************************************************************************
 ** \brief	 GPIO_EnableInt
 **			 使能IO口的中断功能
 ** \param [in] Port  ： GPIO0、GPIO1、GPIO2、GPIO3
 **			    PinMSK: GPIO_PIN_0_MSK ~ GPIO_PIN_7_MSK			 
 ** \return  none
 ** \note   
 **  (1)P0的PinMSK输入值范围：GPIO_PIN_0_MSK ~ GPIO_PIN_5_MSK
 **  (2)P1的PinMSK输入值范围：GPIO_PIN_3_MSK ~ GPIO_PIN_7_MSK
 **  (3)P2的PinMSK输入值范围：GPIO_PIN_1_MSK ~ GPIO_PIN_6_MSK
 **  (4)P3的PinMSK输入值范围：GPIO_PIN_0_MSK ~ GPIO_PIN_2_MSK、GPIO_PIN_5_MSK、GPIO_PIN_6_MSK
 ******************************************************************************/
void GPIO_EnableInt(uint8_t Port, uint8_t PinMSK);
/********************************************************************************
 ** \brief	 GPIO_DisableInt
 **			 关闭IO口的中断功能
 ** \param [in] Port  ： GPIO0、GPIO1、GPIO2、GPIO3
 **			    PinMSK: GPIO_PIN_0_MSK ~ GPIO_PIN_7_MSK			 
 ** \return  none
 ** \note   
 **  (1)P0的PinMSK输入值范围：GPIO_PIN_0_MSK ~ GPIO_PIN_5_MSK
 **  (2)P1的PinMSK输入值范围：GPIO_PIN_3_MSK ~ GPIO_PIN_7_MSK
 **  (3)P2的PinMSK输入值范围：GPIO_PIN_1_MSK ~ GPIO_PIN_6_MSK
 **  (4)P3的PinMSK输入值范围：GPIO_PIN_0_MSK ~ GPIO_PIN_2_MSK、GPIO_PIN_5_MSK、GPIO_PIN_6_MSK
 ******************************************************************************/
void GPIO_DisableInt(uint8_t Port, uint8_t PinMSK);

 /******************************************************************************
 ** \brief	 GPIO_GetIntFlag
 **			 获取中断标志
 ** \param [in] Port  ： GPIO0、GPIO1、GPIO2、GPIO3
 **			    PinNum:  GPIO_PIN_0~GPIO_PIN_7 (0~7)		 
 ** \return  0：无中断产生
 **			 1：有中断产生
 ** \note  
 **  (1)P0的PinNum输入值范围：GPIO_PIN_0~GPIO_PIN_5
 **  (2)P1的PinNum输入值范围：GPIO_PIN_3~GPIO_PIN_7
 **  (3)P2的PinNum输入值范围：GPIO_PIN_1~GPIO_PIN_6
 **  (4)P3的PinNum输入值范围：GPIO_PIN_0~GPIO_PIN_2、GPIO_PIN_5、GPIO_PIN_6
 ******************************************************************************/
uint8_t  GPIO_GetIntFlag(uint8_t Port, uint8_t PinNum);
 /********************************************************************************
 ** \brief	 GPIO_ClearIntFlag
 **			清除中断标志位
 ** \param [in] Port  ： GPIO0、GPIO1、GPIO2、GPIO3
 **			    PinNum:  GPIO_PIN_0~GPIO_PIN_7 (0~7)		 
 ** \return  none
 ** \note  
 **  (1)P0的PinNum输入值范围：GPIO_PIN_0~GPIO_PIN_5
 **  (2)P1的PinNum输入值范围：GPIO_PIN_3~GPIO_PIN_7
 **  (3)P2的PinNum输入值范围：GPIO_PIN_1~GPIO_PIN_6
 **  (4)P3的PinNum输入值范围：GPIO_PIN_0~GPIO_PIN_2、GPIO_PIN_5、GPIO_PIN_6   
 ******************************************************************************/
void GPIO_ClearIntFlag(uint8_t Port, uint8_t PinNum);
 /******************************************************************************
 ** \brief	 GPIO_SetExtIntMode
 **			 设置外部中断的相应模式
 ** \param [in] Port  ： GPIO0、GPIO1、GPIO2、GPIO3
 **			    PinNum:  GPIO_PIN_0~GPIO_PIN_7 (0~7)		 
 **			    Mode: GPIO_Int_Disable(关闭中断)
 **			          GPIO_Int_Rising(仅上升沿触发中断)
 **			          GPIO_Int_Falling(仅下降沿触发中断)
 **			          GPIO_Int_DualEdge(上升沿和下降沿都触发中断)
 ** \return  无
 ** \note  
 **  (1)P0的PinNum输入值范围：GPIO_PIN_0~GPIO_PIN_5
 **  (2)P1的PinNum输入值范围：GPIO_PIN_3~GPIO_PIN_7
 **  (3)P2的PinNum输入值范围：GPIO_PIN_1~GPIO_PIN_6
 **  (4)P3的PinNum输入值范围：GPIO_PIN_0~GPIO_PIN_2、GPIO_PIN_5、GPIO_PIN_6
 ******************************************************************************/
void GPIO_SetExtIntMode(uint8_t Port, uint8_t PinNum, uint8_t Mode);

 /******************************************************************************
 ** \brief	 GPIO_WriteBit
 **			 设置GPIO输出的电平
 ** \param [in] Port  ： GPIO0、GPIO1、GPIO2、GPIO3
 **			    PinNum:  GPIO_PIN_0~GPIO_PIN_7 (0~7)		 
 **			    Val: 1(令对应的GPIO引脚输出1)
 **			         0(令对应的GPIO引脚输出0)
 ** \return  无
 ** \note  
 **  (1)P0的PinNum输入值范围：GPIO_PIN_0~GPIO_PIN_5
 **  (2)P1的PinNum输入值范围：GPIO_PIN_3~GPIO_PIN_7
 **  (3)P2的PinNum输入值范围：GPIO_PIN_1~GPIO_PIN_6
 **  (4)P3的PinNum输入值范围：GPIO_PIN_0~GPIO_PIN_2、GPIO_PIN_5、GPIO_PIN_6
 ******************************************************************************/
void GPIO_WriteBit(uint8_t Port, uint8_t PinNum,bit Val);

 /********************************************************************************
 ** \brief	 GPIO_CheckIfIntEnabled
 **			 检测对应IO口的中断功能是否激活
 ** \param [in] Port  ： GPIO0、GPIO1、GPIO2、GPIO3
 **			    PinMSK: GPIO_PIN_0_MSK ~ GPIO_PIN_7_MSK			 
 ** \return  bit：如果中断开启，则返回1，否则返回0
 ** \note   
 **  (1)P0的PinMSK输入值范围：GPIO_PIN_0_MSK ~ GPIO_PIN_5_MSK
 **  (2)P1的PinMSK输入值范围：GPIO_PIN_3_MSK ~ GPIO_PIN_7_MSK
 **  (3)P2的PinMSK输入值范围：GPIO_PIN_1_MSK ~ GPIO_PIN_6_MSK
 **  (4)P3的PinMSK输入值范围：GPIO_PIN_0_MSK ~ GPIO_PIN_2_MSK、GPIO_PIN_5_MSK、GPIO_PIN_6_MSK
 ******************************************************************************/
bit GPIO_CheckIfIntEnabled(uint8_t Port, uint8_t PinMSK);

 /******************************************************************************
 ** \brief	 GPIO_GetExtIntMode
 **			 获取指定GPIO的中断模式
 ** \param [in] Port  ： GPIO0、GPIO1、GPIO2、GPIO3
 **			    PinNum:  GPIO_PIN_0~GPIO_PIN_7 (0~7)		 
 **			   
 ** \return  int: GPIO_Int_Disable(关闭中断)
 **			          GPIO_Int_Rising(仅上升沿触发中断)
 **			          GPIO_Int_Falling(仅下降沿触发中断)
 **			          GPIO_Int_DualEdge(上升沿和下降沿都触发中断)
 ** \note  
 **  (1)P0的PinNum输入值范围：GPIO_PIN_0~GPIO_PIN_5
 **  (2)P1的PinNum输入值范围：GPIO_PIN_3~GPIO_PIN_7
 **  (3)P2的PinNum输入值范围：GPIO_PIN_1~GPIO_PIN_6
 **  (4)P3的PinNum输入值范围：GPIO_PIN_0~GPIO_PIN_2、GPIO_PIN_5、GPIO_PIN_6
 ******************************************************************************/
int GPIO_GetExtIntMode(uint8_t Port, uint8_t PinNum);

#endif /* __GPIO_H__ */






