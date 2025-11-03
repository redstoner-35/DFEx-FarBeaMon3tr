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
/** \file pga.c
**
**  
**
**	History:
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "pga.h"

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
 ** \brief	 PGA_EnableNormalMode
 **			 使能PGA正常运行模式
 ** \param [in] RunMode:  (1)PGA_NORM_SINGLE		正常模式：单端输入模式
 **						  (2)PGA_NORM_SHAM_DIFFERENCE	正常模式：伪差分输入模式
 ** \return  none
 ** \note   
 *****************************************************************************/
void PGA_EnableNormalMode(uint8_t PGAMode)
{
	uint8_t Temp=0;
	
	PGACON3 &= ~(PGA_PGACON3_PGAMODE_Msk);
	Temp = PGACON0;
	Temp &= ~(PGA_PGACON0_PGAMS_Msk);
	Temp |= PGAMode<<PGA_PGACON0_PGAMS_Pos;
	PGACON0 = Temp;
}

/*****************************************************************************
 ** \brief	 PGA_ConfigInput
 **			 设置PGA输入
 ** \param [in] Port:  (1)PGA_SIG_PGAP		PGAP端口
 **						  (2)PGA_SIG_PGAGND		PGAGND端口
 **						  (3)PGA_DIF_H_PGAP_L_PGAGND	差分模式：高端接PGAP，低端接PGAGAND
 **						  (4)PGA_DIF_H_PGAGND_L_PGAP	差分模式：高端接PGAGND，低端接PGAP				
 ** \return  none
 ** \note   
 *****************************************************************************/
void PGA_ConfigInput(uint8_t Port)
{
	uint8_t Temp=0;
	
	Temp = PGACON0;
	Temp &= ~(PGA_PGACON0_PGAPS_Msk);
	Temp |= Port;
	PGACON0 = Temp;
}

/*****************************************************************************
 ** \brief	 PGA_EnableOutput
 **			 使能PGA输出
 ** \param [in] none
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_EnableOutput(void)
{
	PGACON2 |= PGA_PGACON2_PGATEN_Msk;
}
/*****************************************************************************
 ** \brief	 PGA_DisableOutput
 **			 关闭PGA输出
 ** \param [in] none
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_DisableOutput(void)
{
	PGACON2 &= ~(PGA_PGACON2_PGATEN_Msk);	
}

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
void PGA_ConfigGain(uint8_t Gain)
{
	uint8_t Temp=0;
	
	Temp = PGACON0;
	Temp &= ~(PGA_PGACON0_PGAGS_Msk);
	Temp |= Gain<<PGA_PGACON0_PGAGS_Pos;
	PGACON0 = Temp;
}

/*****************************************************************************
 ** \brief	 PGA_ConfigOffsetAdj
 **			 配置PGA失调电压调节方式
 ** \param [in] OffsetAdj:	(1)PGA_OFFSET_PGAADJ
 **							(2)PGA_OFFSET_CONFIG
 **				AdjVlue : 失调电压调节值：0x00~0x3f
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_ConfigOffsetAdj(uint8_t OffsetAdj, uint8_t AdjVlue)
{
	uint8_t Temp=0;
	
	Temp = PGACON3;
	Temp &= ~(PGA_PGACON3_PGAADJ_Msk);
	Temp |= AdjVlue;
	PGACON3 = Temp;
	PGAADJE = OffsetAdj;
}


/*****************************************************************************
 ** \brief	 PGA_EnableSHA
 **			 开启PGA采样保持器
 ** \param [in] none
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_EnableSHA(void)
{
	PGACON1 &= ~(PGA_PGACON1_PGAAMS_Msk);
}

/*****************************************************************************
 ** \brief	 PGA_DisableSHA
 **			 关闭PGA采样保持器
 ** \param [in] none
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_DisableSHA(void)
{
	PGACON1 |= PGA_PGACON1_PGAAMS_Msk;
}

/*****************************************************************************
 ** \brief	 PGA_ConfigSampleTime
 **			 设置PGA采样保持器采样保持时间
 ** \param [in] SampleTime: PGA_SHT_1 ... PGA_SHT_16
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_ConfigSampleTime(uint8_t SampleTime)
{
	uint8_t Temp=0;
	
	Temp = PGACON1;
	Temp &= ~(PGA_PGACON1_PGASHT_Msk);
	Temp |= SampleTime;
	PGACON1 = Temp;
}


/*****************************************************************************
 ** \brief	 PGA_Start
 **			 开启PGA
 ** \param [in]none
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_Start(void)
{
	PGACON0 |= PGA_PGACON0_PGAEN_Msk;
}
/*****************************************************************************
 ** \brief	 PGA_Stop
 **			 关闭PGA
 ** \param [in]none
 ** \return  none
 ** \note   
*****************************************************************************/
void PGA_Stop(void)
{
	PGACON0 &= ~(PGA_PGACON0_PGAEN_Msk);
}


