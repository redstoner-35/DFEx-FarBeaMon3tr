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
/** \file uart.c
**
**  
**
**	History:
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "uart.h"
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
/******************************************************************************
 ** \brief UART_ConfigRunMode
 **			配置UART运行模式
 ** \param [in]     UARTn: 			(1)UART0
 **									(2)UART1
 **					UARTMode:		(1)UART_MOD_SYN : 同步模式
 **									(2)UART_MOD_ASY_8BIT: 8位异步模式
 **									(3)UART_MOD_ASY_9BIT：9位异步模式
 **									(4)UART_MOD_ASY_9BIT_STATIC：9位波特率固定异步模式
 **					UARTBaudTimer:  (1)UART_BAUD_TMR1 :	Timer1产生波特率
 **									(2)UART_BAUD_TMR4 :	Timer4产生波特率
 **									(3)UART_BAUD_TMR2 : Timer2产生波特率
 **									(4)UART_BAUD_BRT : BRT产生波特率
 ** \return none
******************************************************************************/
void UART_ConfigRunMode(uint8_t UARTn, uint8_t UARTMode, uint8_t UARTBaudTimer)
{
	uint8_t Temp=0; 
	
	if(UARTn == UART0)
	{
		Temp = SCON0;
		Temp &= ~(UART_SCON0_U0SMn_Msk);
		Temp |= (UARTMode<< UART_SCON0_U0SMn_Pos);
		SCON0 = Temp;

		FUNCCR &= ~(SYS_FUNCCR_UART0_CKS1_Msk | SYS_FUNCCR_UART0_CKS0_Msk);		
		switch(UARTBaudTimer)
		{
			case  UART_BAUD_TMR1:		
				break;
			case  UART_BAUD_TMR4:
				FUNCCR |= SYS_FUNCCR_UART0_CKS0_Msk;
				break;
			case  UART_BAUD_TMR2:
				FUNCCR |= SYS_FUNCCR_UART0_CKS1_Msk;
				break;		
			case  UART_BAUD_BRT:
				FUNCCR |= SYS_FUNCCR_UART0_CKS1_Msk | SYS_FUNCCR_UART0_CKS0_Msk;
				break;	
			default:
				break;
		}			
		
	}
	if(UARTn == UART1)
	{
		Temp = SCON1;
		Temp &= ~(UART_SCON1_U1SMn_Msk);
		Temp |= (UARTMode<< UART_SCON1_U1SMn_Pos);
		SCON1 = Temp;
		
		FUNCCR &= ~(SYS_FUNCCR_UART1_CKS1_Msk | SYS_FUNCCR_UART1_CKS0_Msk);		
		switch(UARTBaudTimer)
		{
			case  UART_BAUD_TMR1:		
				break;
			case  UART_BAUD_TMR4:
				FUNCCR |= SYS_FUNCCR_UART1_CKS0_Msk;
				break;
			case  UART_BAUD_TMR2:
				FUNCCR |= SYS_FUNCCR_UART1_CKS1_Msk;
				break;		
			case  UART_BAUD_BRT:
				FUNCCR |= SYS_FUNCCR_UART1_CKS1_Msk | SYS_FUNCCR_UART1_CKS0_Msk;
				break;	
			default:
				break;
		}				
	}		
}
/******************************************************************************
 ** \brief UART_EnableDoubleFrequency
 **			使能UART波特率倍频模式
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return none
******************************************************************************/
void UART_EnableDoubleFrequency(uint8_t UARTn)
{
	if(UARTn == UART0)
	{
		PCON |= SYS_PCON_SMOD0_Msk;
	}
	if(UARTn == UART1)
	{
		PCON |= SYS_PCON_SMOD1_Msk;
	}	
}
/******************************************************************************
 ** \brief UART_DisableDoubleFrequency
 **			关闭UART波特率倍频模式
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return none
******************************************************************************/
void UART_DisableDoubleFrequency(uint8_t UARTn)
{
	if(UARTn == UART0)
	{
		PCON &= ~(SYS_PCON_SMOD0_Msk);
	}
	if(UARTn == UART1)
	{
		PCON &= ~(SYS_PCON_SMOD1_Msk);
	}
}

