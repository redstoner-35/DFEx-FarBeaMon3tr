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
/** \file system.c
**
**  
**
**	History:
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "system.h"

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
 ** \brief	 SYS_EnableLVD
 **			 开启电压监测功能
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SYS_EnableLVD(void)
{
	LVDCON |= LVD_LVDCON_LVDEN_Msk;
}
/*****************************************************************************
 ** \brief	 SYS_DisableLVD
 **			  关闭电压监测功能
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SYS_DisableLVD(void)
{
	LVDCON &= ~(LVD_LVDCON_LVDEN_Msk);	
}
/*****************************************************************************
 ** \brief	 SYS_ConfigLVD
 **			 配置系统电压监测电压
 ** \param [in] LVDValue :(1)SYS_LVD_2_0V
 **						  (2)SYS_LVD_2_2V
 **						  (3)SYS_LVD_2_4V
 **						  (4)SYS_LVD_2_7V
 **						  (5)SYS_LVD_3_0V
 **						  (6)SYS_LVD_3_7V
 **						  (7)SYS_LVD_4_0V
 **						  (7)SYS_LVD_4_3V
 ** \return  none
 ** \note	 
*****************************************************************************/
void SYS_ConfigLVD(uint8_t LVDValue)
{
	uint8_t Temp=0;
	
	Temp = LVDCON;
	Temp &= ~(LVD_LVDCON_LVDSEL_Msk);
	Temp |= LVDValue;	
	LVDCON = Temp;	
}

/*****************************************************************************
 ** \brief	 SYS_EnableLVDInt
 **			 开启电压监测中断功能
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SYS_EnableLVDInt(void)
{
	LVDCON |= LVD_LVDCON_LVDINTE_Msk;	
}
/*****************************************************************************
 ** \brief	 SYS_DisableLVDInt
 **			  关闭电压监测中断功能
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SYS_DisableLVDInt(void)
{
	LVDCON &= ~(LVD_LVDCON_LVDINTE_Msk);	
}

/*****************************************************************************
 ** \brief	 SYS_GetLVDIntFlag
 **			 获取LVD中断标志位
 ** \param [in] none
 ** \return  0:无中断 1：有中断
 ** \note	 
*****************************************************************************/
uint8_t SYS_GetLVDIntFlag(void)
{
	return((LVDCON & LVD_LVDCON_LVDINTF_Msk)? 1:0);
}

/*****************************************************************************
 ** \brief	 SYS_ClearLVDIntFlag
 **			 清除LVD中断标志位
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SYS_ClearLVDIntFlag(void)
{
	LVDCON	&= ~(LVD_LVDCON_LVDINTF_Msk);
}


/*****************************************************************************
 ** \brief	 SYS_EnableWDTReset
 **			 使能WDT复位System
 ** \param [in] none
 **
 ** \return none
 ** \note
 *****************************************************************************/
void SYS_EnableWDTReset(void)
{
	if(EA==1) //操作TA时序时不允许被打断，因而需要关闭中断
	{
		EA=0;		//在CPU_WAITCLOCK选择1T的模式时，在EA=0后必须加nop,选择多T时不加。
		_nop_();
		TA = 0xAA;
		TA = 0x55;
		WDCON |= WDT_WDCON_WDTRE_Msk;
		EA=1;
	}
	else
	{
		TA = 0xAA;
		TA = 0x55;
		WDCON |= WDT_WDCON_WDTRE_Msk;
	}
}
/*****************************************************************************
 ** \brief	 SYS_DisableWDTReset
 **			 关闭WDT复位System
 ** \param [in] none
 **
 ** \return none
 ** \note
 *****************************************************************************/
void SYS_DisableWDTReset(void)
{
	if(EA==1) //操作TA时序时不允许被打断，因而需要关闭中断
	{
		EA=0;		//在CPU_WAITCLOCK选择1T的模式时，在EA=0后必须加nop,选择多T时不加。
		_nop_();
		TA = 0xAA;
		TA = 0x55;
		WDCON &= ~(WDT_WDCON_WDTRE_Msk);
		EA=1;
	}
	else
	{
		TA = 0xAA;
		TA = 0x55;
		WDCON &= ~(WDT_WDCON_WDTRE_Msk);
	}
}
/*****************************************************************************
 ** \brief	 SYS_GetWDTResetFlag
 **			 获取WDT复位System的标志
 ** \param [in] none
 **
 ** \return 0：复位不由WDT溢出引起    1：复位由WDT溢出引起
 ** \note
 *****************************************************************************/
uint8_t SYS_GetWDTResetFlag(void)
{
	return((WDCON & WDT_WDCON_WDTRF_Msk)? 1:0);
}
/*****************************************************************************
 ** \brief	 SYS_ClearWDTResetFlag
 **			 清除WDT复位System的标志
 ** \param [in] none
 **
 ** \return   none
 ** \note
 *****************************************************************************/
void SYS_ClearWDTResetFlag(void)
{
	if(EA==1) //操作TA时序时不允许被打断，因而需要关闭中断
	{
		EA=0;		//在CPU_WAITCLOCK选择1T的模式时，在EA=0后必须加nop,选择多T时不加。
		_nop_();
		TA = 0xAA;
		TA = 0x55;
		WDCON &= ~(WDT_WDCON_WDTRF_Msk);	
		EA=1;
	}
	else
	{
		TA = 0xAA;
		TA = 0x55;
		WDCON &= ~(WDT_WDCON_WDTRF_Msk);

	}
}

/*****************************************************************************
 ** \brief	 SYS_EnableSoftwareReset
 **			 使能软件复位System
 ** \param [in] none
 **
 ** \return none
 ** \note
 *****************************************************************************/
