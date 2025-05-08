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
/** \file opa.c
**
**  
**
**	History:
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "opa.h"

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
 ** \brief	 OPA_SetRunMode
 **			 设置运放运行模式
 ** \param [in] OPAn : OPA0、OPA1
 **				OpaMode:  (1)OPA_MODE_OPA
 **						  (2)OPA_MODE_ACMP
 ** \return  none
 ** \note   
 *****************************************************************************/
void OPA_SetRunMode(uint8_t OPAn, uint8_t OpaMode)
{
	uint8_t Temp=0;
	
	if(OPA0 == OPAn)
	{
		Temp = OP0CON0;
		Temp &= ~(OPA_OP0CON0_OP0FIL_Msk);
		Temp |= (OpaMode << OPA_OP0CON0_OP0FIL_Pos);
		OP0CON0 = Temp;
	}
	if(OPA1 == OPAn)
	{
		Temp = OP1CON0;
		Temp &= ~(OPA_OP1CON0_OP1FIL_Msk);
		Temp |= (OpaMode << OPA_OP1CON0_OP1FIL_Pos);	
		OP1CON0 = Temp;	
	}
}

/*****************************************************************************
 ** \brief	 OPA_ConfigPositive
 **			 设置运放正端输入
 ** \param [in] OPAn : OPA0、OPA1
 **				PositiveSource:  (1)OPA_POSSEL_P	:OPnP
 **								 (2)OPA_POSSEL_BG
 ** \return  none
 ** \note    
 *****************************************************************************/
void OPA_ConfigPositive(uint8_t OPAn, uint8_t PositiveSource)
{
	uint8_t Temp=0;
	
	if(OPA0 == OPAn)
	{
		Temp = OP0CON0;
		Temp &= ~(OPA_OP0CON0_OP0PS_Msk);
		Temp |= (PositiveSource << OPA_OP0CON0_OP0PS_Pos);
		OP0CON0 = Temp;
	}
	if(OPA1 == OPAn)
	{
		Temp = OP1CON0;
		Temp &= ~(OPA_OP1CON0_OP1PS_Msk);
		Temp |= (PositiveSource << OPA_OP1CON0_OP1PS_Pos);	
		OP1CON0 = Temp;
	}
}

/*****************************************************************************
 ** \brief	 OPA_ConfigNegative
 **			 设置运放器负端输入
 ** \param [in] OPAn : OPA0、OPA1
 **				NegativeSource:  (1)OPA_NEGSEL_N		:OPnN			
 ** \return  none
 ** \note   
 *****************************************************************************/
void OPA_ConfigNegative(uint8_t OPAn, uint8_t NegativeSource)
{
	uint8_t Temp=0;
	
	if(OPA0 == OPAn)
	{
		Temp = OP0CON0;
		Temp &= ~(OPA_OP0CON0_OP0NS_Msk);
		Temp |= (NegativeSource << OPA_OP0CON0_OP0NS_Pos);
		OP0CON0 = Temp;
	}
	if(OPA1 == OPAn)
	{
		Temp = OP1CON0;
		Temp &= ~(OPA_OP1CON0_OP1NS_Msk);
		Temp |= (NegativeSource << OPA_OP1CON0_OP1NS_Pos);
		OP1CON0 = Temp;
	}

}

/*****************************************************************************
 ** \brief	 OPA_EnableOutput
 **			 开启运放器输出端
 ** \param [in] OPAn : OPA0、OPA1
 ** \return  none
 ** \note   
 *****************************************************************************/
void OPA_EnableOutput(uint8_t OPAn)
{
	if(OPA0 == OPAn)
	{
		OP0CON0 |= (OPA_OP0CON0_OP0OS_Msk);
	}
	if(OPA1 == OPAn)
	{
		OP1CON0 |= (OPA_OP1CON0_OP1OS_Msk);	
	}
}

/*****************************************************************************
 ** \brief	 OPA_DisableOutput
 **			 关闭运放器输出端
 ** \param [in] OPAn : OPA0、OPA1
 ** \return  none
 ** \note   
 *****************************************************************************/
void OPA_DisableOutput(uint8_t OPAn)
{
	if(OPA0 == OPAn)
	{
		OP0CON0 &= ~(OPA_OP0CON0_OP0OS_Msk);
	}
	if(OPA1 == OPAn)
	{
		OP1CON0 &= ~(OPA_OP1CON0_OP1OS_Msk);
	}
}



/********************************************************************************
 ** \brief	 OPA_Start
 **			 开启OP
 ** \param [in] OPAn : OPA0、OPA1
 ** \return  none
 ** \note   
 ******************************************************************************/
void OPA_Start(uint8_t OPAn)
{
	if(OPA0 == OPAn)
	{
		OP0CON0 |= OPA_OP0CON0_OP0EN_Msk;
	}
	if(OPA1 == OPAn)
	{
		OP1CON0 |= OPA_OP1CON0_OP1EN_Msk;	
	}
}

/********************************************************************************
 ** \brief	 OPA_Stop
 **			 关闭OPA
 ** \param [in] OPAn : OPA0、OPA1 
 ** \return  none
 ** \note   
 ******************************************************************************/
void OPA_Stop(uint8_t OPAn)
{
	if(OPA0 == OPAn)
	{
		OP0CON0 &= ~OPA_OP0CON0_OP0EN_Msk;
	}
	if(OPA1 == OPAn)
	{
		OP1CON0 &= ~OPA_OP1CON0_OP1EN_Msk;	
	}	
}