/******************************************************************************
 ** \brief UART_ConfigBaudRate
 **			配置UART波特率
 ** \param [in]    UARTn:  (1)UART0 : UART0
 **									(2)UART1 : UART1
 **					BaudRateValue:  波特率值
 ** \return Timer1/4: 8位自动重装模式的THx值
 **			Timer2: 16 位自动重装模式的 RLDH:RLDL 值
 **			BRT: 16 位定时模式 BRTH:BRTL 值
******************************************************************************/
uint16_t UART_ConfigBaudRate(uint8_t UARTn, uint32_t  BaudRateValue)
{
#ifdef  USE_FORMULA
	float  BaudRateTemp =0;	
	float  BaudRate = 0.55;
	uint16_t PeriodValue =0;
	uint8_t  SMOD_Flag =0;
	uint8_t  T_Flag =0;	
	uint8_t  BaudRateTimer =0;
	
	SMOD_Flag =1;
	if(UARTn == UART0)
	{	
		if(PCON & SYS_PCON_SMOD0_Msk)
		{
			SMOD_Flag = 2;						/*获取SMOD位*/
		}	
		BaudRateTimer = FUNCCR & (SYS_FUNCCR_UART0_CKS1_Msk | SYS_FUNCCR_UART0_CKS0_Msk);
		switch(BaudRateTimer)
		{
			case 0x00:
				BaudRateTimer = UART_BAUD_TMR1;
				break;
			case 0x01:
				BaudRateTimer = UART_BAUD_TMR4;
				break;			
			case 0x04:
				BaudRateTimer = UART_BAUD_TMR2;
				break;			
			case 0x05:
				BaudRateTimer = UART_BAUD_BRT;
				break;	
			default:
				BaudRateTimer = 0;
				break;
		}				
	}
	if(UARTn == UART1)
	{	
		if(PCON & SYS_PCON_SMOD1_Msk)
		{
			SMOD_Flag = 2;						/*获取SMOD位*/
		}	
		BaudRateTimer = FUNCCR & (SYS_FUNCCR_UART1_CKS1_Msk | SYS_FUNCCR_UART1_CKS0_Msk);
		switch(BaudRateTimer)
		{
			case 0x00:
				BaudRateTimer = UART_BAUD_TMR1;
				break;
			case 0x02:
				BaudRateTimer = UART_BAUD_TMR4;
				break;			
			case 0x08:
				BaudRateTimer = UART_BAUD_TMR2;
				break;			
			case 0x0A:
				BaudRateTimer = UART_BAUD_BRT;
				break;	
			default:
				BaudRateTimer = 0;
				break;
		}	
	}
	switch(BaudRateTimer)
	{
		case UART_BAUD_TMR1:	// Timer1
			T_Flag = ((CKCON & TMR_CKCON_T1M_Msk)? 1:3);		/*获取T1M位*/
			BaudRateTemp = 256 - SystemClock *SMOD_Flag/ 32 / 4 /T_Flag/BaudRateValue;					
			break;
		case UART_BAUD_TMR4:	// Timer4
			T_Flag = ((T34MOD & TMR_T34MOD_T4M_Msk)? 1:3);		/*获取T4M位*/
			BaudRateTemp = 256 - SystemClock *SMOD_Flag/ 32 / 4 /T_Flag/BaudRateValue;			
			break;
		case UART_BAUD_TMR2:	// Timer2
			T_Flag = (( T2CON & TMR_T2CON_T2PS_Msk)? 2:1);			/*获取T2PS位*/
			BaudRateTemp = 65536 - SystemClock *SMOD_Flag/ 32 / 12 /T_Flag/BaudRateValue; 		
			break;			
		case UART_BAUD_BRT:	// BRT
			T_Flag = BRTCON & UART_BRTCON_BRTCKDIV_Msk;		/*获取BRTCLKVIV位*/
			BaudRateTemp = 65536 - SystemClock *SMOD_Flag/ 32 / (1<< T_Flag)/BaudRateValue;	
			break;							
		default:
			break;
	}				
	if((BaudRateTemp - (uint16_t) BaudRateTemp) > BaudRate)
	{
		PeriodValue = (uint16_t)BaudRateTemp + 1;
	}
	else
	{
		PeriodValue = BaudRateTemp;
	}	
	return (PeriodValue);
#else	
	UARTn =0;
	BaudRateValue =0;
	return 0;
#endif 
}

