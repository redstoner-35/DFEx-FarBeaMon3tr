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
/** \file lse.h
**
** 
**
** History:
** 
*****************************************************************************/
#ifndef __LSE_H__
#define __LSE_H__
/*****************************************************************************/
/* Include files */
/*****************************************************************************/
#include "cms8s6990.h"
/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define') */
/*****************************************************************************/

/*****************************************************************************/
/* Global type definitions ('typedef') */
/*****************************************************************************/


/*****************************************************************************/
/* Global variable declarations ('extern', definition in C source) */
/*****************************************************************************/


/*****************************************************************************/
/* Global function prototypes ('extern', definition in C source) */
/*****************************************************************************/

/*****************************************************************************
 ** \brief	 LSE_EnableLSE
 **			 使能LSE模块
 ** \param [in] none
 ** \return 
 ** \note   
 *****************************************************************************/
void  LSE_EnableLSE(void);
/*****************************************************************************
 ** \brief	 SYS_DisableLSE
 **			 关闭LSE模块
 ** \param [in] none
 ** \return 
 ** \note   
 *****************************************************************************/
void  LSE_DisableLSE(void);

/*****************************************************************************
 ** \brief	 LSE_Start
 **			 使能LSE计数
 ** \param [in] none
 ** \return 
 ** \note   
 *****************************************************************************/
void LSE_Start(void);
/*****************************************************************************
 ** \brief	 LSE_Stop
 **			 停止LSE计数
 ** \param [in] none
 ** \return 
 ** \note   
 *****************************************************************************/
void LSE_Stop(void);

/*****************************************************************************
 ** \brief	 LSE_ConfigLSE
 **			 设置LSE定时时间
 ** \param [in] Time: 16位定时时间
 ** \return none
 ** \note   
 *****************************************************************************/
void LSE_ConfigLSE(uint16_t  Time);

/*****************************************************************************
 ** \brief	 LSE_EnableLSEWakeup
 **			 使能LSE定时唤醒功能
 ** \param [in] none
 ** \return 
 ** \note   
 *****************************************************************************/
void LSE_EnableLSEWakeup(void);

/*****************************************************************************
 ** \brief	 LSE_DisableLSEWakeup
 **			 关闭LSE定时唤醒功能
 ** \param [in] none
 ** \return none
 ** \note    
 *****************************************************************************/
void LSE_DisableLSEWakeup(void);

/*****************************************************************************
 ** \brief	 LSE_EnableLSEInt
 **			 使能LSE中断
 ** \param [in] none
 ** \return none
 ** \note   
 *****************************************************************************/
void LSE_EnableLSEInt(void);
/*****************************************************************************
 ** \brief	LSE_DisableLSEInt
 **			 关闭LSE中断
 ** \param [in] none
 ** \return none
 ** \note   
 *****************************************************************************/
void LSE_DisableLSEInt(void);
/*****************************************************************************
 ** \brief	 LSE_GetLSEIntFlag
 **			 获取LSE中断标志位
 ** \param [in] none
 ** \return 1：产生中断  0：无中断
 ** \note  
 *****************************************************************************/
uint8_t  LSE_GetLSEIntFlag(void);
/*****************************************************************************
 ** \brief	 LSE_ClearLSEIntFlag
 **			 清除LSE中断标志位
 ** \param [in] none
 ** \return none
 ** \note   
 *****************************************************************************/
void  LSE_ClearLSEIntFlag(void);

/*****************************************************************************
 ** \brief	 LSE_GetLSEState
 **			 获取LSE稳定状态位
 ** \param [in] none
 ** \return 1：稳定  0：未稳定
 ** \note  
 *****************************************************************************/
uint8_t  LSE_GetLSEState(void);


#endif /*  __LSE_H__ */