/********************************************************************************
 ** \brief	 OPA_GetResult
 **			 获取结果
 ** \param [in] OPAn : OPA0、OPA1
 ** \return  0/1
 ** \note   
 ******************************************************************************/
uint8_t OPA_GetResult(uint8_t OPAn)
{
	if(OPA0 == OPAn)
	{
		return ((OP0CON1 & OPA_OP0CON1_OP0DOUT_Msk)? 1:0);
	}
	if(OPA1 == OPAn)
	{
		return ((OP1CON1 & OPA_OP1CON1_OP1DOUT_Msk)? 1:0);
	}	
	return 0;
}


/*****************************************************************************
 ** \brief	 OPA_ConfigOffsetAdj
 **			 配置OPA失调电压调节方式
 ** \param [in] OPAn : OPA0、OPA1		
 **				OffsetAdj:	(1)OPA_OFFSET_CONFIG
 **							(2)OPA_OFFSET_OPADJ
 **				AdjVlue : 失调电压调节值：0x00~0x1f
 ** \return  none
 ** \note   
*****************************************************************************/
void OPA_ConfigOffsetAdj(uint8_t OPAn,uint8_t OffsetAdj, uint8_t AdjVlue)
{
	uint8_t Temp=0;
	
	if(OPA0 == OPAn)
	{
		Temp = OP0CON1;
		Temp &= ~(OPA_OP0CON1_OP0ADJ_Msk);
		Temp |= 0x1f & AdjVlue;
		OP0CON1 = Temp;
		OP0ADJE = OffsetAdj;
	}
	if(OPA1 == OPAn)
	{
		Temp = OP1CON1;
		Temp &= ~(OPA_OP1CON1_OP1ADJ_Msk);
		Temp |= 0x1f & AdjVlue;
		OP1CON1 = Temp;
		OP1ADJE = OffsetAdj;
	}		
}
/*****************************************************************************
 ** \brief	 OPA_Delay
 **			 延时函数
 ** \param [in]none
 ** \return  none
 ** \note   
*****************************************************************************/
static void OPA_Delay(void)
{
	volatile uint8_t i;
	for(i=0;i<50;i++)
		_nop_();
}
/*****************************************************************************
 ** \brief	 OPA_GetOffsetAdjValue
 **			 获取OPA失调电压调节值
 ** \param [in] OPAn : OPA0、OPA1
 **				OpaMode:  (1)OPA_MODE_OPA
 **						  (2)OPA_MODE_ACMP	
 ** \return  5位失调电压修调值
 ** \note   
 *****************************************************************************/
uint8_t  OPA_GetOffsetAdjValue(uint8_t OPAn, uint8_t OpaMode)
{
	uint8_t temp, OPAOut, Adjvalue;
	Adjvalue =0;	
	if(OPA0 == OPAn)
	{
		/*(1)关闭OPA0*/		
		OPA_Stop(OPA0);
		OPA_SetRunMode(OPA0,OpaMode);
		/*(2)设置修调模式下的信号输入端*/
		OP0CON1 = 0x00;	/*选择OP0N端口为信号输入端*/	
		P31CFG = 0x01;	/*设置P31为OPA0_N*/
		
		OP0CON0 &= ~(0x3<<2);
		/*（3）开启OPA0调节模式*/
		OP0CON0 |= (1<<6);	
		/*(4)设置OPA的调节值源为OPA0ADJ<4:0>*/
		OP0ADJE = 0xAA;
		/*(5)开启OPA模块*/
		OPA_Start(OPA0);
		/*(6)获取调节值*/
		OPA_Delay();
		OPAOut = OPA_GetResult(OPA0);
		for(temp=0; temp<0x20; temp++)
		{
			OP0CON1 = temp;
			OPA_Delay();
			if(OPAOut != OPA_GetResult(OPA0))
			{
				Adjvalue = temp;
				break;
			}
		}
		/*(7)关闭OPA修调模式*/
		OP0CON0 &= ~(1<<6);
		OPA_Stop(OPA0);
		return 	Adjvalue;
	}	

	if(OPA1 == OPAn)
	{
		/*(1)关闭OPA0*/		
		OPA_Stop(OPA1);
		OPA_SetRunMode(OPA1,OpaMode);
		/*(2)设置修调模式下的信号输入端*/
		OP1CON1 = 0x00;	/*选择OPA1N端口为信号输入端*/			
		P23CFG = 0x01;/*设置P23为OPA1_N*/
		OP1CON0 &= ~(0x3<<2);
		/*（3）开启OPA1调节模式*/
		OP1CON0 |= (1<<6);	
		/*(4)设置OPA的调节值源为OPA1ADJ<4:0>*/
		OP1ADJE = 0xAA;
		/*(5)开启OPA模块*/
		OPA_Start(OPA1);
		/*(6)获取调节值*/
		OPA_Delay();
		OPAOut = OPA_GetResult(OPA1);
		for(temp=0; temp<0x20; temp++)
		{
			OP1CON1 = temp;
			OPA_Delay();
			if(OPAOut != OPA_GetResult(OPA1))
			{
				Adjvalue = temp;
				break;
			}
		}
		/*(7)关闭OPA修调模式*/
		OP1CON0 &= ~(1<<6);
		OPA_Stop(OPA1);
		
		return 	Adjvalue;
	}
	return 0;	
}
