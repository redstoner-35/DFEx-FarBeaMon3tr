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
/** \file lse.c
**
**  
**
**	History:
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "lse.h"

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
 ** \brief	 LSE_EnableLSE
 **			 使能LSE模块
 ** \param [in] none
 ** \return 
 ** \note   
 *****************************************************************************/
void  LSE_EnableLSE(void)
{
	LSECON |= LSE_LSECON_LSEEN_Msk;
}
/*****************************************************************************
 ** \brief	 SYS_DisableLSE
 **			 关闭LSE模块
 ** \param [in] none
 ** \return 
 ** \note   
 *****************************************************************************/
void  LSE_DisableLSE(void)
{
	LSECON &= ~(LSE_LSECON_LSEEN_Msk);
}

/*****************************************************************************
 ** \brief	 LSE_Start
 **			 使能LSE计数
 ** \param [in] none
 ** \return 
 ** \note   
 *****************************************************************************/
void LSE_Start(void)
{
	LSECON |=LSE_LSECON_LSECNTEN_Msk;
}
/*****************************************************************************
 ** \brief	 LSE_Stop
 **			 停止LSE计数
 ** \param [in] none
 ** \return 
 ** \note   
 *****************************************************************************/
void LSE_Stop(void)
{
	LSECON &= ~(LSE_LSECON_LSECNTEN_Msk);
}

/*****************************************************************************
 ** \brief	 LSE_ConfigLSE
 **			 设置LSE定时时间
 ** \param [in] Time: 16位定时时间
 ** \return none
 ** \note   
 *****************************************************************************/
void LSE_ConfigLSE(uint16_t  Time)
{
	LSECRH = (Time>>8);
	LSECRL = Time;
}

/*****************************************************************************
 ** \brief	 LSE_EnableLSEWakeup
 **			 使能LSE定时唤醒功能
 ** \param [in] none
 ** \return 
 ** \note   
 *****************************************************************************/
void LSE_EnableLSEWakeup(void)
{
	LSECON |= (LSE_LSECON_LSEWUEN_Msk);
}

/*****************************************************************************
 ** \brief	 LSE_DisableLSEWakeup
 **			 关闭LSE定时唤醒功能
 ** \param [in] none
 ** \return none
 ** \note    
 *****************************************************************************/
void LSE_DisableLSEWakeup(void)
{
	LSECON &= ~(LSE_LSECON_LSEWUEN_Msk);
}

/*****************************************************************************
 ** \brief	 LSE_EnableLSEInt
 **			 使能LSE中断
 ** \param [in] none
 ** \return none
 ** \note   
 *****************************************************************************/
void LSE_EnableLSEInt(void)
{
	LSECON |= LSE_LSECON_LSEIE_Msk;
}
/*****************************************************************************
 ** \brief	LSE_DisableLSEInt
 **			 关闭LSE中断
 ** \param [in] none
 ** \return none
 ** \note   
 *****************************************************************************/
void LSE_DisableLSEInt(void)
{
	LSECON &= ~(LSE_LSECON_LSEIE_Msk);
}
/*****************************************************************************
 ** \brief	 LSE_GetLSEIntFlag
 **			 获取LSE中断标志位
 ** \param [in] none
 ** \return 1：产生中断  0：无中断
 ** \note  
 *****************************************************************************/
uint8_t  LSE_GetLSEIntFlag(void)
{
	return((LSECON & LSE_LSECON_LSEIF_Msk)? 1:0);
}
/*****************************************************************************
 ** \brief	 LSE_ClearLSEIntFlag
 **			 清除LSE中断标志位
 ** \param [in] none
 ** \return none
 ** \note   
 *****************************************************************************/
void  LSE_ClearLSEIntFlag(void)
{
	LSECON &= ~(LSE_LSECON_LSEIF_Msk);
}
/*****************************************************************************
 ** \brief	 LSE_GetLSEState
 **			 获取LSE稳定状态位
 ** \param [in] none
 ** \return 1：稳定  0：未稳定
 ** \note  
 *****************************************************************************/
uint8_t  LSE_GetLSEState(void)
{
	return((LSECON & LSE_LSECON_LSESTA_Msk)? 1:0);
}