/******************************************************************************
 ** \brief UART_EnableReceive
 **			开启接收模式
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 
******************************************************************************/
void UART_EnableReceive(uint8_t UARTn)
{
	if(UARTn == UART0)
	{
		SCON0 |= UART_SCON0_U0REN_Msk;
	}
	if(UARTn == UART1)
	{
		SCON1 |= UART_SCON1_U1REN_Msk;
	}
}
/******************************************************************************
 ** \brief UART_DisableReceive
 **			关闭接收模式
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 
******************************************************************************/
void UART_DisableReceive(uint8_t UARTn)
{
	if(UARTn == UART0)
	{
		SCON0 &= ~(UART_SCON0_U0REN_Msk);
	}
	if(UARTn == UART1)
	{
		SCON1 &= ~(UART_SCON1_U1REN_Msk);
	}
}

/******************************************************************************
 ** \brief UART_EnableInt
 **			开启中断
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 
******************************************************************************/
void UART_EnableInt(uint8_t UARTn)
{
	if(UARTn == UART0)
	{
		ES0 =1;
	}
	if(UARTn == UART1)
	{
		ES1 =1;
	}	
}
/******************************************************************************
 ** \brief UART_DisableInt
 **			关闭中断
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 
******************************************************************************/
void UART_DisableInt(uint8_t UARTn)
{
	if(UARTn == UART0)
	{
		ES0 =0;
	}
	if(UARTn == UART1)
	{
		ES1 =0;
	}
}
/******************************************************************************
 ** \brief UART_GetReceiveIntFlag
 **			获取接收中断标志位
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 0：无中断，1：有中断
******************************************************************************/
uint8_t  UART_GetReceiveIntFlag(uint8_t UARTn)
{
	if(UART0 ==  UARTn)
	{
		return((SCON0 & UART_SCON0_RI0_Msk)? 1:0);
	}
	if(UART1 == UARTn)
	{
		return((SCON1 & UART_SCON1_RI1_Msk)? 1:0);
	}
	return 0;
}

/******************************************************************************
 ** \brief UART_ClearReceiveIntFlag
 **			清除接收中断标志位
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 
******************************************************************************/
void UART_ClearReceiveIntFlag(uint8_t UARTn)
{
	uint8_t temp;
	
	if(UART0 ==  UARTn)
	{				
		temp  = SCON0;
		temp &= ~(UART_SCON0_RI0_Msk);
		SCON0 = temp| UART_SCON0_TI0_Msk;		
	}
	if(UART1 == UARTn)
	{
		temp  = SCON1;
		temp &= ~(UART_SCON1_RI1_Msk);
		SCON1 = temp | UART_SCON1_TI1_Msk;				
	}	
}

/******************************************************************************
 ** \brief UART_GetSendIntFlag
 **			获取发送中断标志位
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 0：无中断，1：有中断
******************************************************************************/
uint8_t  UART_GetSendIntFlag(uint8_t UARTn)
{
	if(UART0 ==  UARTn)
	{
		return((SCON0 & UART_SCON0_TI0_Msk)? 1:0);
	}
	if(UART1 ==  UARTn)
	{
		return((SCON1 & UART_SCON1_TI1_Msk)? 1:0);
	}
	return 0;
}

/******************************************************************************
 ** \brief UART_ClearSendIntFlag
 **			清除发送中断标志位
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 
******************************************************************************/
void UART_ClearSendIntFlag(uint8_t UARTn)
{
	uint8_t temp;
	
	if(UART0 ==  UARTn)
	{
		temp  = SCON0;
		temp &= ~(UART_SCON0_TI0_Msk);
		SCON0 = temp | UART_SCON0_RI0_Msk;	
	}
	if(UART1 == UARTn)
	{
		temp  = SCON1;
		temp &= ~(UART_SCON1_TI1_Msk);
		SCON1 = temp | UART_SCON1_RI1_Msk;			
	}	
}

