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
/** \file timer.h
**
** 
**
** History:
** 
******************************************************************************/
#ifndef __TIMER_H__
#define __TIMER_H__
/*****************************************************************************/
/* Include files */
/*****************************************************************************/
#include "cms8s6990.h"
/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define') */
/*****************************************************************************/
/*----------------------------------------------------------------------------
 **Timer 模块
---------------------------------------------------------------------------*/
#define		TMR0					(0x00)			/* Timer 0 Number*/	
#define		TMR1					(0x01)			/* Timer 1 Number*/	
#define 	TMR2					(0x02)			/* Timer 2 Number*/	
#define		TMR3					(0x03)			/* Timer 3 Number*/	
#define 	TMR4					(0x04)			/* Timer 4 Number*/	

/*----------------------------------------------------------------------------
 **Timer 运行模式
---------------------------------------------------------------------------*/
#define		TMR_MODE_TIMING		 	(0x00)			/*定时器定时模式(Timer 0,1,3,4)*/
#define		TMR_MODE_COUNT			(0x01)			/*定时器计数模式(Timer 0,1)*/
	
/*----------------------------------------------------------------------------
 **Timer 时钟分频
---------------------------------------------------------------------------*/	
#define 	TMR_CLK_DIV_12			(0x00)			/*系统时钟12分频(Timer 0,1,3,4)*/
#define 	TMR_CLK_DIV_4			(0x01)			/*系统时钟4分频(Timer 0,1,3,4)*/
/*----------------------------------------------------------------------------
 **Timer 定时/计数模式
---------------------------------------------------------------------------*/	
#define 	TMR_TIM_13BIT					(0x00)		/*定时/计数13位模式*/
#define 	TMR_TIM_16BIT					(0x01)		/*定时/计数16位模式*/
#define 	TMR_TIM_AUTO_8BIT				(0x02)		/*定时/计数8位自动重载模式*/
#define 	TMR_TIM_TWO_INDEPENDENT_8BIT	(0x03)		/*定时/计数2个独立8位模式*/

/*----------------------------------------------------------------------------
 ****************************************************************************
---------------------------------------------------------------------------*/	
/*----------------------------------------------------------------------------
 **Timer2 定时/计数/比较/捕获模式
---------------------------------------------------------------------------*/	
#define		TMR2_MODE_TIMING		(0x01)			/*定时器2定时模式*/
#define		TMR2_MODE_COUNT			(0x02)			/*定时器2计数模式*/
#define 	TMR2_MODE_COMPARE		(0x02)			/*定时器2比较模式*/
/*----------------------------------------------------------------------------
 **Timer2 时钟分频
---------------------------------------------------------------------------*/	
#define 	TMR2_CLK_DIV_12			(0x00)						/*系统时钟12分频*/
#define 	TMR2_CLK_DIV_24			(0x01<<TMR_T2CON_T2PS_Pos)	/*系统时钟24分频*/
/*----------------------------------------------------------------------------
 **Timer2 比较/捕获输入通道
---------------------------------------------------------------------------*/	
#define 	TMR2_CC0				(0x00)			/*Timer2 Compare or Capture Channel 0 number*/
#define 	TMR2_CC1				(0x01)			/*Timer2 Compare or Capture Channel 1 number*/
#define 	TMR2_CC2				(0x02)			/*Timer2 Compare or Capture Channel 2 number*/
#define 	TMR2_CC3				(0x03)			/*Timer2 Compare or Capture Channel 3 number*/


