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
/** \file uart.h
**
**
**
** History:
** 
******************************************************************************/
#ifndef __UART_H__
#define __UART_H__
/*****************************************************************************/
/* Include files */
/*****************************************************************************/
#include "cms8s6990.h"
/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define') */
/*****************************************************************************/
/*----------------------------------------------------------------------------
 **UART 通道
/*---------------------------------------------------------------------------*/
#define UART0					(0x00)			/*UART channel 0 number*/
#define UART1					(0x01)			/*UART channel 0 number*/
/*----------------------------------------------------------------------------
 **UART 模式
/*---------------------------------------------------------------------------*/
#define UART_MOD_SYN				(0x00)		/*同步模式*/
#define	UART_MOD_ASY_8BIT 			(0x01)		/*8位异步模式且波特率可变*/				
#define UART_MOD_ASY_9BIT			(0x03)		/*9位异步模式且波特率可变*/
#define UART_MOD_ASY_9BIT_STATIC	(0x02)		/*9位异步模式且波特率为Fsys/32或者Fsys/64*/
/*----------------------------------------------------------------------------
 **UART 波特率时钟
/*---------------------------------------------------------------------------*/
#define UART_BAUD_TMR1				(0x00)		/*波特率时钟选择Timer1*/
#define UART_BAUD_TMR4				(0x01)		/*波特率时钟选择Timer4*/
#define UART_BAUD_TMR2				(0x02)		/*波特率时钟选择Timer2*/
#define UART_BAUD_BRT				(0x03)		/*波特率时钟选择BRT*/
/*----------------------------------------------------------------------------
 ** BRT 波特率定时器时钟分频
/*---------------------------------------------------------------------------*/
#define BRT_CLK_DIV_1				(0x00)		/*Fbrt = Fsys/1*/
#define BRT_CLK_DIV_2				(0x01)		/*Fbrt = Fsys/2*/
#define BRT_CLK_DIV_4				(0x02)		/*Fbrt = Fsys/4*/
#define BRT_CLK_DIV_8				(0x03)		/*Fbrt = Fsys/8*/
#define BRT_CLK_DIV_16				(0x04)		/*Fbrt = Fsys/16*/
#define BRT_CLK_DIV_32				(0x05)		/*Fbrt = Fsys/32*/
#define BRT_CLK_DIV_64				(0x06)		/*Fbrt = Fsys/64*/
#define BRT_CLK_DIV_128				(0x07)		/*Fbrt = Fsys/128*/
/*****************************************************************************/
/* Global type definitions ('typedef') */
/*****************************************************************************/



/*****************************************************************************/
/* Global variable declarations ('extern', definition in C source) */
/*****************************************************************************/


/*****************************************************************************/
/* Global function prototypes ('extern', definition in C source) */
/*****************************************************************************/
/******************************************************************************
 ** \brief UART_ConfigRunMode
 **			配置UART运行模式
 ** \param [in]     UARTn: 			(1)UART0
 **									(2)UART1
 **					UARTMode:		(1)UART_MOD_SYN : 同步模式
 **									(2)UART_MOD_ASY_8BIT: 8位异步模式
 **									(3)UART_MOD_ASY_9BIT：9位异步模式
 **									(4)UART_MOD_ASY_9BIT_STATIC：9位波特率固定异步模式
 **					UARTBaudTimer:  (1)UART_BAUD_TMR1 :	Timer1产生波特率
 **									(2)UART_BAUD_TMR4 :	Timer4产生波特率
 **									(3)UART_BAUD_TMR2 : Timer2产生波特率
 **									(4)UART_BAUD_BRT : BRT产生波特率
 ** \return none
******************************************************************************/
void UART_ConfigRunMode(uint8_t UARTn, uint8_t UARTMode, uint8_t UARTBaudTimer);
/******************************************************************************
 ** \brief UART_EnableDoubleFrequency
 **			使能UART波特率倍频模式
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return none
******************************************************************************/
void UART_EnableDoubleFrequency(uint8_t UARTn);
/******************************************************************************
 ** \brief UART_DisableDoubleFrequency
 **			关闭UART波特率倍频模式
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return none
******************************************************************************/
void UART_DisableDoubleFrequency(uint8_t UARTn);

