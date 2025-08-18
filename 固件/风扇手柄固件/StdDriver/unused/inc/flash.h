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
/** \file flash.h
**
** 
**
** History:
** 
*****************************************************************************/
#ifndef __FLASH_H__
#define __FLASH_H__
/*****************************************************************************/
/* Include files */
/*****************************************************************************/
#include "cms8s6990.h"
/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define') */
/*****************************************************************************/
/*----------------------------------------------------------------------------
 **FLASH 区域
/*---------------------------------------------------------------------------*/
#define	FLASH_CODE			(0x00)
#define	FLASH_DATA			(0x01<< FLASH_MCTRL_MREG_Pos)
/*----------------------------------------------------------------------------
 **FLASH 操作
/*---------------------------------------------------------------------------*/
#define	FLASH_WRITE			((0x2<< FLASH_MCTRL_MMODE_Pos) | FLASH_MCTRL_MSTART_Msk)
#define	FLASH_READ			((0x0<< FLASH_MCTRL_MMODE_Pos) | FLASH_MCTRL_MSTART_Msk)
#define	FLASH_ERASE			((0x3<< FLASH_MCTRL_MMODE_Pos) | FLASH_MCTRL_MSTART_Msk)

#define	FLASH_WRITE_DATA  0x19
#define	FLASH_READ_DATA 	0x11
#define	FLASH_ERASE_DATA  0x1D
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
 ** \brief	FLASH_UnLock
 **			解锁
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void FLASH_UnLock(void);
/*****************************************************************************
 ** \brief	FLASH_Lock
 **			上锁
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void FLASH_Lock(void);

/*****************************************************************************
 ** \brief	FLASH_Write_CodeArea
 **			写程序区缓存
 ** \param [in] Addr: 16bit	  (FLASH_CODE_ADDR: 0x00~0x3FFF;	
 **				Data: 8bit
 ** \return  none
 ** \note	 
*****************************************************************************/
void FLASH_Write_CodeArea( uint16_t Addr, uint8_t Data);

/*****************************************************************************
 ** \brief	FLASH_Write_DataArea
 **			写数据区缓存
 ** \param [in] Addr: 16bit	  FLASH_DATA_ADDR: 0x00~0x3FF;	
 **				Data: 8bit
 ** \return  none
 ** \note	 
*****************************************************************************/
void FLASH_Write_DataArea( uint16_t Addr, uint8_t Data);

/*****************************************************************************
 ** \brief	FLASH_Write
 **			写缓存(程序区或数据区)
 ** \param [in] FLASHModule	：(1)FLASH_DATA:Flash数据空间
 **							  (2)FLASH_CODE：Flash程序空间
 **				Addr: 16bit	  (1)FLASH_DATA_ADDR: 0x00~0x3FF;
 **							  (2)FLASH_CODE_ADDR: 0x00~0x3FFF;	
 **				Data: 8bit
 ** \return  none
 ** \note	 
*****************************************************************************/
void FLASH_Write(uint8_t FLASHModule, uint16_t Addr, uint8_t Data);

/*****************************************************************************
 ** \brief	FLASH_Read_CodeArea
 **			读程序区缓存
 ** \param [in]  Addr: 16bit	  FLASH_CODE_ADDR: 0x00~0x3FFF;	
 ** \return  8bit Data
 ** \note	
*****************************************************************************/
uint8_t FLASH_Read_CodeArea( uint16_t Addr);

/*****************************************************************************
 ** \brief	FLASH_Read_DataArea
 **			读数据区缓存
 ** \param [in]  Addr: 16bit	  FLASH_DATA_ADDR: 0x00~0x3FF;
 ** \return  8bit Data
 ** \note	
*****************************************************************************/
uint8_t FLASH_Read_DataArea( uint16_t Addr);

/*****************************************************************************
 ** \brief	FLASH_Read
 **			读缓存(程序区或数据区)
 ** \param [in]  FLASHModule：(1)FLASH_DATA
 **							  (2)FLASH_CODE
 **				Addr: 16bit	  (1)FLASH_DATA_ADDR: 0x00~0x3FF;
 **							  (2)FLASH_CODE_ADDR: 0x00~0x3FFF;	
 ** \return  8bit Data
 ** \note	
*****************************************************************************/
uint8_t FLASH_Read(uint8_t FLASHModule, uint16_t Addr);

/*****************************************************************************
 ** \brief	FLASH_Erase_CodeArea
 **			擦除程序区
 ** \param [in]  Addr: 16bit	  FLASH_CODE_ADDR: 0x00~0x3FFF;
 ** \return  none
 ** \note	
*****************************************************************************/
void FLASH_Erase_CodeArea( uint16_t Addr);

/*****************************************************************************
 ** \brief	FLASH_Erase_DataArea
 **			擦除数据区
 ** \param [in]  Addr: 16bit	  FLASH_DATA_ADDR: 0x00~0x3FF;
 ** \return  none
 ** \note	
*****************************************************************************/
void FLASH_Erase_DataArea( uint16_t Addr);

/*****************************************************************************
 ** \brief	FLASH_Erase
 **			擦除(程序区或数据区)
 ** \param [in]  FLASHModule：(1)FLASH_DATA
 **							  (2)FLASH_CODE
 **				Addr: 16bit	  (1)FLASH_DATA_ADDR: 0x00~0x3FF;
 **							  (2)FLASH_CODE_ADDR: 0x00~0x3FFF;
 ** \return  none
 ** \note	
*****************************************************************************/
void FLASH_Erase(uint8_t FLASHModule, uint16_t Addr);

#endif /* __FLASH_H__ */