void SYS_EnableSoftwareReset(void)
{
	if(EA==1) //操作TA时序时不允许被打断，因而需要关闭中断
	{
		EA=0;		//在CPU_WAITCLOCK选择1T的模式时，在EA=0后必须加nop,选择多T时不加。
		_nop_();
		TA = 0xAA;
		TA = 0x55;
		WDCON |= WDT_WDCON_SWRST_Msk;	
		EA=1;
	}
	else
	{
		TA = 0xAA;
		TA = 0x55;
		WDCON |= WDT_WDCON_SWRST_Msk;	
	}
}
/*****************************************************************************
 ** \brief	 SYS_DisableSoftwareReset
 **			 关闭软件复位System
 ** \param [in] none
 **
 ** \return none
 ** \note
 *****************************************************************************/
void SYS_DisableSoftwareReset(void)
{
	if(EA==1) //操作TA时序时不允许被打断，因而需要关闭中断
	{
		EA=0;		//在CPU_WAITCLOCK选择1T的模式时，在EA=0后必须加nop,选择多T时不加。
		_nop_();
		TA = 0xAA;
		TA = 0x55;
		WDCON &= ~(WDT_WDCON_SWRST_Msk);	
		EA=1;
	}
	else
	{
		TA = 0xAA;
		TA = 0x55;
		WDCON &= ~(WDT_WDCON_SWRST_Msk);	
	}
}
/*****************************************************************************
 ** \brief	 SYS_GetPowerOnResetFlag
 **			 获取上电复位System的标志
 ** \param [in] none
 **
 ** \return   0：复位不由上电复位引起    1：复位由上电复位引起
 ** \note
 *****************************************************************************/
uint8_t SYS_GetPowerOnResetFlag(void)
{
	return((WDCON & WDT_WDCON_PORF_Msk)? 1:0);
}
/*****************************************************************************
 ** \brief	 SYS_ClearPowerOnResetFlag
 **			 清除PowerOn复位System的标志
 ** \param [in] none
 **
 ** \return   none
 ** \note
 *****************************************************************************/
void SYS_ClearPowerOnResetFlag(void)
{
	WDCON &= ~(WDT_WDCON_PORF_Msk);	
}


/*****************************************************************************
 ** \brief	 SYS_EnableWakeUp
 **			 使能休眠唤醒
 ** \param [in] none
 **
 ** \return none
 ** \note	 可由外部中断、GPIO中断唤醒
 *****************************************************************************/
void SYS_EnableWakeUp(void)
{
	PCON |= SYS_PCON_SWE_Msk;
}
/*****************************************************************************
 ** \brief	 SYS_EnableWakeUp
 **			 关闭休眠唤醒
 ** \param [in] none
 **
 ** \return none
 ** \note	只能由外部复位或者LVR复位唤醒
 *****************************************************************************/
void SYS_DisableWakeUp(void)
{
	PCON &= ~(SYS_PCON_SWE_Msk);
}

/*****************************************************************************
 ** \brief	 SYS_EnterStop
 **			 进入休眠中STOP状态
 ** \param [in] none
 **
 ** \return none
 ** \note	
 *****************************************************************************/
void SYS_EnterStop(void)
{ 
	_nop_();
	_nop_();
	PCON |= SYS_PCON_STOP_Msk;
	_nop_();	
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();	
}
/*****************************************************************************
 ** \brief	 SYS_EnterIdle
 **			 进入休眠模式中Idle状态
 ** \param [in] none
 **
 ** \return none
 ** \note   
 *****************************************************************************/
void SYS_EnterIdle(void)
{
	_nop_();
	_nop_();
	PCON |= SYS_PCON_IDLE_Msk;
	_nop_();	
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
}

/*****************************************************************************
 ** \brief	 SYS_EnableWakeUpTrig
 **			 使能定时唤醒功能
 ** \param [in] none
 **
 ** \return none
 ** \note   
 *****************************************************************************/
void SYS_EnableWakeUpTrig(void)
{
	WUTCRH |= (1<<7);

}
/*****************************************************************************
 ** \brief	 SYS_DisableWakeUpTrig
 **			 关闭定时唤醒功能
 ** \param [in] none
 **
 ** \return none
 ** \note   
 *****************************************************************************/
void SYS_DisableWakeUpTrig(void)
{
	WUTCRH &= ~(1<<7);
}

/*****************************************************************************
 ** \brief	 SYS_ConfigWUTCLK
 **			设置定时唤醒时钟
 ** \param [in] clkdiv: (1)WUT_CLK_DIV_1 
 **						(2)WUT_CLK_DIV_8
 **						(2)WUT_CLK_DIV_32
**						(2)WUT_CLK_DIV_256
 ** \return none
 ** \note   
 *****************************************************************************/
void SYS_ConfigWUTCLK(uint8_t clkdiv )
{
	uint8_t Temp=0;
	
	Temp = WUTCRH;
	Temp &= ~(0x3<<4);
	Temp |= (clkdiv<<4);
	WUTCRH = Temp;	
}

/*****************************************************************************
 ** \brief	 SYS_ConfigWUTTime
 **			设置定时唤醒时间
 ** \param [in] time: 12it（0x0 ~ 0xFFF）
 ** \return none
 ** \note   
 *****************************************************************************/
void SYS_ConfigWUTTime(uint16_t time )
{
	uint8_t Temp=0;
	
	Temp = WUTCRH;
	Temp &=0xf0;
	Temp |= time>>8;
	WUTCRH = Temp;
	
	WUTCRL = time;
}