/*****************************************************************************
 ** \brief	 PGA_GetOutValue
 **			 获取PGA输出值
 ** \param [in]none
 ** \return  0/1
 ** \note   
*****************************************************************************/
static uint8_t PGA_GetOutValue(void)
{
	return((PGACON2 & PGA_PGACON2_PGADOUT_Msk)? 1:0);
}


/*****************************************************************************
 ** \brief	 PGA_Delay
 **			 延时函数
 ** \param [in]none
 ** \return  none
 ** \note   
*****************************************************************************/
static void PGA_Delay(void)
{
	volatile uint8_t i;
	for(i=0;i<50;i++)
		_nop_();
}

/*****************************************************************************
 ** \brief	 PGA_GetOffsetAdjValue
 **			 获取PGA失调电压调节值
 ** \param [in] AdjMode:  (1)PGA_ADJ_MODE0			内部输入短接到地
 **						  (2)PGA_ADJ_MODE1			内部输入短接，外部从PGAP给不同的共模点
 **						  (3)PGA_ADJ_MODE2			内部输入短接，外部从PGAGND给不同的共模点 			
 ** \return  5位失调电压修调值
 ** \note   
 *****************************************************************************/
uint8_t  PGA_GetOffsetAdjValue(uint8_t AdjMode)
{
	uint8_t temp, PgaOut,Adjvalue, PGACON3Temp;
	Adjvalue =0;
	
	/*(1)关闭PGA*/
	PGA_Stop();	
	PGA_DisableSHA();	
	/*(2)设置PGA的修调模式*/
	if( PGA_ADJ_MODE0 == AdjMode)
	{
		PGACON3 = (0x1<<6);
	}
	if( PGA_ADJ_MODE1 == AdjMode)
	{
		PGACON3 = (0x2<<6);
		P31CFG = 0x01;			//设置P31为PGAP		
	}	
	if( PGA_ADJ_MODE2 == AdjMode)
	{
		PGACON3 = (0x3<<6);
		P32CFG = 0x01;			//设置P32为PGAGND			
	}
	/*(3)设置PGA的修调值来源为PGAADJ<5:0>*/
	PGAADJE = 0xAA;
	/*(4)开启PGA*/
	PGA_Start();
	/*(5)循环扫描PGA的输出*/	
	PGACON3Temp = PGACON3;
	
	PGA_Delay();	
	PgaOut = PGA_GetOutValue();
	for(temp=0; temp<0x40; temp++)
	{
		PGACON3Temp &= 0xc0;
		PGACON3Temp |= temp;
		PGACON3 = PGACON3Temp;
		PGA_Delay();
		if(	PgaOut != PGA_GetOutValue()) 
		{
			Adjvalue = temp;		/*记录修调值*/	
			break;
		}	
	}
	/*(6)退出PGA修调模式*/
	PGA_Stop();
	PGACON3 = 0x00;
	return 	Adjvalue;
}





