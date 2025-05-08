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
/** \file buzzer.c
**
** 
**
**	History:
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "buzzer.h"

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
/****************************************************************************
 ** \brief	 BUZ_EnableBuzzer
 **			 ¿ªÆô·äÃùÆ÷
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void BUZ_EnableBuzzer(void)
{
	BUZCON |= BUZZER_BUZCON_BUZEN_Msk;
}
/*****************************************************************************
 ** \brief	 BUZ_DisableBuzzer
 **			 ¹Ø±Õ·äÃùÆ÷
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void BUZ_DisableBuzzer(void)
{
	BUZCON &= ~(BUZZER_BUZCON_BUZEN_Msk);
}

/*****************************************************************************
 ** \brief	 BUZ_ConfigBuzzer
 **			 ÅäÖÃ·äÃùÆ÷
 ** \param [in] BuzCKS£º(1)BUZ_CKS_8
 **							  (2)BUZ_CKS_16
 **							  (3)BUZ_CKS_32
 **							  (4)BUZ_CKS_64
 **				CLKDiv: 	   (1)0x00   : BuzzerÊä³öµÍµçÆ½
 **							   (2) 0x1~0xff 
 ** \return  none
 ** \note	 ÆµÂÊ¼ÆËã¹«Ê½£º Fbuz = Fsys/(2 * BuzCKS*CLKDiv)
*****************************************************************************/
void BUZ_ConfigBuzzer(uint8_t BuzCKS, uint8_t CLKDiv)
{
	uint8_t Temp=0;
	
	Temp = BUZCON;
	Temp &= ~(BUZZER_BUZCON_BUZCKS_Msk);
	Temp |= BuzCKS;
	BUZCON = Temp;
	
	BUZDIV = CLKDiv;	
}