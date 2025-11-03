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
/** \file spi.h
**
** 
**
** History:
** 
*****************************************************************************/
#ifndef __SPI_H__
#define __SPI_H__
/*****************************************************************************/
/* Include files */
/*****************************************************************************/
#include "cms8s6990.h"
/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define') */
/*****************************************************************************/
/*----------------------------------------------------------------------------
 **SPI 时钟分频
/*---------------------------------------------------------------------------*/
#define		SPI_CLK_DIV_4		(0x00)
#define		SPI_CLK_DIV_8		(0x1<< SPI_SPCR_SPRn_Pos)
#define		SPI_CLK_DIV_16		(0x2<< SPI_SPCR_SPRn_Pos)
#define		SPI_CLK_DIV_32		(0x3<< SPI_SPCR_SPRn_Pos)
#define		SPI_CLK_DIV_64		(0x1<< SPI_SPCR_SPR2_Pos)
#define		SPI_CLK_DIV_128		((0x1<< SPI_SPCR_SPR2_Pos) | (0x1<< SPI_SPCR_SPRn_Pos))
#define		SPI_CLK_DIV_256		((0x1<< SPI_SPCR_SPR2_Pos) | (0x2<< SPI_SPCR_SPRn_Pos))
#define		SPI_CLK_DIV_512		((0x1<< SPI_SPCR_SPR2_Pos) | (0x3<< SPI_SPCR_SPRn_Pos))

/*----------------------------------------------------------------------------
 **SPI 时钟极性
/*---------------------------------------------------------------------------*/
#define		SPI_CLK_CPOL_LOW	(0x00) 						/*SPI时钟空闲时为低*/
#define		SPI_CLK_CPOL_HIGH	(0x01<< SPI_SPCR_CPOL_Pos)	/*SPI时钟空闲时为高*/

/*----------------------------------------------------------------------------
 **SPI 时钟相位
/*---------------------------------------------------------------------------*/
#define		SPI_CLK_CPHA_0		(0x00)						/*SPI先发数据再发时钟，即在时钟偶数沿采样*/
#define		SPI_CLK_CPHA_1		(0x01<< SPI_SPCR_CPHA_Pos)	/*SPI先发时钟再发数据，即在时钟奇数沿采样*/
/*----------------------------------------------------------------------------
 **SPI 主控模式NSSx片选信号
 ** 注：在SPI_NSS_AUTO_CONTROL自动控制模式中：写SPIBuffer就会自动发送片选(SSCR中的内容)+数据，但发送完1Byte数据后，SPI会自动拉高NSSx
/*---------------------------------------------------------------------------*/
#define		SPI_NSS_SSCR_CONTROL		(0x0)			/*NSSx一直输出寄存器SSCR的内容*/
#define		SPI_NSS_AUTO_CONTROL		(0x01)			/*SPI空闲时NSSx输出高电平，SPI传输时输出SSCR的内容*/

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
 ** \brief	SPI_Start
 **			开启SPI模块
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SPI_Start(void);
/*****************************************************************************
 ** \brief	SPI_Stop
 **			关闭SPI模块
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SPI_Stop(void);
/*****************************************************************************
 ** \brief	SPI_ConfigClk
 **			配置SPI运行时钟
 ** \param [in] SPIClkDiv： (1)SPI_CLK_DIV_4 ~ SPI_CLK_DIV_512
 ** \return  none
 ** \note	 Fspi = Fsys / SPIClkDiv;
*****************************************************************************/
void SPI_ConfigClk(uint8_t SPIClkDiv);
/*****************************************************************************
 ** \brief	SPI_ConfigRunMode
 **			配置SPI运行模式
 ** \param [in] ClkPositive: 时钟极性
 **							 (1)SPI_CLK_CPOL_LOW      ：SPI时钟空闲时为低
 **							 (2)SPI_CLK_CPOL_HIGH	  ：SPI时钟空闲时为高
 **				ClkPhash: 时钟相位
 **							  (1)SPI_CLK_CPHA_0 SPI先发数据再发时钟，即在时钟偶数沿采样
 **							  (2)SPI_CLK_CPHA_1:SPI先发时钟再发数据，即在时钟奇数沿采样
 **				NSSMode: NSS脚控制选择(只用于主控模式)
 **							  (1)SPI_NSS_SSCR_CONTROL:NSSx一直输出寄存器SSCR的内容
 **							  (2)SPI_NSS_AUTO_CONTROL:SPI空闲时NSSx输出高电平，SPI传输时输出SSCR的内容
 ** \return  none
 ** \note	
*****************************************************************************/
void SPI_ConfigRunMode(uint8_t ClkPositive, uint8_t ClkPhash, uint8_t NSSMode);

/*****************************************************************************
 ** \brief	SPI_EnableMasterMode
 **			使能主控模式
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SPI_EnableMasterMode(void);
/*****************************************************************************
 ** \brief	SPI_DisableMasterMode
 **			关闭主控模式
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SPI_DisableMasterMode(void);
/*****************************************************************************
 ** \brief	SPI_EnableSlaveMode
 **			使能从机模式
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SPI_EnableSlaveMode(void);
/*****************************************************************************
 ** \brief	SPI_DisableSlaveMode
 **			关闭从机模式
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SPI_DisableSlaveMode(void);

/*****************************************************************************
 ** \brief	SPI_GetTransferIntFlag
 **			获取传输完毕中断标志位
 ** \param [in] none
 ** \return  1:SPI传输完毕  0:SPI未传输完毕
 ** \note	 
*****************************************************************************/
uint8_t SPI_GetTransferIntFlag(void);
/*****************************************************************************
 ** \brief	SPI_ClearTransferIntFlag
 **			清除传输完毕中断标志位
 ** \param [in] none
 ** \return  note
 ** \note	 
*****************************************************************************/
void SPI_ClearTransferIntFlag(void);
/*****************************************************************************
 ** \brief	SPI_GetWriteErrorIntFlag
 **			获取写冲突中断标志位
 ** \param [in] none
 ** \return  1:SPI发生写冲突  0:SPI无写冲突
 ** \note	 
*****************************************************************************/
uint8_t SPI_GetWriteErrorIntFlag(void);
/*****************************************************************************
 ** \brief	SPI_ClearWriteErrorIntFlag
 **			清除写冲突中断标志位
 ** \param [in] none
 ** \return none
 ** \note	 
*****************************************************************************/
void SPI_ClearWriteErrorIntFlag(void);
/*****************************************************************************
 ** \brief	SPI_EnableIntFlag
 **			开启SPI中断
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SPI_EnableIntFlag(void);
/*****************************************************************************
 ** \brief	SPI_DisableIntFlag
 **			关闭SPI中断
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SPI_DisableIntFlag(void);
/*****************************************************************************
 ** \brief	 SPI_GetIntFlag
 **			 获取SPI完成中断标志位
 ** \param [in]  none
 ** \return  0：无中断，1：产生中断
 ** \note 
*****************************************************************************/
uint8_t SPI_GetIntFlag(void);

/*****************************************************************************
 ** \brief	 SPI_ClearIntFlag
 **			 清SPI完成中断标志位
 ** \param [in] none
 ** \return  none
 ** \note   
*****************************************************************************/
void SPI_ClearIntFlag(void);




#endif /* __SPI_H__ */