/*----------------------------------------------------------------------------
 **Timer2 捕获模块 捕获产生模式
---------------------------------------------------------------------------*/	
#define 	TMR2_CAP_WRITE_REGISTER	(0x03)						/*捕获通道在写寄存器时捕获*/
#define		TMR2_CAP_EDGE_FALLING	(0x00)						/*捕获通道下降沿捕获*/
#define		TMR2_CAP_EDGE_RISING	(0x01)						/*捕获通道上升沿捕获*/
#define		TMR2_CAP_EDGE_BOTH		(0x03)						/*捕获通道在上升和下降边沿都可捕获*/	
/*----------------------------------------------------------------------------
 **Timer2 重载模式
---------------------------------------------------------------------------*/	
#define 	TMR2_AUTO_LOAD			(0x02<< TMR_T2CON_T2Rn_Pos)	/*Timer2自动重载模式*/
#define		TMR2_T2EX_LOAD			(0x03<< TMR_T2CON_T2Rn_Pos)	/*Timer2在T2EX引脚下降沿重载模式*/
#define		TMR2_LOAD_DISBALE		(0x00<< TMR_T2CON_T2Rn_Pos)	/*禁止重载*/
/*----------------------------------------------------------------------------
 **Timer2 比较模式
---------------------------------------------------------------------------*/	
#define		TMR2_CMP_MODE_0			(0x00)						/*Timer2比较模式0*/
#define		TMR2_CMP_MODE_1			(0x01<< TMR_T2CON_T2CM_Pos)	/*Timer2比较模式1*/
/*----------------------------------------------------------------------------
 **Timer2 比较模块0 中断产生模式
---------------------------------------------------------------------------*/	
#define		TMR2_CMP_INT_MODE1		(0x01<< TMR_T2CON_I3FR_Pos)	/*比较通道0在TL2/TH2与RLDL/RLDH从不相等到相等时刻产生中断*/	
#define		TMR2_CMP_INT_MODE0		(0x00)						/*比较通道0在TL2/TH2与RLDL/RLDH从相等到不相等时刻产生中断*/	


/***************************************************************************
 ** \brief	TMR0_SET_PERIOD
 **			配置定时器定时周期
 ** \param [in] TimerPeriodH:(1)TH0 
 **				TimerPeriodL:(1)TL0 
 ** \return  none
 ** \note    13位定时器 ：THx[7:0],TLx[4:0]
*****************************************************************************/
#define		TMR0_SET_PERIOD(TimerPeriodH,TimerPeriodL)	do{\
															TH0 = (uint8_t)TimerPeriodH;\
															TL0 = (uint8_t)TimerPeriodL;\					
														}while(0)
/***************************************************************************
 ** \brief	TMR1_SET_PERIOD
 **			配置定时器定时周期
 ** \param [in] TimerPeriodH:(1)TH1 
 **				TimerPeriodL:(1)TL1 
 ** \return  none
 ** \note    13位定时器 ：THx[7:0],TLx[4:0]
*****************************************************************************/
#define		TMR1_SET_PERIOD(TimerPeriodH,TimerPeriodL)	do{\
															TH1 = (uint8_t)TimerPeriodH;\
															TL1 = (uint8_t)TimerPeriodL;\					
															}while(0)
/***************************************************************************
 ** \brief	TMR3_SET_PERIOD
 **			配置定时器定时周期
 ** \param [in] TimerPeriodH:(1)TH3 
 **				TimerPeriodL:(1)TL3 
 ** \return  none
 ** \note    13位定时器 ：THx[7:0],TLx[4:0]
*****************************************************************************/
#define		TMR3_SET_PERIOD(TimerPeriodH,TimerPeriodL)	do{\
															TH3 = (uint8_t)TimerPeriodH;\
															TL3 = (uint8_t)TimerPeriodL;\					
															}while(0)
															
/***************************************************************************
 ** \brief	TMR4_SET_PERIOD
 **			配置定时器定时周期
 ** \param [in] TimerPeriodH:(1)TH4 
 **				TimerPeriodL:(1)TL4 
 ** \return  none
 ** \note    13位定时器 ：THx[7:0],TLx[4:0]
*****************************************************************************/
#define		TMR4_SET_PERIOD(TimerPeriodH,TimerPeriodL)	do{\
															TH4 =(uint8_t)TimerPeriodH;\
															TL4 =(uint8_t)TimerPeriodL;\					
															}while(0)															
																													
