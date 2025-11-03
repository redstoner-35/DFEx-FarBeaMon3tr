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
/** \file pga.h
**
** 
**
** History:
** 
*****************************************************************************/
#ifndef __PGA_H__
#define __PGA_H__
/*****************************************************************************/
/* Include files */
/*****************************************************************************/
#include "cms8s6990.h"
/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define') */
/*****************************************************************************/
/*----------------------------------------------------------------------------
 **PGA 放大倍数
---------------------------------------------------------------------------*/
#define 	PGA_GAIN_1		(0x0)			/*1 倍*/
#define 	PGA_GAIN_2		(0x1)			/*2 倍*/
#define 	PGA_GAIN_4		(0x2)			/*4 倍*/
#define 	PGA_GAIN_8		(0x3)			/*8 倍*/
#define 	PGA_GAIN_16		(0x4)			/*16 倍*/
#define 	PGA_GAIN_32		(0x5)			/*32 倍*/
#define 	PGA_GAIN_64		(0x6)			/*64 倍*/
#define 	PGA_GAIN_128	(0x7)			/*128 倍*/

/*----------------------------------------------------------------------------
 **PGA 工作模式
---------------------------------------------------------------------------*/
#define		PGA_NORMAL_MODE			(0x00)		/*正常工作模式*/
#define		PGA_ADJ_MODE			(0x3)		/*调节模式*/
/*----------------------------------------------------------------------------
 **PGA 正常输入模式
---------------------------------------------------------------------------*/
#define		PGA_NORM_SINGLE				(0x00)		/*正常模式：单端输入模式*/
#define		PGA_NORM_SHAM_DIFFERENCE	(0x01)		/*正常模式：伪差分输入模式*/
/*----------------------------------------------------------------------------
 **PGA  修调模式
---------------------------------------------------------------------------*/
#define		PGA_ADJ_MODE0			(0x01)		/*内部输入短接到地*/
#define		PGA_ADJ_MODE1			(0x02)		/*内部输入短接，外部从PGAP给不同的共模点 */
#define		PGA_ADJ_MODE2			(0x03)		/*内部输入短接，外部从PGAGND给不同的共模点 */

/*----------------------------------------------------------------------------
 **PGA 输入端口
---------------------------------------------------------------------------*/
#define	PGA_SIG_PGAP		(0x01)			//PGAP端口
#define PGA_SIG_PGAGND		(0x00)			//PGAGND端口
#define PGA_DIF_H_PGAP_L_PGAGND	 (0x01)		//差分模式：高端接PGAP，低端接PGAGAND
#define PGA_DIF_H_PGAGND_L_PGAP	 (0x00)		//差分模式：高端接PGAGND，低端接PGAP	
/*----------------------------------------------------------------------------
 **PGA 失调电压调节方式
---------------------------------------------------------------------------*/
#define 	PGA_OFFSET_PGAADJ		(0xAA)		/*运放n失调电压调节方式选择：由PGACON2寄存器中的PGAADJ<5:0>决定*/
#define 	PGA_OFFSET_CONFIG		(0x00)		/*运放n失调电压调节方式选择：由CONFIG相关位决定*/
/*----------------------------------------------------------------------------
 **PGA 采样保持时间
---------------------------------------------------------------------------*/
#define		PGA_SHT_1			(0x0)
#define		PGA_SHT_2			(0x1)
#define		PGA_SHT_3			(0x2)
#define		PGA_SHT_4			(0x3)
#define		PGA_SHT_5			(0x4)
#define		PGA_SHT_6			(0x5)
#define		PGA_SHT_7			(0x6)
#define		PGA_SHT_8			(0x7)
#define		PGA_SHT_9			(0x8)
#define		PGA_SHT_10			(0x9)
#define		PGA_SHT_11			(0xa)
#define		PGA_SHT_12			(0xb)
#define		PGA_SHT_13			(0xc)
#define		PGA_SHT_14			(0xd)
#define		PGA_SHT_15			(0xe)
#define		PGA_SHT_16			(0xf)

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
 ** \brief	 PGA_EnableNormalMode
 **			 使能PGA正常运行模式
 ** \param [in] RunMode:  (1)PGA_NORM_SINGLE		正常模式：单端输入模式
 **						  (2)PGA_NORM_SHAM_DIFFERENCE	正常模式：伪差分输入模式
 ** \return  none
 ** \note   
 *****************************************************************************/
