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

/****************************************************************************/
/** \file spi.c
**
** 
**
**	History:
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "spi.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
*****************************************************************************/

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
*****************************************************************************/

/****************************************************************************/
/*	Local type definitions('typedef')
*****************************************************************************/

/****************************************************************************/
/*	Local variable  definitions('static')
*****************************************************************************/

/****************************************************************************/
/*	Local function prototypes('static')
*****************************************************************************/

/****************************************************************************/
/*	Function implementation - global ('extern') and local('static')
*****************************************************************************/
/*****************************************************************************
 ** \brief	SPI_Start
 **			开启SPI模块
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SPI_Start(void)
{
	SPCR |= SPI_SPCR_SPEN_Msk;
}
/*****************************************************************************
 ** \brief	SPI_Stop
 **			关闭SPI模块
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SPI_Stop(void)
{
	SPCR &= ~(SPI_SPCR_SPEN_Msk);
}
/*****************************************************************************
 ** \brief	SPI_ConfigClk
 **			配置SPI运行时钟
 ** \param [in] SPIClkDiv： (1)SPI_CLK_DIV_4 ~ SPI_CLK_DIV_512
 ** \return  none
 ** \note	 Fspi = Fsys / SPIClkDiv;
*****************************************************************************/
void SPI_ConfigClk(uint8_t SPIClkDiv)
{
	uint8_t Temp=0;
	
	Temp = SPCR;
	Temp &= ~(SPI_SPCR_SPR2_Msk | SPI_SPCR_SPRn_Msk);
	Temp |= SPIClkDiv;
	SPCR = Temp;
}
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
void SPI_ConfigRunMode(uint8_t ClkPositive, uint8_t ClkPhash, uint8_t NSSMode)
{
	uint8_t Temp=0;
	
	Temp = SPCR;
	Temp &= ~(SPI_SPCR_CPOL_Msk | SPI_SPCR_CPHA_Msk);
	Temp |= (ClkPositive | ClkPhash);
	SPCR = Temp;
	
	Temp = SPSR;
	Temp &= ~(SPI_SPSR_SSCEN_Msk);
	Temp |= NSSMode;
	SPSR = Temp;	
}

/*****************************************************************************
 ** \brief	SPI_EnableMasterMode
 **			使能主控模式
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SPI_EnableMasterMode(void)
{
	SPCR |= (SPI_SPCR_MSTR_Msk);
}
/*****************************************************************************
 ** \brief	SPI_DisableMasterMode
 **			关闭主控模式
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SPI_DisableMasterMode(void)
{
	SPCR &= ~(SPI_SPCR_MSTR_Msk);
}
/*****************************************************************************
 ** \brief	SPI_EnableSlaveMode
 **			使能从机模式
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SPI_EnableSlaveMode(void)
{
	SPCR &= ~(SPI_SPCR_MSTR_Msk);

}
/*****************************************************************************
 ** \brief	SPI_DisableSlaveMode
 **			关闭从机模式
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SPI_DisableSlaveMode(void)
{
	SPCR |= (SPI_SPCR_MSTR_Msk);	
}


/*****************************************************************************
 ** \brief	SPI_GetTransferIntFlag
 **			获取传输完毕中断标志位
 ** \param [in] none
 ** \return  1:SPI传输完毕  0:SPI未传输完毕
 ** \note	 
*****************************************************************************/
uint8_t SPI_GetTransferIntFlag(void)

{
	return ((SPSR & SPI_SPSR_SPISIF_Msk)? 1:0);
}
/*****************************************************************************
 ** \brief	SPI_ClearTransferIntFlag
 **			清除传输完毕中断标志位
 ** \param [in] none
 ** \return  note
 ** \note	 
*****************************************************************************/
void SPI_ClearTransferIntFlag(void)
{
	uint8_t temp;
	temp = SPSR;
	temp = SPDR;
}
/*****************************************************************************
 ** \brief	SPI_GetWriteErrorIntFlag
 **			获取写冲突中断标志位
 ** \param [in] none
 ** \return  1:SPI发生写冲突  0:SPI无写冲突
 ** \note	 
*****************************************************************************/
uint8_t SPI_GetWriteErrorIntFlag(void)
{
	return ((SPSR & SPI_SPSR_WCOL_Msk)? 1:0);	
}
/*****************************************************************************
 ** \brief	SPI_ClearWriteErrorIntFlag
 **			清除写冲突中断标志位
 ** \param [in] none
 ** \return none
 ** \note	 
*****************************************************************************/
void SPI_ClearWriteErrorIntFlag(void)
{
	uint8_t temp;
	temp = SPSR;
	temp = SPDR;
}
/*****************************************************************************
 ** \brief	SPI_EnableIntFlag
 **			开启SPI中断
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SPI_EnableIntFlag(void)
{
	EIE2 |= IRQ_EIE2_SPIIE_Msk;
}
/*****************************************************************************
 ** \brief	SPI_DisableIntFlag
 **			关闭SPI中断
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SPI_DisableIntFlag(void)
{
	EIE2 &= ~(IRQ_EIE2_SPIIE_Msk);	
}

/*****************************************************************************
 ** \brief	 SPI_GetIntFlag
 **			 获取SPI完成中断标志位
 ** \param [in]  none
 ** \return  0：无中断，1：产生中断
 ** \note 
*****************************************************************************/
uint8_t SPI_GetIntFlag(void)
{
	return ((EIF2 & IRQ_EIF2_SPIIF_Msk) ? 1:0);
}
/*****************************************************************************
 ** \brief	 SPI_ClearIntFlag
 **			 清SPI完成中断标志位
 ** \param [in] none
 ** \return  none
 ** \note   
*****************************************************************************/
void SPI_ClearIntFlag(void)
{
	EIF2 = 0xFF &(~(IRQ_EIF2_SPIIF_Msk));
}





