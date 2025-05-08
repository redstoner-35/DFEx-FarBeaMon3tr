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
/** \file extint.h
**
** 
**
** History:
** 
*****************************************************************************/
#ifndef __EXTINT_H__
#define __EXTINT_H__
/*****************************************************************************/
/* Include files */
/*****************************************************************************/
#include "cms8S6990.h"
/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define') */
/*****************************************************************************/
/*----------------------------------------------------------------------------
 **外部中断通道
/*---------------------------------------------------------------------------*/
#define		EXTINT0				(0x00)			/*EXTINT channel 0 number*/
#define		EXTINT1				(0x01)			/*EXTINT channel 1 number*/
/*----------------------------------------------------------------------------
 **外部中断触发模式
/*---------------------------------------------------------------------------*/
#define  EXTINT_TRIG_LOW		(0x00)			/*低电平触发模式*/
#define  EXTINT_TRIG_FALLING  	(0x01)			/*下降沿触发模式*/

/*****************************************************************************/
/* Global type definitions ('typedef') */
/*****************************************************************************/


/*****************************************************************************/
/* Global variable declarations ('extern', definition in C source) */
/*****************************************************************************/


/*****************************************************************************/
/* Global function prototypes ('extern', definition in C source) */
/*****************************************************************************/
/********************************************************************************
 ** \brief	 EXTINT_ConfigInt
 **			 配置外部中断功能
 ** \param [in]  Extintn : (1)EXTINT0     :外部中断0
 **            				(2)EXTINT1     :外部中断1
 **			     IntMode  ：(1)EXTINT_TRIG_LOW	:低电平触发模式
 **							(2)EXTINT_TRIG_FALLING :下降沿触发模式
 ** \return  none
 ** \note  
 ******************************************************************************/
void EXTINT_ConfigInt(uint8_t Extintn, uint8_t IntMode);
/********************************************************************************
 ** \brief	 EXTINT_EnableInt
 **			 使能外部中断功能
 ** \param [in]  Extintn : (1)EXTINT0     :外部中断0
 **            				(2)EXTINT1     :外部中断1
 ** \return  none
 ** \note   
 ******************************************************************************/
void EXTINT_EnableInt(uint8_t Extintn);

/********************************************************************************
 ** \brief	 EXTINT_DisableInt
 **			 关闭外部中断功能
 ** \param [in]  Extintn : (1)EXTINT0     :外部中断0
 **            				(2)EXTINT1     :外部中断1 
 ** \return  none
 ** \note   
 ******************************************************************************/
void EXTINT_DisableInt(uint8_t Extintn);

/********************************************************************************
 ** \brief	 EXTINT_GetIntFlag
 **			 获取中断标志
 ** \param [in]  Extintn : (1)EXTINT0     :外部中断0
 **            				(2)EXTINT1     :外部中断1	 
 ** \return  0:无中断1：有中断
 ** \note  
 ******************************************************************************/
uint8_t  EXTINT_GetIntFlag(uint8_t Extintn);
 
 /********************************************************************************
 ** \brief	 EXTINT_ClearIntFlag
 **			 清中断标志
 ** \param [in]  Extintn : (1)EXTINT0     :外部中断0
 **            				(2)EXTINT1     :外部中断1		 
 ** \return  none
 ** \note   
 ******************************************************************************/
void EXTINT_ClearIntFlag(uint8_t Extintn);


#endif /* __EXTINT_H__ */






