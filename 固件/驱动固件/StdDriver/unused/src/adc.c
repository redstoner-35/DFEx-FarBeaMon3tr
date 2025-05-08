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
/** \file adc.c
**
**  
**
**	History:
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "adc.h"

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
 ** \brief	 ADC_Start			
 ** \param [in] none       
 ** \return  none
*****************************************************************************/
void  ADC_Start(void)
{
	ADCON1 |= ADC_ADCON1_ADEN_Msk;
}
/*****************************************************************************
 ** \brief	 ADC_Stop			
 ** \param [in] none         
 ** \return  none
*****************************************************************************/
void  ADC_Stop(void)
{
	ADCON1 &= ~(ADC_ADCON1_ADEN_Msk);
}
/*****************************************************************************
 ** \brief	 ADC_ConfigRunMode
 **			  配置ADC的运行模式
 ** \param [in] 
 **				ADCClkDiv: ADC_CLK_DIV_2 ~ ADC_CLK_DIV_256
 **          	ADCResultTpye: (1)ADC_RESULT_LEFT    左对齐
 **							   (2)ADC_RESULT_RIGHT   右对齐
 ** \return  none
*****************************************************************************/
void  ADC_ConfigRunMode( uint8_t ADCClkDiv, uint8_t ADCResultTpye)
{
	uint8_t Temp=0;
	
	Temp = ADCON0;
	Temp &= ~(ADC_ADCON0_ADFM_Msk);
	Temp |= ADCResultTpye;
	ADCON0 = Temp;
	
	Temp = ADCON1;
	Temp &= ~(ADC_ADCON1_ADCKS_Msk);
	Temp |= (ADCClkDiv<< ADC_ADCON1_ADCKS_Pos);
	ADCON1 = Temp;
}

/*****************************************************************************
 ** \brief	 ADC_EnableHardwareTrig
 **			使能硬件触发ADC
 ** \param [in] none
 ** \return  none
 ** \note 	 
*****************************************************************************/
void  ADC_EnableHardwareTrig(void)
{
	ADCON2 |= (ADC_ADCON2_ADCEX_Msk);
}
/*****************************************************************************
 ** \brief	 ADC_DisableHardwareTrig
 **			 关闭硬件触发ADC
 ** \param [in] none
 ** \return  none
 ** \note 	 
*****************************************************************************/
void  ADC_DisableHardwareTrig(void)
{
	ADCON2 &= ~(ADC_ADCON2_ADCEX_Msk);
}
/*****************************************************************************
 ** \brief	 ADC_ConfigHardwareTrig
 **			 配置硬件触发模式
 ** \param [in] ADCTGSource: ADC_TG_PWM0、ADC_TG_PWM2、ADC_TG_PWM4、ADC_TG_ADET
 **				TGMode：(1)ADC_TG_FALLING
 **						(2)ADC_TG_RISING
 **						(3)ADC_TG_PWM_PM
 **						(4)ADC_TG_PWM_PE
 ** \return  none
 ** \note 	 
*****************************************************************************/
void  ADC_ConfigHardwareTrig(uint8_t ADCTGSource, uint8_t TGMode)
{
	uint8_t Temp=0;
	
	Temp = ADCON2;
	Temp &= ~(ADC_ADCON2_ADTGS_Msk | ADC_ADCON2_ADEGS_Msk);
	Temp |= (ADCTGSource<< ADC_ADCON2_ADTGS_Pos) | (TGMode<<ADC_ADCON2_ADEGS_Pos);
	ADCON2 = Temp;
	
}

/*****************************************************************************
 ** \brief	 ADC_EnableChannel
 **			 使能ADC转换通道
 ** \param [in]  ADCChannel: ADC_CH_0 ..... ADC_CH_31
 ** \return  none
 ** \note 	
*****************************************************************************/
void  ADC_EnableChannel(uint8_t ADCChannel)
{
	uint8_t Temp=0;
	
	Temp = ADCON0;
	Temp &= ~(ADC_ADCON0_ADCHS4_Msk);
	Temp |= ((ADCChannel & 0x10)<<3);
	ADCON0 = Temp;
	
	Temp = ADCON1;
	Temp &= ~(ADC_ADCON1_ADCHS_Msk);
	Temp |= ADCChannel & 0xf;
	ADCON1 = Temp;
	
}

/*****************************************************************************
 ** \brief	 ADC_ConfigAN31
 **			 配置AN31通道
 ** \param [in]	An31Channel:（1）ADC_CH_31_BGR...ADC_CH_31_VDD
 ** \return  none
 ** \note 	
*****************************************************************************/
void  ADC_ConfigAN31(uint8_t An31Channel)
{
	uint8_t Temp=0;
	
	Temp = ADCON0;
	Temp &= ~(ADC_ADCON0_AN31SEL_Msk);
	Temp |= (An31Channel<<ADC_ADCON0_AN31SEL_Pos);
	ADCON0 = Temp;
	
}

/*****************************************************************************
 ** \brief	 ADC_SetTrigDelayTime
 **			 配置硬件触发模式的延时时间
 ** \param [in]  TrigTime: 0~1023
 ** \return  none
 ** \note 	(TrigTime + 4)) *Tsys
*****************************************************************************/
void  ADC_SetTrigDelayTime(uint16_t TrigTime)
{
	uint8_t Temp=0;
	
	ADDLYL =  TrigTime;
	Temp = ADCMPC;
	Temp &= ~(ADC_ADCMPC_ADDLY_Msk);
	Temp |= ADC_ADCMPC_ADDLY_Msk & (TrigTime >> 8);
	ADCMPC = Temp;
	
}