void PGA_EnableNormalMode(uint8_t PGAMode);

/*****************************************************************************
 ** \brief	 PGA_ConfigInput
 **			 设置PGA输入
 ** \param [in] Port: 	  (1)PGA_SIG_PGAP		PGAP端口
 **						  (2)PGA_SIG_PGAGND		PGAGND端口
 **						  (3)PGA_DIF_H_PGAP_L_PGAGND	差分模式：高端接PGAP，低端接PGAGAND
 **						  (4)PGA_DIF_H_PGAGND_L_PGAP	差分模式：高端接PGAGND，低端接PGAP				
 ** \return  none
 ** \note   
 *****************************************************************************/
void PGA_ConfigInput(uint8_t Port);

/*****************************************************************************
 ** \brief	 PGA_EnableOutput
 **			 使能PGA输出
 ** \param [in] none
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_EnableOutput(void);

/*****************************************************************************
 ** \brief	 PGA_DisableOutPut
 **			 关闭PGA输出
 ** \param [in] none
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_DisableOutPut(void);

/*****************************************************************************
 ** \brief	 PGA_EnableSHA
 **			 开启PGA采样保持器
 ** \param [in] none
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_EnableSHA(void);

/*****************************************************************************
 ** \brief	 PGA_DisableSHA
 **			 关闭PGA采样保持器
 ** \param [in] none
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_DisableSHA(void);

/*****************************************************************************
 ** \brief	 PGA_ConfigSampleTime
 **			 设置PGA采样保持器采样保持时间
 ** \param [in] SampleTime: PGA_SHT_1 ... PGA_SHT_16
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_ConfigSampleTime(uint8_t SampleTime);


/*****************************************************************************
 ** \brief	 PGA_ConfigGain
 **			 配置PGA放大增益
 ** \param [in] Gain:  (1)PGA_GAIN_1
 **					   (2)PGA_GAIN_2
 **					   (3)PGA_GAIN_4 
 **					   (4)PGA_GAIN_8
 **					   (5)PGA_GAIN_16 
 **					   (6)PGA_GAIN_32
 **					   (7)PGA_GAIN_64 
 **					   (8)PGA_GAIN_128
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_ConfigGain(uint8_t Gain);

/*****************************************************************************
 ** \brief	 PGA_ConfigOffsetAdj
 **			 配置PGA失调电压调节方式
 ** \param [in] OffsetAdj:	(1)PGA_OFFSET_PGAADJ
 **							(2)PGA_OFFSET_CONFIG
 **				AdjVlue : 失调电压调节值：0x00~0x3f
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_ConfigOffsetAdj(uint8_t OffsetAdj, uint8_t AdjVlue);

/*****************************************************************************
 ** \brief	 PGA_Start
 **			 开启PGA
 ** \param [in]none
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_Start(void);
/*****************************************************************************
 ** \brief	 PGA_Stop
 **			 关闭PGA
 ** \param [in]none
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_Stop(void);

/*****************************************************************************
 ** \brief	 PGA_GetOffsetAdjValue
 **			 获取PGA失调电压调节值
 ** \param [in] AdjMode:  (1)PGA_ADJ_MODE0			内部输入短接到地
 **						  (2)PGA_ADJ_MODE1			内部输入短接，外部从PGAP给不同的共模点
 **						  (3)PGA_ADJ_MODE2			内部输入短接，外部从PGAGND给不同的共模点 			
 ** \return  5位失调电压修调值
 ** \note   
 *****************************************************************************/
uint8_t  PGA_GetOffsetAdjValue(uint8_t AdjMode);








#endif /* __PGA_H__ */