/******************************************************************************
 ** \brief UART_GetBuff
 **			获取接收Buff的值
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 
******************************************************************************/
uint8_t UART_GetBuff(uint8_t UARTn)
{
	if(UART0 == UARTn)
	{
		return(SBUF0);
	}
	if(UART1 ==  UARTn)
	{
		return(SBUF1);
	}
	return 0;
}

/******************************************************************************
 ** \brief UART_SendBuff
 **			发送数据到发送Buff
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 **				   UARTSendValue : 发送的8位值
 ** \return 
******************************************************************************/
void UART_SendBuff(uint8_t UARTn, uint8_t UARTSendValue)
{
	if(UART0 == UARTn)
	{
		SBUF0 = UARTSendValue;
	}
	if(UART1 == UARTn)
	{
		SBUF1 = UARTSendValue;
	}
}

/******************************************************************************
 ** \brief UART_SendNinthBit
 **			发送数据的第九位
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 **				   UARTSendValue : 发送的第九位值
 ** \return 
******************************************************************************/
void UART_SendNinthBit(uint8_t UARTn, uint8_t UARTSendValue)
{
	if(UART0 == UARTn)
	{
		if(0 == UARTSendValue)
		{
			SCON0 &= ~(UART_SCON0_U0TB8_Msk);
		}
		else
		{
			SCON0 |= UART_SCON0_U0TB8_Msk;	
		}
	}
	if(UART1 == UARTn)
	{
		if(0 == UARTSendValue)
		{
			SCON1 &= ~(UART_SCON1_U1TB8_Msk);
		}
		else 
		{
			SCON1 |= UART_SCON1_U1TB8_Msk;	
		}
	}
}
/******************************************************************************
 ** \brief UART_GetNinthBit
 **			获取数据的第九位
 ** \param [in]    UARTn: (1) UART0
 **						  (2) UART1
 ** \return 0/1
******************************************************************************/
uint8_t UART_GetNinthBit(uint8_t UARTn)
{
	if(UART0 == UARTn)
	{
		return ((SCON0 & UART_SCON0_U0RB8_Msk)? 1:0);
	}
	else
	{
		return ((SCON1 & UART_SCON1_U1RB8_Msk)? 1:0);
	}
}

/******************************************************************************
 ** \brief UART_EnableBRT
 **		   使能BRT时钟模块
 ** \param [in]none 
 ** \return none
******************************************************************************/
void UART_EnableBRT(void)
{
	BRTCON |= UART_BRTCON_BRTEN_Msk;
}
/******************************************************************************
 ** \brief UART_ConfigBRTClk
 **		   配置BRT模块时钟
 ** \param [in] BRTClkDiv : BRT_CLK_DIV_1 ~ BRT_CLK_DIV_128
 ** \return none
******************************************************************************/
void UART_ConfigBRTClk(uint8_t BRTClkDiv)
{
	uint8_t Temp=0;
	
	Temp = BRTCON;
	Temp &= ~(UART_BRTCON_BRTCKDIV_Msk);
	Temp |= BRTClkDiv;
	BRTCON = Temp;
}

/******************************************************************************
 ** \brief UART_ConfigBRTPeriod
 **		   配置BRT模块时钟周期
 ** \param [in] BRTPeriod: 16bit
 ** \return none
******************************************************************************/
void UART_ConfigBRTPeriod(uint16_t BRTPeriod)
{
	BRTDL = BRTPeriod;
	BRTDH = BRTPeriod>>8;
}
/******************************************************************************
 ** \brief UART_DisableBRT
 **		   关闭BRT时钟模块
 ** \param [in] none
 ** \return none
******************************************************************************/
void UART_DisableBRT(void)
{
	BRTCON &= ~(UART_BRTCON_BRTEN_Msk);
}