/*****************************************************************************
 ** \brief	 ADC_ConfigADCBrake
 **			 配置ADC刹车模式 
 ** \param [in]  ADCBreak: (1)ADC_BRK_LESS_THAN_ADCMP 	: ADRES < ADCMP 触发刹车
 **						   (2)ADC_BRK_NOT_LESS_THAN_ADCMP : ADRES >= ADCMP 触发刹车 
 ** \return  none
 ** \note 
*****************************************************************************/
void  ADC_ConfigADCBrake(uint8_t ADCBrake)
{
	uint8_t Temp=0;
	
	Temp = ADCMPC;
	Temp &= ~(ADC_ADCMPC_ADCMPPS_Msk);
	Temp |=  ADCBrake;
	ADCMPC = Temp;
	
}

/*****************************************************************************
 ** \brief	 ADC_ConfigCompareValue
 **			 配置ADC比较值
 ** \param [in]  ADCCompareValue: ADCMPH,ADCMPL
 ** \return  none
 ** \note 
*****************************************************************************/
void ADC_ConfigCompareValue(uint16_t ADCCompareValue)
{
	ADCMPL = ADCCompareValue & 0xF;
	ADCMPH = ADCCompareValue>>4; 
}

/*****************************************************************************
 ** \brief	 ADC_GetCmpResult
 **			 获取ADC比较结果
 ** \param [in]  none
 ** \return   0/1
 ** \note 
*****************************************************************************/
uint8_t  ADC_GetCmpResult(void)
{
	return ((ADCMPC & ADC_ADCMPC_ADCMPO_Msk) ? 1:0);
}

/*****************************************************************************
 ** \brief	 ADC_GetADCResult
 **			 获取ADC转换结果
 ** \param [in]  none
 ** \return  16bits ADC值
 ** \note 
*****************************************************************************/
uint16_t  ADC_GetADCResult(void)
{
	if(!(ADCON0 & ADC_ADCON0_ADFM_Msk))		/*左对齐*/
	{
		return (0xfff & (((ADRESH<<4) | (ADRESL>>4))));
	}
	return (0xfff & ((ADRESH<<8)| ADRESL));	
}

/*****************************************************************************
 ** \brief	 ADC_EnableInt
 **			 开启ADC转换完成中断
 ** \param [in]  none
 ** \return  none
 ** \note 
*****************************************************************************/
void ADC_EnableInt(void)
{
	EIE2 |= IRQ_EIE2_ADCIE_Msk;
}
/*****************************************************************************
 ** \brief	 ADC_DisableInt
 **			 关闭ADC转换完成中断
 ** \param [in]  none
 ** \return  none
 ** \note 
*****************************************************************************/
void ADC_DisableInt(void)
{
	EIE2 &= ~(IRQ_EIE2_ADCIE_Msk);
}
/*****************************************************************************
 ** \brief	 ADC_GetIntFlag
 **			 获取ADC转换完成中断标志位
 ** \param [in]  none
 ** \return  0：无中断，1：产生中断
 ** \note 
*****************************************************************************/
uint8_t ADC_GetIntFlag(void)
{
	return ((EIF2 & IRQ_EIF2_ADCIF_Msk) ? 1:0);
}
/*****************************************************************************
 ** \brief	 ADC_ClearIntFlag
 **			 清ADC转换完成中断标志位
 ** \param [in] none
 ** \return  none
 ** \note   
*****************************************************************************/
void ADC_ClearIntFlag(void)
{
	EIF2 = 0xFF & (~(IRQ_EIF2_ADCIF_Msk));
}

/*****************************************************************************
 ** \brief	 ADC_EnableLDO
 **			使能ADC的LDO
 ** \param [in] none
 ** \return  none
 ** \note   
*****************************************************************************/
void ADC_EnableLDO(void)
{
	ADCLDO |= ADC_ADCLDO_LDOEN_Msk;
}
/*****************************************************************************
 ** \brief	 ADC_DisableLDO
 **			关闭ADC的LDO
 ** \param [in] none
 ** \return  none
 ** \note   
*****************************************************************************/
void ADC_DisableLDO(void)
{
	ADCLDO &= ~(ADC_ADCLDO_LDOEN_Msk);
}
/*****************************************************************************
 ** \brief	 ADC_ConfigADCVref
 **			 配置ADC的VREF
 ** \param [in] ADCVref :(1)ADC_VREF_1P2V :	Vref =1.2v
 **						 (2)ADC_VREF_2V	  : Vref =2v
 **						 (2)ADC_VREF_2P4V : Vref =2.4v
 **						 (2)ADC_VREF_3V	  : Vref =3v
 ** \return  none
 ** \note   
*****************************************************************************/
void ADC_ConfigADCVref(uint8_t ADCVref)
{
	uint8_t Temp=0;
	
	Temp = ADCLDO;
	Temp &= ~(ADC_ADCLDO_VSEL_Msk);
	Temp |= ADCVref;
	ADCLDO = Temp;
	
}