/***************************************************************************
 ** \brief	TMR2_SET_PERIOD
 **			配置定时器定时周期
 ** \param [in]  TimerPeriod:16bit
 ** \return  none
 ** \note    
*****************************************************************************/
#define		TMR2_SET_PERIOD(TimerPeriod)	do{\
													TH2 = (uint8_t)(TimerPeriod >> 8);\
													TL2 = (uint8_t)(TimerPeriod);\														
												}while(0)

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
 ** \brief	TMR_ConfigRunMode
 **			配置定时器运行模式
 ** \param [in] Timern	：		TMR0,TMR1,TMR3,TMR4
 **				TimerMode 	：(1)TMR_MODE_TIMING	:定时模式
 **							  (2)TMR_MODE_COUNT		:计数模式
 **				TimerModeBranch	：模式的分支
 **								(1)TMR_TIM_13BIT 	:13位定时/计数模式
 **								(2)TMR_TIM_16BIT 	:16位定时/计数模式		
 **								(3)TMR_TIM_AUTO_8BIT :8位自动重载定时/计数模式
 **								(4)TMR_TIM_TWO_INDEPENDENT_8BIT	:两个独立的8位定时/计数模式
 ** \return  none
 ** \note    此函数不支持TMR2 
 *****************************************************************************/
void TMR_ConfigRunMode(uint8_t Timern, uint8_t TimerMode, uint8_t TimerModeBranch);
/*****************************************************************************
 ** \brief	TMR_ConfigTimerClk
 **			配置定时器运行时钟
 ** \param [in]  Timern	：		TMR0,TMR1,TMR3,TMR4
 **				TimerClkDiv	：(1)TMR_CLK_DIV_4 
 **							  (2)TMR_CLK_DIV_12
 ** \note   
 *****************************************************************************/
void TMR_ConfigTimerClk(uint8_t Timern,uint8_t TimerClkDiv);
/***************************************************************************
 ** \brief	TMR_ConfigTimerPeriod
 **			配置定时器定时周期
 ** \param [in] Timern	：		TMR0,TMR1,TMR3,TMR4
 **				TimerPeriodH:(1)TH0 (2)TH1 (3)TH3 (4)TH4
 **				TimerPeriodL:(1)TL0 (2)TL1 (3)TL3 (4)TL4
 ** \return  none
 ** \note   13位定时器 ：THx[7:0],TLx[4:0]
*****************************************************************************/
void TMR_ConfigTimerPeriod(uint8_t Timern , uint8_t TimerPeriodHigh,uint8_t TimerPeriodLow);
/*****************************************************************************
 ** \brief	TMR_EnableGATE
 **			使能定时器门控
 ** \param [in] Timern	：	TMR0,TMR1
 ** \return  none
 ** \note   
 *****************************************************************************/
void TMR_EnableGATE(uint8_t Timern);
/*****************************************************************************
 ** \brief	TMR_DisableGATE
 **			关闭定时器门控
 ** \param [in] Timern	：	TMR0,TMR1
 ** \return  none
 ** \note   
 *****************************************************************************/
void TMR_DisableGATE(uint8_t Timern);

/*****************************************************************************
 ** \brief	TMR_GetCountValue
 **			获取定时器计数模式的计数值
 ** \param [in] Timern	：	TMR0,TMR1
 ** \return  16bits的计数值：[THn:TLn]
 ** \note   模式2/3: 请酌情使用
 *****************************************************************************/
uint16_t TMR_GetCountValue(uint8_t Timern);

/*****************************************************************************
 ** \brief	TMR_EnableOverflowInt
 **			使能定时器溢出中断
 ** \param [in] Timern	：		TMR0,TMR1,TMR3,TMR4
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR_EnableOverflowInt(uint8_t TimerMask);

/*****************************************************************************
 ** \brief	TMR_DisableOverflowInt
 **			关闭定时器溢出中断
 ** \param [in] Timern	：		TMR0,TMR1,TMR3,TMR4
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR_DisableOverflowInt(uint8_t TimerMask);
/*****************************************************************************
 ** \brief	TMR_GetOverflowIntFlag
 **			获取定时器中断标志
 ** \param [in] Timern	：		TMR0,TMR1,TMR3,TMR4
 ** \return   0：无中断， 1：有中断
 ** \note   定时器0、3使用模式3时，其中TLn的溢出中断使用TMR0、3中断标志
 **									   THn的溢出中断使用TMR1、4中断标志 
 *****************************************************************************/
