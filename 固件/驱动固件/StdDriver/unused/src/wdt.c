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
/** \file wdt.c
**
**  
**
**	History:
**
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "wdt.h"

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
 ** \brief	 WDT_ConfigOverflowTime
 **			设置WDT溢出时间
 ** \param [in] TsysCoefficient : (1)WDT_CLK_131072 	
 **								  (2)WDT_CLK_262144 
 **								  (3)WDT_CLK_524288
 **								  (4)WDT_CLK_1048576 
 **								  (5)WDT_CLK_2097152 
 **								  (6)WDT_CLK_4194304 
 **								  (7)WDT_CLK_16777216 
 **								  (8)WDT_CLK_67108864 
 ** \return none 
 ** \note	若Fsys =16Mhz, 则     (1)WDT_CLK_131072  , 溢出时间= Tsys * 131072 = 8.192ms
 **								  (2)WDT_CLK_262144 , 溢出时间= Tsys * 262144 = 16.384ms
 **								  (3)WDT_CLK_524288 , 溢出时间= Tsys * 524288 = 32.768ms
 **								  (4)WDT_CLK_1048576 , 溢出时间= Tsys * 1048576 = 65.536ms
 **								  (5)WDT_CLK_2097152 , 溢出时间= Tsys * 2097152 = 131.072ms
 **								  (6)WDT_CLK_4194304 , 溢出时间= Tsys * 4194304 = 262.144ms
 **								  (7)WDT_CLK_16777216 , 溢出时间= Tsys * 16777216 = 1.048s 
 **								  (8)WDT_CLK_67108864 , 溢出时间= Tsys * 67108864 = 4.194s 
 *****************************************************************************/
void  WDT_ConfigOverflowTime(uint8_t TsysCoefficient)
{
	uint8_t Temp=0;
	
	Temp = CKCON;
	Temp &= ~(TMR_CKCON_WTSn_Msk);
	Temp |= (TsysCoefficient << TMR_CKCON_WTSn_Pos);
	CKCON = Temp;
}
/*****************************************************************************
 ** \brief	 WDT_ClearWDT
 **			清除WDT计数器
 ** \param [in] none
 **
 ** \return none 
 ** \note
 *****************************************************************************/
void WDT_ClearWDT(void)
{
	if(1 == EA)
	{
		EA = 0;			//在CPU_WAITCLOCK选择1T的模式时，在EA=0后必须加nop,选择多T时不加。
		_nop_();
		TA = 0xAA;
		TA = 0x55;
		WDCON |= WDT_WDCON_WDTCLR_Msk;	
		EA = 1;
	}
	else
	{
		TA = 0xAA;
		TA = 0x55;
		WDCON |= WDT_WDCON_WDTCLR_Msk;		
	}	
}
/*****************************************************************************
 ** \brief	 WDT_EnableOverflowInt
 **			 使能WDT计数溢出中断
 ** \param [in] none
 **
 ** \return none 
 ** \note
 *****************************************************************************/
void WDT_EnableOverflowInt(void)
{
	EIE2 |= IRQ_EIE2_WDTIE_Msk;
}
/*****************************************************************************
 ** \brief	 WDT_EnableOverflowInt
 **			 关闭WDT计数溢出中断
 ** \param [in] none
 **
 ** \return none 
 ** \note
 *****************************************************************************/
 void WDT_DisableOverflowInt(void)
 {
	EIE2 &= ~(IRQ_EIE2_WDTIE_Msk);
 }
/*****************************************************************************
 ** \brief	 WDT_GetOverflowIntFlag
 **			 获取WDT计数溢出中断标志
 ** \param [in] none
 **
 ** \return 0：无中断，1：有中断
 ** \note
 *****************************************************************************/
uint8_t WDT_GetOverflowIntFlag(void)
{
	return ((WDCON & WDT_WDCON_WDTIF_Msk)? 1:0);
}
/*****************************************************************************
 ** \brief	 WDT_ClearOverflowIntFlag
 **			 清除WDT计数溢出中断标志
 ** \param [in] none
 **
 ** \return 0：无中断，1：有中断
 ** \note
 *****************************************************************************/
void WDT_ClearOverflowIntFlag(void)
{
	if(EA==1) //操作TA时序时不允许被打断，因而需要关闭中断
	{
		EA=0;		//在CPU_WAITCLOCK选择1T的模式时，在EA=0后必须加nop,选择多T时不加。
		_nop_();
		TA = 0xAA;
		TA = 0x55;
		WDCON &= ~(WDT_WDCON_WDTIF_Msk);
		EA=1;
	}
	else
	{
		TA = 0xAA;
		TA = 0x55;
		WDCON &= ~(WDT_WDCON_WDTIF_Msk);
	}
}




