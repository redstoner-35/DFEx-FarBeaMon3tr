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
/** \file opa.h
**
** 
**
** History:
** 
*****************************************************************************/
#ifndef __OPA_H__
#define __OPA_H__
/*****************************************************************************/
/* Include files */
/*****************************************************************************/
#include "cms8s6990.h"
/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define') */
/*****************************************************************************/
/*----------------------------------------------------------------------------
 **opa 通道
---------------------------------------------------------------------------*/
#define OPA0					(0x00)		/*opa channel 0 number*/
#define OPA1					(0x01)		/*opa channel 1 number*/
/*----------------------------------------------------------------------------
 **opa 运行模式
---------------------------------------------------------------------------*/
#define OPA_MODE_OPA			(0x01)		/*运放模式*/
#define OPA_MODE_ACMP			(0x00)		/*比较模式*/

/*----------------------------------------------------------------------------
 **opa 调节
---------------------------------------------------------------------------*/
#define OPA_OFFSET_CONFIG		(0x00)		/*opa调节模式失调电压由Config决定*/
#define OPA_OFFSET_OPADJ		(0xAA)		/*opa调节模式失调电压由OPnADJ[4:0]决定*/
/*----------------------------------------------------------------------------
 **opa 端口
---------------------------------------------------------------------------*/
#define OPA_NEGSEL_N				(0x00)		/*opa 负端输入选择OPn_N*/		
#define OPA_POSSEL_P				(0x00)		/*opa 正端输入选择OPn_P*/		
#define OPA_POSSEL_BG				(0x01)		/*opa 正端输入选择 BG 1.2V*/
#define OPA_OUT_O					(0x01)		/*opa 输出端选择OPn_O*/
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
 ** \brief	 OPA_SetRunMode
 **			 设置运放运行模式
 ** \param [in] OPAn : OPA0、OPA1
 **				OpaMode:  (1)OPA_MODE_OPA
 **						  (2)OPA_MODE_ACMP
 ** \return  none
 ** \note   
 *****************************************************************************/
void OPA_SetRunMode(uint8_t OPAn, uint8_t OpaMode);

/*****************************************************************************
 ** \brief	 OPA_ConfigPositive
 **			 设置运放正端输入
 ** \param [in] OPAn : OPA0、OPA1
 **				PositiveSource:  (1)OPA_POSSEL_P	:OPnP
 **								 (2)OPA_POSSEL_BG
 ** \return  none
 ** \note    
 *****************************************************************************/
void OPA_ConfigPositive(uint8_t OPAn, uint8_t PositiveSource);

/*****************************************************************************
 ** \brief	 OPA_ConfigNegative
 **			 设置运放负端输入
 ** \param [in] OPAn : OPA0、OPA1
 **				NegativeSource:  (1)OPA_NEGSEL_N		:OPnN			
 ** \return  none
 ** \note   
 *****************************************************************************/
void OPA_ConfigNegative(uint8_t OPAn, uint8_t NegativeSource);

/*****************************************************************************
 ** \brief	 OPA_DisableOutput
 **			 关闭运放器输出端
 ** \param [in] OPAn : OPA0、OPA1
 ** \return  none
 ** \note   
 *****************************************************************************/
void OPA_DisableOutput(uint8_t OPAn);

/*****************************************************************************
 ** \brief	 OPA_EnableOutput
 **			 开启运放器输出端
 ** \param [in] OPAn : OPA0、OPA1
 ** \return  none
 ** \note   
 *****************************************************************************/
void OPA_EnableOutput(uint8_t OPAn);

/*****************************************************************************
 ** \brief	 OPA_ConfigOffsetAdj
 **			 配置OPA失调电压调节方式
 ** \param [in] OPAn : OPA0、OPA1		
 **				OffsetAdj:	(1)OPA_OFFSET_CONFIG
 **							(2)OPA_OFFSET_OPADJ
 **				AdjVlue : 失调电压调节值：0x00~0x1f
 ** \return  none
 ** \note   
*****************************************************************************/
void OPA_ConfigOffsetAdj(uint8_t OPAn,uint8_t OffsetAdj, uint8_t AdjVlue);

/********************************************************************************
 ** \brief	 OPA_Start
 **			 开启OP
 ** \param [in] OPAn : OPA0、OPA1
 ** \return  none
 ** \note   
 ******************************************************************************/
void OPA_Start(uint8_t OPAn);

/********************************************************************************
 ** \brief	 OPA_Stop
 **			 关闭OPA
 ** \param [in] OPAn : OPA0、OPA1 
 ** \return  none
 ** \note   
 ******************************************************************************/
void OPA_Stop(uint8_t OPAn);

/********************************************************************************
 ** \brief	 OPA_GetResult
 **			 获取获取结果
 ** \param [in] OPAn : OPA0、OPA1
 ** \return  0/1
 ** \note   
 ******************************************************************************/
uint8_t OPA_GetResult(uint8_t OPAn);

/*****************************************************************************
 ** \brief	 OPA_GetOffsetAdjValue
 **			 获取OPA失调电压调节值
 ** \param [in] OPAn : OPA0、OPA1
 **				OpaMode:  (1)OPA_MODE_OPA
 **						  (2)OPA_MODE_ACMP	
 ** \return  5位失调电压修调值
 ** \note   
 *****************************************************************************/
uint8_t  OPA_GetOffsetAdjValue(uint8_t OPAn, uint8_t OpaMode);





#endif /* __OPA_H__ */