uint8_t TMR_GetOverflowIntFlag(uint8_t TimerNumber);
/*****************************************************************************
 ** \brief	TMR_ClearOverflowIntFlag
 **			清除定时器溢出中断标志
 ** \param [in] Timern	：		TMR0,TMR1,TMR3,TMR4
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR_ClearOverflowIntFlag(uint8_t TimerNumber);
/*****************************************************************************
 ** \brief	TMR_Start
 **			开启定时器
 ** \param [in] Timern	：		TMR0,TMR1,TMR3,TMR4
 ** \return  none
 ** \note   
 *****************************************************************************/
void TMR_Start(uint8_t Timern);
/*****************************************************************************
 ** \brief	TMR_Stop
 **			关闭定时器
 ** \param [in] Timern	：		TMR0,TMR1,TMR3,TMR4
 ** \return  none
 ** \note   
 *****************************************************************************/
void TMR_Stop(uint8_t Timern);


/*****************************************************************************
 ** \brief	TMR2_ConfigRunMode
 **			配置Timer2运行模式
 ** \param [in]	Timer2Mode 	:	(1)TMR2_MODE_TIMING	:定时模式
 **								(2)TMR2_MODE_COUNT	:计数模式
 **			   Timer2LoadMode	：	
 **								(1)TMR2_AUTO_LOAD	:定时器2自动重载模式
 **								(2)TMR2_T2EX_LOAD	:定时器2外部触发重载模式
**								(3)TMR2_LOAD_DISBALE: 禁止重载
 ** \return  none
 ** \note    
*****************************************************************************/
void TMR2_ConfigRunMode(  uint8_t Timer2Mode, uint8_t Timer2LoadMode);
/*****************************************************************************
 ** \brief	TMR2_ConfigTimerClk
 **			配置定时器运行时钟
 ** \param [in]	TimerClkDiv	：TMR2_CLK_DIV_12 、TMR2_CLK_DIV_24
 ** \return  none
 ** \note   
 *****************************************************************************/
void TMR2_ConfigTimerClk(uint8_t TimerClkDiv);

/***************************************************************************
 ** \brief	TMR2_ConfigTimerPeriod
 **			配置定时器2定时周期
 ** \param [in] TimerPeriod；定时器2定时周期
 ** \return  none
 ** \note   
*****************************************************************************/
void TMR2_ConfigTimerPeriod(uint16_t TimerPeriod);
/*****************************************************************************
 ** \brief	TMR2_EnableGATE
 **			使能定时器2门控功能
 ** \param [in] none
 ** \return  none
 ** \note   
 *****************************************************************************/
void TMR2_EnableGATE(void);
/*****************************************************************************
 ** \brief	TMR2_DisableGATE
 **			关闭定时器2门控功能
 ** \param [in] none
 ** \return  none
 ** \note  
 *****************************************************************************/
void TMR2_DisableGATE(void);
/*****************************************************************************
 ** \brief	TMR2_EnableCompare
 **			使能比较模式
 ** \param [in]	Timer2CCn:  TMR2_CC0 ~ TMR2_CC3
 **				CompareMode :(1)TMR2_CMP_MODE_0
 **							 (2)TMR2_CMP_MODE_1
 ** \return  none
 ** \note   
 *****************************************************************************/
void TMR2_EnableCompare(uint8_t Timer2CCn, uint8_t CompareMode);
/*****************************************************************************
 ** \brief	TMR2_DisableCompare
 **			关闭比较模式
 ** \param [in]	Timer2CCMask:  TMR2_CC0 ~ TMR2_CC3
 ** \return  none
 ** \note   
 *****************************************************************************/