/******************************************************************************
 ** \brief UART_ConfigBaudRate
 **			配置UART波特率
 ** \param [in]    UARTn:  (1)UART0 : UART0
 **									(2)UART1 : UART1
 **					BaudRateValue:  波特率值
 ** \return Timer1/4: 8位自动重装模式的THx值
 **			Timer2: 16 位自动重装模式的 RLDH:RLDL 值
 **			BRT: 16 位定时模式 BRTH:BRTL 值
******************************************************************************/
uint16_t UART_ConfigBaudRate(uint8_t UARTn, uint32_t  BaudRateValue);

/******************************************************************************
 ** \brief UART_EnableReceive
 **			开启接收模式
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 
******************************************************************************/
void UART_EnableReceive(uint8_t UARTn);
/******************************************************************************
 ** \brief UART_DisableReceive
 **			关闭接收模式
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 
******************************************************************************/
void UART_DisableReceive(uint8_t UARTn);

/******************************************************************************
 ** \brief UART_EnableInt
 **			开启中断
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 
******************************************************************************/
void UART_EnableInt(uint8_t UARTn);
/******************************************************************************
 ** \brief UART_DisableInt
 **			关闭中断
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 
******************************************************************************/
void UART_DisableInt(uint8_t UARTn);


/******************************************************************************
 ** \brief UART_GetReceiveIntFlag
 **			获取接收中断标志位
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 0：无中断，1：有中断
******************************************************************************/
uint8_t  UART_GetReceiveIntFlag(uint8_t UARTn);

/******************************************************************************
 ** \brief UART_ClearReceiveIntFlag
 **			清除接收中断标志位
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 
******************************************************************************/
void UART_ClearReceiveIntFlag(uint8_t UARTn);

/******************************************************************************
 ** \brief UART_GetSendIntFlag
 **			获取发送中断标志位
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 0：无中断，1：有中断
******************************************************************************/
uint8_t  UART_GetSendIntFlag(uint8_t UARTn);

/******************************************************************************
 ** \brief UART_ClearSendIntFlag
 **			清除发送中断标志位
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 
******************************************************************************/
void UART_ClearSendIntFlag(uint8_t UARTn);

/******************************************************************************
 ** \brief UART_GetBuff
 **			获取接收Buff的值
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 
******************************************************************************/
uint8_t UART_GetBuff(uint8_t UARTn);

/******************************************************************************
 ** \brief UART_SendBuff
 **			发送数据到发送Buff
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 **				   UARTSendValue : 发送的8位值
 ** \return 
******************************************************************************/
void UART_SendBuff(uint8_t UARTn, uint8_t UARTSendValue);

/******************************************************************************
 ** \brief UART_SendNinthBit
 **			发送数据的第九位
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 **				   UARTSendValue : 发送的第九位值 0 /1
 ** \return 
******************************************************************************/
void UART_SendNinthBit(uint8_t UARTn, uint8_t UARTSendValue);
/******************************************************************************
 ** \brief UART_GetNinthBit
 **			获取数据的第九位
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 0/1
******************************************************************************/
uint8_t UART_GetNinthBit(uint8_t UARTChannelNumber);

/******************************************************************************
 ** \brief UART_EnableBRT
 **		   使能BRT时钟模块
 ** \param [in]none 
 ** \return none
******************************************************************************/
void UART_EnableBRT(void);
/******************************************************************************
 ** \brief UART_ConfigBRTClk
 **		   配置BRT模块时钟
 ** \param [in] BRTClkDiv : BRT_CLK_DIV_1 ~ BRT_CLK_DIV_128
 ** \return none
******************************************************************************/
void UART_ConfigBRTClk(uint8_t BRTClkDiv);
/******************************************************************************
 ** \brief UART_ConfigBRTPeriod
 **		   配置BRT模块时钟周期
 ** \param [in] BRTPeriod: 16bit
 ** \return none
******************************************************************************/
void UART_ConfigBRTPeriod(uint16_t BRTPeriod);

/******************************************************************************
 ** \brief UART_DisableBRT
 **		   关闭BRT时钟模块
 ** \param [in] none
 ** \return none
******************************************************************************/
void UART_DisableBRT(void);




#endif /* __UART_H__ */






