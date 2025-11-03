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
/** \file i2c.c
**
** 
**
**	History:
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "i2c.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/

/****************************************************************************/
/*	Local function prototypes('static')
****************************************************************************/

/****************************************************************************/
/*	Function implementation - global ('extern') and local('static')
****************************************************************************/
/*****************************************************************************
 ** \brief	I2C_ConfigCLK
 **			配置I2C的时钟
 ** \param [in] I2CMtp: 0x0~0x7f
 ** \return  none
 ** \note	(1)I2CMtp = 0 ,SCL = 3*10*Tsys
 **			(2)I2CMtp != 0, SCL = 2*(1+I2CMtp)*10*Tsys
*****************************************************************************/
void I2C_ConfigCLK(uint8_t I2CMtp)
{
	I2CMTP = I2CMtp;
}
/*****************************************************************************
 ** \brief	I2C_EnableMasterMode
 **			使能主控模式
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void I2C_EnableMasterMode(void)
{
	I2CMCR = 0x00;
	I2CSCR  = 0x00;
}
/*****************************************************************************
 ** \brief	I2C_DisableMasterMode
 **			关闭主控模式
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void I2C_DisableMasterMode(void)
{
	;
}

/*****************************************************************************
 ** \brief	I2C_GetMasterSendAddrFlag
 **			获取主控模式寻址应答标志
 ** \param [in] none
 ** \return 0: 从机有应答  1；从机无应答
 ** \note	 
*****************************************************************************/
uint8_t I2C_GetMasterSendAddrFlag(void)
{
	return ((I2CMSR & I2C_I2CMSR_ADD_ACK_Msk)? 1:0);
}

/*****************************************************************************
 ** \brief	I2C_GetBusArbitrationFlag
 **			获取主控模总线仲裁标志
 ** \param [in] none
 ** \return 0: 正常  1:丢失了总线控制权
 ** \note	 
*****************************************************************************/
uint8_t I2C_GetBusArbitrationFlag(void)
{
	return ((I2CMSR & I2C_I2CMSR_ARB_LOST_Msk)? 1:0);	
}
/*****************************************************************************
 ** \brief	I2C_GetMasterErrorFlag
 **			获取主控模式错误标志
 ** \param [in] none
 ** \return 0: 无错误 1；有错误
 ** \note	错误标志产生条件：(1)寻址从机无应答 (2)发送数据从机无应答
 **							  (3)I2C总线仲裁冲突
*****************************************************************************/
uint8_t I2C_GetMasterErrorFlag(void)
{
	return ((I2CMSR & I2C_I2CMSR_ERROR_Msk)? 1:0);	
}
/*****************************************************************************
 ** \brief	I2C_GetMasterBusyFlag
 **			获取主控模块Busy状态标志
 ** \param [in] none
 ** \return  1；正在发送
 ** \note	
*****************************************************************************/
uint8_t I2C_GetMasterBusyFlag(void)
{
	return ((I2CMSR & I2C_I2CMSR_BUSY_Msk)? 1:0);	
}
/*****************************************************************************
 ** \brief	I2C_GetI2CBusBusyFlag
 **			获取总线忙状态标志
 ** \param [in] none
 ** \return 0: 空闲 1；总线忙
 ** \note	
*****************************************************************************/
uint8_t I2C_GetI2CBusBusyFlag(void)
{
	return ((I2CMSR & I2C_I2CMSR_BUS_BUSY_Msk)? 1:0);	
}
/*****************************************************************************
 ** \brief	I2C_GetMasterIdleFlag
 **			获取主控模式空闲状态标志
 ** \param [in] none
 ** \return 0: 工作 1；空闲
 ** \note	
*****************************************************************************/
uint8_t I2C_GetMasterIdleFlag(void)
{
	return ((I2CMSR & I2C_I2CMSR_IDLE_Msk)? 1:0);	
}

/*****************************************************************************
 ** \brief	I2C_EnableInt
 **			开启中断
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void I2C_EnableInt(void)
{
	EIE2 |= IRQ_EIE2_I2CIE_Msk;
}
/*****************************************************************************
 ** \brief	I2C_DisableInt
 **			关闭中断
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void I2C_DisableInt(void)
{
	EIE2 &= ~(IRQ_EIE2_I2CIE_Msk);
}
/*****************************************************************************
 ** \brief	I2C_GetMasterIntFlag
 **			获取主控模式下的中断标志位
 ** \param [in] none
 ** \return  0:无中断 1：有中断
 ** \note	 
*****************************************************************************/
uint8_t I2C_GetMasterIntFlag(void)
{
	return ((I2CMSR & I2C_I2CMSR_I2CMIF_Msk)? 1:0);	
}
/*****************************************************************************
 ** \brief	I2C_ClearMasterIntFlag
 **			清除主控模式下的中断标志位
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void I2C_ClearMasterIntFlag(void)
{
	I2CMSR = 0x00;
}