void TMR2_DisableCompare(uint8_t Timer2CCn);
/*****************************************************************************
 ** \brief	TMR2_ConfigCompareValue
 **			配置比较通道比较值
 ** \param [in]	Timer2CCMask:  TMR2_CC0 ~ TMR2_CC3
 **				CompareValue:  0x0 ~ 0xFFFF;
 ** \return  none
 ** \note   
 *****************************************************************************/
void TMR2_ConfigCompareValue(uint8_t Timer2CCn, uint16_t CompareValue);

/*****************************************************************************
 ** \brief	TMR2_ConfigCompareIntMode
 **			配置比较通道比较中断模式
 ** \param [in]	Timer2CompareIntMode:	(1)TMR2_CMP_INT_MODE0   
 **										(2)TMR2_CMP_INT_MODE1	  
 ** \return  none
 ** \note   
 **						 
 *****************************************************************************/
void TMR2_ConfigCompareIntMode(uint8_t Timer2CompareIntMode);

/*****************************************************************************
 ** \brief	TMR2_EnableCapture
 **			使能捕获模式
 ** \param [in]	Timer2CCMask:  TMR2_CC0 ~ TMR2_CC3
 **				Timer2CaptureMode: (1)TMR2_CAP_WRITE_REGISTER   捕获通道在写寄存器时捕获
 **								   (2)TMR2_CAP_EDGE_FALLING		捕获通道下降沿捕获			  	
 **								   (3)TMR2_CAP_EDGE_RISING		捕获通道上升沿捕获
 **								   (4)TMR2_CAP_EDGE_BOTH		捕获通道在上升和下降边沿都可捕获							   
 ** \return  none
 ** \note  
 *****************************************************************************/
void TMR2_EnableCapture(uint8_t Timer2CCn, uint8_t Timer2CaptureMode);

/*****************************************************************************
 ** \brief	TMR2_DisableCapture
 **			关闭捕获模式
 ** \param [in]	Timer2CCn:  TMR2_CC0 ~ TMR2_CC3
 ** \return  none
 ** \note   
*****************************************************************************/
void TMR2_DisableCapture(uint8_t Timer2CCn);

/*****************************************************************************
 ** \brief	TMR2_GetCaptureValue
 **			获取捕获值
 ** \param [in]	Timer2CCn:  TMR2_CC0 ~ TMR2_CC3			
 ** \return  16bit value
 ** \note   
 *****************************************************************************/
uint16_t TMR2_GetCaptureValue(uint8_t Timer2CCn);

/*****************************************************************************
 ** \brief	TMR2_EnableOverflowInt
 **			使能定时器2溢出中断
 ** \param [in] none
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR2_EnableOverflowInt(void);
/*****************************************************************************
 ** \brief	TMR2_DisableOverflowInt
 **			关闭定时器2溢出中断
 ** \param [in] none
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR2_DisableOverflowInt(void);
/*****************************************************************************
 ** \brief	TMR_GetOverflowIntFlag
 **			获取定时器2溢出中断标志
 ** \param [in] none
 ** \return   0：无中断， 1：有中断
 ** \note   
 *****************************************************************************/
uint8_t TMR2_GetOverflowIntFlag(void);
/*****************************************************************************
 ** \brief	TMR2_ClearOverflowIntFlag
 **			清除定时器2溢出中断标志
 ** \param [in] none
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR2_ClearOverflowIntFlag(void);

/*****************************************************************************
 ** \brief	TMR2_EnableT2EXInt
 **			使能定时器2外部加载T2EX中断
 ** \param [in] none
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR2_EnableT2EXInt(void);

/*****************************************************************************
 ** \brief	TMR2_DisableT2EXInt
 **			关闭定时器2外部加载T2EX中断
 ** \param [in] none
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR2_DisableT2EXInt(void);
/*****************************************************************************
 ** \brief	TMR_GetT2EXIntFlag
 **			获取定时器2外部加载T2EX中断标志
 ** \param [in] none
 ** \return   0：无中断， 1：有中断
 ** \note   
 *****************************************************************************/
uint8_t TMR2_GetT2EXIntFlag(void);
/*****************************************************************************
 ** \brief	TMR2_ClearT2EXIntFlag
 **			清除定时器2外部加载T2EX中断标志
 ** \param [in] none
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR2_ClearT2EXIntFlag(void);

/*****************************************************************************
 ** \brief	TMR2_EnableCompareInt
 **			使能定时器2通道比较中断
 ** \param [in]Timer2CCn:  TMR2_CC0 ~ TMR2_CC3
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR2_EnableCompareInt(uint8_t Timer2CCn);

/*****************************************************************************
 ** \brief	TMR2_DisableCompareInt
 **			关闭定时器2通道比较中断
 ** \param [in] Timer2CCn:  TMR2_CC0 ~ TMR2_CC3
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR2_DisableCompareInt(uint8_t Timer2CCn);
/*****************************************************************************
 ** \brief	TMR2_GetCompareIntFlag
 **			获取定时器2通道比较中断标志
 ** \param [in]Timer2CCn:  TMR2_CC0 ~ TMR2_CC3
 ** \return   0：无中断， 1：有中断
 ** \note   
 *****************************************************************************/
uint8_t TMR2_GetCompareIntFlag(uint8_t Timer2CCn);
/*****************************************************************************
 ** \brief	TMR2_ClearCompareIntFlag
 **			清除定时器2通道比较中断标志
 ** \param [in]Timer2CCn:  TMR2_CC0 ~ TMR2_CC3
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR2_ClearCompareIntFlag(uint8_t Timer2CCn);

/*****************************************************************************
 ** \brief	TMR2_EnableCaptureInt
 **			使能定时器2通道捕获中断
 ** \param [in]Timer2CCn:  TMR2_CC0 ~ TMR2_CC3
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR2_EnableCaptureInt(uint8_t Timer2CCn);
/*****************************************************************************
 ** \brief	TMR2_DisableCaptureInt
 **			关闭定时器2通道捕获中断
 ** \param [in]Timer2CCn:  TMR2_CC0 ~ TMR2_CC3
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR2_DisableCaptureInt(uint8_t Timer2CCn);
/*****************************************************************************
 ** \brief	TMR2_GetCaptureIntFlag
 **			获取定时器2通道捕获中断标志
 ** \param [in]Timer2CCn:  TMR2_CC0 ~ TMR2_CC3
 ** \return   0：无中断， 1：有中断
 ** \note   
 *****************************************************************************/
uint8_t TMR2_GetCaptureIntFlag(uint8_t Timer2CCn);
/*****************************************************************************
 ** \brief	TMR2_ClearCaptureIntFlag
 **			清除定时器2通道捕获中断标志
 ** \param [in]Timer2CCn:  TMR2_CC0 ~ TMR2_CC3
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR2_ClearCaptureIntFlag(uint8_t Timer2CCn);


/*****************************************************************************
 ** \brief	TMR2_AllIntEnable
 **			定时器2总中断开启
 ** \param [in] none
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR2_AllIntEnable(void);

/*****************************************************************************
 ** \brief	TMR2_AllIntDisable
 **			定时器2总中断关闭
 ** \param [in] none
 ** \return   none
 ** \note   
 *****************************************************************************/
void TMR2_AllIntDisable(void);
/*****************************************************************************
 ** \brief	TMR2_Start
 **			开启定时器2
 ** \param [in] none
 ** \return  none
 ** \note   
 *****************************************************************************/
void TMR2_Start(void);
/*****************************************************************************
 ** \brief	TMR2_Stop
 **			关闭定时器2
 ** \param [in] none
 ** \return  none
 ** \note   
 *****************************************************************************/
void TMR2_Stop(void);

#endif /* __TIMER_H__ */






