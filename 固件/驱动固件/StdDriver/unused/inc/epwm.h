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
/** \file epwm.h
**
** 
**
** History:
** 
******************************************************************************/
#ifndef __EPWM_H__
#define __EPWM_H__
/*****************************************************************************/
/* Include files */
/*****************************************************************************/
#include "cms8s6990.h"
/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define') */
/*****************************************************************************/
/*----------------------------------------------------------------------------
 **EPWM通道Mask
/*---------------------------------------------------------------------------*/
#define EPWM_CHANNEL_NUM                     (6)    /* EPWM channel number */
#define EPWM_CH_0_MSK                       (0x1)   /* EPWM channel 0 mask */
#define EPWM_CH_1_MSK                       (0x2)   /* EPWM channel 1 mask */
#define EPWM_CH_2_MSK                       (0x4)   /* EPWM channel 2 mask */
#define EPWM_CH_3_MSK                       (0x8)   /* EPWM channel 3 mask */
#define EPWM_CH_4_MSK                       (0x10)  /* EPWM channel 4 mask */
#define EPWM_CH_5_MSK                       (0x20)  /* EPWM channel 5 mask */

#define EPWM0								(0x00)	/* EPWM channel 0 number */
#define EPWM1								(0x01)	/* EPWM channel 1 number */
#define EPWM2								(0x02)	/* EPWM channel 2 number */
#define EPWM3								(0x03)	/* EPWM channel 3 number */
#define EPWM4								(0x04)	/* EPWM channel 4 number */
#define EPWM5								(0x05)	/* EPWM channel 5 number */
/*----------------------------------------------------------------------------
 **EPWM计数模式
/*---------------------------------------------------------------------------*/
#define EPWM_COUNT_UP_DOWN			(0x02)  /*EPWM计数（COUNT）模式：上下计数（中心对齐）模式*/
#define EPWM_COUNT_DOWN				(0x00)  /*EPWM计数（COUNT）模式：向下计数（边沿对齐）模式*/
/*----------------------------------------------------------------------------
 **EPWM输出比较单元(OCU)模式
/*---------------------------------------------------------------------------*/
#define EPWM_OCU_ASYMMETRIC			(0x04)  /*EPWM输出比较单元（OCU）模式； 非对称计数模式*/
#define EPWM_OCU_SYMMETRIC			(0x00)  /*EPWM输出比较单元（OCU）模式； 对称计数模式*/
/*----------------------------------------------------------------------------
 **EPWM波形发生器单元(WFG)模式
/*---------------------------------------------------------------------------*/
#define EPWM_WFG_INDEPENDENT 		(0x00)  /*EPWM波形发生器（WFG）模式：单个（独立）模式*/
#define EPWM_WFG_COMPLEMENTARY 		(0x10)  /*EPWM波形发生器（WFG）模式：互补模式*/
#define EPWM_WFG_SYNCHRONIZED		(0x20)  /*EPWM波形发生器（WFG）模式：同步模式*/
/*----------------------------------------------------------------------------
 **EPWM输出控制单元(OC)模式
/*---------------------------------------------------------------------------*/
#define EPWM_OC_GROUP					(0x08)  /*EPWM输出控制（OC）模式： 组控模式*/
#define EPWM_OC_INDEPENDENT 			(0x00)  /*EPWM输出控制（OC）模式： 通道各自独立控制*/
/*----------------------------------------------------------------------------
 **EPWM刹车源
/*---------------------------------------------------------------------------*/
#define EPWM_BRK_SOFTWARE  					(0x00)		/*软件刹车*/
#define EPWM_BRK_FB							(0x01)		/*FB刹车*/
#define EPWM_BRK_ACMP0						(0x02)		/*ACMP0刹车*/
#define EPWM_BRK_ACMP1						(0x03)		/*ACMP1刹车*/
#define EPWM_BRK_ADC						(0x04)		/*ADC比较刹车*/
/*----------------------------------------------------------------------------
 **EPWM刹车源FB刹车触发模式
/*---------------------------------------------------------------------------*/
#define EPWM_BRK_FB_LOW						(0x0<< EPWM_PWMFBKC_PWMFBES_Pos)	/*FB低电平触发刹车*/
#define EPWM_BRK_FB_HIGH					(0x1<< EPWM_PWMFBKC_PWMFBES_Pos)	/*FB高电平触发刹车*/
/*----------------------------------------------------------------------------
 **EPWM 刹车计数器模式
/*---------------------------------------------------------------------------*/
#define EPWM_BRK_COUNT_HOLD					(0x1<< EPWM_PWMFBKC_PWMFBCCE_Pos)	/*刹车后计数器不清0*/
#define EPWM_BRK_COUNT_CLEAR				(0x00)					/*刹车后计数器清0*/

/*----------------------------------------------------------------------------
 **EPWM时钟分频系数
 * 总分频(PWMnDIV分频) = Fpwmn-PSC分频 * CLK_DIV分频
 *   Fpwmn-PSC分频 = PWMnPSC/2 (PWMnPSC不为0的情况)----2分频
/*---------------------------------------------------------------------------*/
#define EPWM_CLK_DIV_1						(0xff)					//=Fsys（系统时钟）
#define EPWM_CLK_DIV_2						(0x04)					//= Fpwmn-PSC   (PWMnPSC/2)
#define EPWM_CLK_DIV_4						(0x00)					//= Fpwmn-PSC/2 ((PWMnPSC/2)/2)
#define EPWM_CLK_DIV_8						(0x01)					//= Fpwmn-PSC/4 ((PWMnPSC/2)/4)
#define EPWM_CLK_DIV_16						(0x02)					//= Fpwmn-PSC/8 ((PWMnPSC/2)/8)
#define EPWM_CLK_DIV_32						(0x03)					//= Fpwmn-PSC/16((PWMnPSC/2)/16)



/*****************************************************************************
 ** \brief	 EPWM_ENABLE_LOAD
 **			 使能加载周期、占空比数据(PERIODn、CMPn、CMPDn)
 ** \param [in]ChannelMask : EPWM_CH_0_MSK ~ EPWM_CH_5_MSK
 ** \return  none
 ** \note    
 ****************************************************************************/
#define EPWM_ENABLE_LOAD(ChannelMask) 	(PWMLOADEN |= ChannelMask)

/*****************************************************************************
 ** \brief	 EPWM_ENABLE_COUNT
 **			 使能EPWM计数
 ** \param [in] ChannelMask : EPWM_CH_0_MSK ~ EPWM_CH_5_MSK
 ** \return  none
** \note    
 ****************************************************************************/
#define EPWM_ENABLE_COUNT(ChannelMask) 	(PWMCNTE |= ChannelMask)

/*****************************************************************************
 ** \brief	 EPWM_DISABLE_COUNT
 **			 关闭EPWM计数
 ** \param [in] ChannelMask : EPWM_CH_0_MSK ~ EPWM_CH_5_MSK
 ** \return  none
** \note   
 ****************************************************************************/
#define EPWM_DISABLE_COUNT(ChannelMask) 	(PWMCNTE &= ~ChannelMask)

/*****************************************************************************
 ** \brief	 EPWM_ENABLE_OUTPUT
 **			 使能EPWM输出
 ** \param [in] ChannelMask : EPWM_CH_0_MSK 、EPWM_CH_1_MSK 、EPWM_CH_2_MSK 
 **							 EPWM_CH_3_MSK 、EPWM_CH_4_MSK 、EPWM_CH_5_MSK
 ** \return  none
 ****************************************************************************/
#define EPWM_ENABLE_OUTPUT(ChannelMask) 	(PWMOE |= ChannelMask)
/*****************************************************************************
 ** \brief	 EPWM_DISABLE_OUTPUT
 **			 关闭EPWM输出
 ** \param [in]ChannelMask : EPWM_CH_0_MSK 、EPWM_CH_1_MSK 、EPWM_CH_2_MSK 
 **							 EPWM_CH_3_MSK 、EPWM_CH_4_MSK 、EPWM_CH_5_MSK
 ** \return  none
 ****************************************************************************/
#define EPWM_DISABLE_OUTPUT(ChannelMask) 	(PWMOE &= ~ChannelMask)
/*****************************************************************************
 ** \brief	 EPWM_SET_MASK_LEVEL
 **			设置EPWM掩码电平
 ** \param [in] MaskValue : xx000000 : 设置EPWM0~5 掩码值为低
 **						  ......
 **						  xx111111 : 设置EPWM0~5 掩码值为高
 ** \return  none
 ** \note    PG0 : bit0 、PG1：bit1、PG2: bit2、PG3：bit3、PG4：bit4、PG5：bit5
 ****************************************************************************/
#define EPWM_SET_MASK_LEVEL(MaskValue)  (PWMMASKD = MaskValue)

/*****************************************************************************
 ** \brief	 EPWM_ENABLE_MASK
 **			开启EPWM掩码输出
 ** \param [in] ChannelMsk : EPWM_CH_0_MSK ~ EPWM_CH_5_MSK
 ** \return  none
 ** \note    
 ****************************************************************************/
#define EPWM_ENABLE_MASK(ChannelMsk)  (PWMMASKE |= ChannelMsk)

/*****************************************************************************
 ** \brief	 EPWM_DISABLE_MASK
 **			 关闭EPWM掩码输出
 ** \param [in] ChannelMsk : EPWM_CH_0_MSK ~ EPWM_CH_5_MSK
 ** \return  none
 ** \note    
 ****************************************************************************/
#define EPWM_DISABLE_MASK(ChannelMsk)  (PWMMASKE &= ~(ChannelMsk))

/*****************************************************************************
 ** \brief	 EPWM_SET_BRAKE_LEVEL
 **			设置EPWM刹车电平
 ** \param [in]MaskValue : xx000000 : 设置EPWM0~5 刹车电平为低
 **						  ......
 **						  xx111111 : 设置EPWM0~5 刹车电平为高
 ** \return  none
 ** \note    PG0 : bit0 、PG1：bit1、PG2: bit2、PG3：bit3、PG4：bit4、PG5：bit5
 ****************************************************************************/
#define EPWM_SET_BRAKE_LEVEL(BrakeValue)  ( PWMFBKD = BrakeValue)
												  

/*****************************************************************************/
/* Global type definitions ('typedef') */
/*****************************************************************************/


/*****************************************************************************/
/* Global variable declarations ('extern', definition in C source) */
/*****************************************************************************/
/*****************************************************************************
 ** \brief	 EPWM_ConfigRunMode
 **			 设置EPWM的运行模式
 ** \param [in] EpwmRunMode:  
							   (1)EPWM_COUNT_UP_DOWN
							   (2)EPWM_COUNT_DOWN
							   (3)EPWM_OCU_ASYMMETRIC
							   (4)EPWM_OCU_SYMMETRIC
							   (5)EPWM_WFG_INDEPENDENT
							   (6)EPWM_WFG_COMPLEMENTARY
							   (7)EPWM_WFG_SYNCHRONIZED
							   (8)EPWM_OC_GROUP
							   (9)EPWM_OC_INDEPENDENT
 ** \return  none
 ** \note    以上9种摸式可组合成不同的运行模式，且共用一个8位寄存器
 ****************************************************************************/
void EPWM_ConfigRunMode( uint8_t EpwmRunMode);
/*****************************************************************************
 ** \brief	 EPWM_ConfigChannelClk	
 ** \param [in] ClkDiv: (1)EPWM_CLK_DIV_1 
 **						(2)EPWM_CLK_DIV_2
 **						(2)EPWM_CLK_DIV_4
 **						(2)EPWM_CLK_DIV_8
 **						(2)EPWM_CLK_DIV_16
 **						(2)EPWM_CLK_DIV_32
 ** \return  none	
 ** \note 
 **	(1)PWM01PSC共同控制PWM0、PWM1通道 ，所以此函数统一设置PWMxxPSC=0x01
 **	若需要其他的分频，请酌情对寄存器赋值
 **	(2)EPWM 分频分为两级：PWMxPSC PWMnDIV 
 **	第一级分频(PWMxPSC)：0x00 预分频停止并且计数器停止，系统时钟分频系数PWMxPSC+1
 **	第二级分频（PWMnDIV): 0x00 FEPWMPSC/2; 0x01 FEPWMPSC/4；0x02:FEPWMPSC/8；0x3 FEPWMPSC/16
 **	0x04 FEPWMPSC ; 其他 Fsys 
 ** 分频级数：1~4096，例Fsys=8M，EPWM的频率范围：1.95k ~ 8M	
 ****************************************************************************/
void EPWM_ConfigChannelClk(uint8_t ChannelNum,uint8_t ClkDiv);
/*****************************************************************************
 ** \brief	 EPWM_ConfigChannelPeriod
 **			 设置EPWM通道的频率以及占空比
 ** \param [in] ChannelNum : EPWM0,EPWM1,EPWM2,EPWM3,EPWM4,EPWM5
 **				Period: 16位周期值
 ** \return none
 ** \note    
 ****************************************************************************/
void EPWM_ConfigChannelPeriod(uint8_t ChannelNum,uint16_t Period);
 
/*****************************************************************************
 ** \brief	 EPWM_ConfigChannelSymDuty
 **			 设置EPWM通道对称模式占空比
 ** \param [in] ChannelNum  :EPWM0,EPWM1,EPWM2,EPWM3,EPWM4,EPWM5
 **				Duty: 16位占空比
 ** \return none
 ** \note   
 ****************************************************************************/
void EPWM_ConfigChannelSymDuty(uint8_t ChannelNum,uint16_t Duty);
 /*****************************************************************************
 ** \brief	 EPWM_ConfigChannelAsymDuty
 **			 设置EPWM通道非对称模式占空比
 ** \param [in] ChannelNum  :EPWM0,EPWM1,EPWM2,EPWM3,EPWM4,EPWM5
 **				DownCmp: 16 bit 向下比较值
 ** \return none
 ** \note    只适用于中心对齐计数(上下计数)模式
 ****************************************************************************/
void EPWM_ConfigChannelAsymDuty(uint8_t ChannelNum,uint16_t UpCmp, uint16_t DowmCmp);
 
 /****************************************************************************
 ** \brief	 EPWM_EnableOneShotMode
 **			 开启One-Shot模式
 ** \param [in]  ChannelMask   EPWM_CH_0_MSK ~ EPWM_CH_5_MSK
 ** \return  none
 ** \note	
*****************************************************************************/
void EPWM_EnableOneShotMode(uint8_t ChannelMask);
/****************************************************************************
 ** \brief	 EPWM_EnableAutoLoadMode
 **			 开启自动加载模式
 ** \param [in]  ChannelMask   EPWM_CH_0_MSK ~ EPWM_CH_5_MSK
 ** \return  none
 ** \note	
*****************************************************************************/
void EPWM_EnableAutoLoadMode(uint8_t ChannelMask);

/*****************************************************************************
 ** \brief	 EPWM_Start
 **			
 ** \param [in]  ChannelMask : EPWM_CH_0_MSK ~ EPWM_CH_5_MSK
 ** \return  none
 ** \note	
 ****************************************************************************/
void EPWM_Start( uint8_t ChannelMask);
/*****************************************************************************
 ** \brief	 EPWM_Stop
 **				
 ** \param [in] ChannelMask   EPWM_CH_0_MSK ~ EPWM_CH_5_MSK
 ** \return  none
 ** \note    停止计数并且EPWM计数器清0
*****************************************************************************/
void EPWM_Stop( uint8_t ChannelMask);

/*****************************************************************************
 ** \brief	 EPWM_EnableOutput
 **			 开启EPWM输出
 ** \param [in] ChannelMask   EPWM_CH_0_MSK ~ EPWM_CH_5_MSK
 ** \return  none
 ** \note   
*****************************************************************************/
void EPWM_EnableOutput( uint8_t ChannelMask);
/*****************************************************************************
 ** \brief	 EPWM_DisableOutput
 **			 关闭EPWM输出
 ** \param [in] ChannelMask   EPWM_CH_0_MSK ~ EPWM_CH_5_MSK
 ** \return  none
 ** \note    
*****************************************************************************/
void EPWM_DisableOutput( uint8_t ChannelMask);
/*****************************************************************************
 ** \brief	 EPWM_EnableFaultBrake
 **			 设置EPWM刹车模式
 ** \param [in] BrakeSource: [0]: EPWM_BRK_SOFTWARE		:软件刹车
 **						   	[1]: EPWM_BRK_FB			:外部硬件刹车通道
 **                        	[2]: EPWM_BRK_ACMP0			:比较器0刹车
 **                        	[3]: EPWM_BRK_ACMP1			:比较器1刹车
 ** 					   	[4]: EPWM_BRK_ADC			:ADC刹车
 **				CountMode:(1)EPWM_BRK_COUNT_HOLD		
 **						  (2)EPWM_BRK_COUNT_CLEAR
 ** \return  none
 ** \note  	 
*****************************************************************************/
void EPWM_EnableFaultBrake( uint8_t BrakeSource , uint8_t CountMode);
/*****************************************************************************
 ** \brief	 EPWM_DisableFaultBrake
 **			 关闭EPWM刹车
 ** \param [in] BrakeSource: [0]: EPWM_BRK_SOFTWARE		:软件刹车
 **						   	[1]: EPWM_BRK_FB			:外部硬件刹车通道
 **                        	[2]: EPWM_BRK_ACMP0			:比较器0刹车
 **                        	[3]: EPWM_BRK_ACMP1			:比较器1刹车
 ** 					   	[4]: EPWM_BRK_ADC			:ADC刹车
 ** \return  none
 ** \note    
*****************************************************************************/
void EPWM_DisableFaultBrake( uint8_t BrakeSource);
/****************************************************************************
 ** \brief	 EPWM_ConfigChannelBrakeLevel
 **			 设置EPWM通道刹车电平
 ** \param [in]  ChannelMask： EPWM_CH_0_MSK ~ EPWM_CH_5_MSK
 **		   		 BrakeLevel: FaultBreake后的EPWM通道电平 ：0/1(低/高)
 ** \return  none
 ** \note   
*****************************************************************************/
void EPWM_ConfigChannelBrakeLevel( uint8_t ChannelMask, uint8_t BrakeLevel);

/****************************************************************************
 ** \brief	 EPWM_EnableDeadZone
 **			 开启死区
 ** \param [in] ChannelMask: EPWM_CH_0_MSK ~ EPWM_CH_5_MSK		
 **			DeadTime : 死区时间( 0~0xFF) = DeadTime * Tepwm
 ** \return  none
 ** \note  
*****************************************************************************/
void EPWM_EnableDeadZone( uint8_t ChannelMask, uint8_t DeadTime);
/*****************************************************************************
 ** \brief	 EPWM_DisableDeadZone
 **			 关闭EPWM死区
 ** \param [in] ChannelMask:  EPWM_CH_0_MSK ~ EPWM_CH_5_MSK		
 ** \return  none
 ** \note  
*****************************************************************************/
void EPWM_DisableDeadZone( uint8_t ChannelMask);
/*****************************************************************************
 ** \brief	 EPWM_EnableMaskControl
 **			 掩码控制开启
 ** \param [in] ChannelMask:  EPWM_CH_0_MSK ~ EPWM_CH_5_MSK	 	
 ** 	    	MaskLevel: 掩码的数据 :0/1(低/高)
 ** \return  none
 ** \note   
*****************************************************************************/
void EPWM_EnableMaskControl( uint8_t ChannelMask, uint8_t MaskLevel);
/*****************************************************************************
 ** \brief	 EPWM_DisableMaskControl
 **			 掩码控制关闭
 ** \param [in] ChannelMask:  EPWM_CH_0_MSK ~ EPWM_CH_5_MSK	 		
 ** \return  none
 ** \note   
*****************************************************************************/
void EPWM_DisableMaskControl( uint8_t ChannelMask);
/*****************************************************************************
 ** \brief	 EPWM_EnableUpCmpInt
 **			 开启向上比较中断
 ** \param [in]  ChannelMask： EPWM_CH_0_MSK ~ EPWM_CH_5_MSK		
 ** \return  none
 ** \note   
*****************************************************************************/
void EPWM_EnableUpCmpInt( uint8_t ChannelMask);
/*****************************************************************************
 ** \brief	 EPWM_DisableUpCmptInt
 **			 关闭向上比较中断
 ** \param [in]ChannelMask ：EPWM_CH_0_MSK ~ EPWM_CH_5_MSK	
 ** \return  none
 ** \note   
*****************************************************************************/
void EPWM_DisableUpCmpInt( uint8_t ChannelMask);
/*****************************************************************************
 ** \brief	 EPWM_GetUpCmpIntFlag
 **			 获取向上比较中断标志位
 ** \param [in] ChannelNum:EPWM0,EPWM1,EPWM2,EPWM3,EPWM4,EPWM5 
 ** \return  0: 无中断产生
 **          1：有中断产生
 ** \note   
*****************************************************************************/
uint8_t EPWM_GetUpCmpIntFlag( uint8_t ChannelNum);
/*****************************************************************************
 ** \brief	 EPWM_ClearUpCmpIntFlag
 **			 清除向上比较中断标志位
 ** \param [in] ChannelNum:EPWM0,EPWM1,EPWM2,EPWM3,EPWM4,EPWM5
 ** \return  none
 ** \note   
*****************************************************************************/
void EPWM_ClearUpCmpIntFlag( uint8_t ChannelNum);
/*****************************************************************************
 ** \brief	 EPWM_EnableDownCmpInt
 **			 开启向下比较中断
 ** \param [in] ChannelMask : EPWM_CH_0_MSK ~ EPWM_CH_5_MSK	
 ** \return  none
 ** \note   
*****************************************************************************/
void EPWM_EnableDownCmpInt( uint8_t ChannelMask);
/*****************************************************************************
 ** \brief	 EPWM_DisableDownCmpInt
 **			 关闭向下比较中断
 ** \param [in] ChannelMask : EPWM_CH_0_MSK ~ EPWM_CH_5_MSK	
 ** \return  none
 ** \note   
*****************************************************************************/
void EPWM_DisableDownCmpInt( uint8_t ChannelMask);
/*****************************************************************************
 ** \brief	 EPWM_GetUpCmpIntFlag
 **			 获取向下比较中断标志位
 ** \param [in] ChannelNum: EPWM0,EPWM1,EPWM2,EPWM3,EPWM4,EPWM5
 ** \return  0: 无中断产生
 **          1：有中断产生
 ** \note   
*****************************************************************************/
uint8_t EPWM_GetDownCmpIntFlag( uint8_t ChannelNum);
/*****************************************************************************
 ** \brief	 EPWM_ClearUpCmpIntFlag
 **			 清除向下比较中断标志位
 ** \param [in] ChannelNum :EPWM0,EPWM1,EPWM2,EPWM3,EPWM4,EPWM5
 ** \return  none
 ** \note   
*****************************************************************************/
void EPWM_ClearDownCmpIntFlag( uint8_t ChannelNum);
/*****************************************************************************
 ** \brief	 EPWM_EnablePeriodInt
 **			 开启周期中断
 ** \param [in] ChannelMask: EPWM_CH_0_MSK ~ EPWM_CH_5_MSK	
 ** \return   none     
 ** \note   
*****************************************************************************/
void EPWM_EnablePeriodInt( uint8_t ChannelMask);
/*****************************************************************************
 ** \brief	 EPWM_DisablePeriodInt
 **			 关闭周期中断
 ** \param [in]  ChannelMask: EPWM_CH_0_MSK ~ EPWM_CH_5_MSK	
 ** \return   none     
 ** \note   
*****************************************************************************/
void EPWM_DisablePeriodInt( uint8_t ChannelMask);
/*****************************************************************************
 ** \brief	 EPWM_ClearPeriodIntFlag
 **			 获取周期中断标志位 
 ** \param [in]  ChannelNum: EPWM0,EPWM1,EPWM2,EPWM3,EPWM4,EPWM5
 ** \return   0：无中断产生
 **           1：有中断产生
 ** \note   
*****************************************************************************/
uint8_t EPWM_GetPeriodIntFlag( uint8_t ChannelNum);
/*****************************************************************************
 ** \brief	 EPWM_ClearPeriodIntFlag
 **			 清除周期中断标志位 
 ** \param [in]  ChannelNum: EPWM0,EPWM1,EPWM2,EPWM3,EPWM4,EPWM5
 ** \return   none     
 ** \note   
*****************************************************************************/
void EPWM_ClearPeriodIntFlag( uint8_t ChannelNum);
/*****************************************************************************
 ** \brief	 EPWM_EnableZeroInt
 **			 开启零点中断
 ** \param [in]  ChannelMask: EPWM_CH_0_MSK ~ EPWM_CH_5_MSK	
 ** \return   none     
 ** \note   
*****************************************************************************/
void EPWM_EnableZeroInt( uint8_t ChannelMask);
/*****************************************************************************
 ** \brief	 EPWM_DisableZeroInt
 **			 关闭零点中断 
 ** \param [in]ChannelMask: EPWM_CH_0_MSK ~ EPWM_CH_5_MSK	
 ** \return   none     
 ** \note   
*****************************************************************************/
void EPWM_DisableZeroInt( uint8_t ChannelMask);
/*****************************************************************************
 ** \brief	 EPWM_GetZeroIntFlag
 **			 获取零点中断标志位 
 ** \param [in] ChannelNum: EPWM0,EPWM1,EPWM2,EPWM3,EPWM4,EPWM5
 ** \return   0：无中断产生
 **           1：有中断产生   
 ** \note   
 *****************************************************************************/
uint8_t EPWM_GetZeroIntFlag( uint8_t ChannelNum);
/*****************************************************************************
 ** \brief	 EPWM_ClearZeroIntFlag
 **			 清除零点中断标志位
 ** \param [in] ChannelNum: EPWM0,EPWM1,EPWM2,EPWM3,EPWM4,EPWM5
 ** \return   none     
 ** \note   
*****************************************************************************/
void EPWM_ClearZeroIntFlag( uint8_t ChannelNum);
/*****************************************************************************
 ** \brief	 EPWM_EnableFaultBrakeInt	
 **			 开启故障刹车中断 
 ** \param [in]none
 ** \return  none
 ** \note  
*****************************************************************************/
void EPWM_EnableFaultBrakeInt( void);
/*****************************************************************************
 ** \brief	 EPWM_DisableFaultBrakeInt	
 **			 关闭故障刹车中断 
 ** \param [in]none
 ** \return  none
 ** \note 
*****************************************************************************/
void EPWM_DisableFaultBrakeInt(void);
/*****************************************************************************
 ** \brief	 EPWM_GetFaultBrakeIntFlag
 **			 获取故障刹车中断标志位
 ** \param [in] none
 ** \return  0: 无中断产生
 **          1：有中断产生
 ** \note  
*****************************************************************************/
uint8_t EPWM_GetFaultBrakeIntFlag( void);
/*****************************************************************************
 ** \brief	 EPWM_ClearFaultBrakeIntFlag	
 ** 		 清除故障刹车中断标志位 
 ** \param [in] none
 ** \return  none
 ** \note 
 *****************************************************************************/
void EPWM_ClearFaultBrakeIntFlag( void);
/*****************************************************************************
 ** \brief	 EPWM_TrigSoftwareBrake
 **			  触发软件刹车
 ** \param [in]	none
 ** \return  none
 ** \note   
*****************************************************************************/
void  EPWM_TrigSoftwareBrake(void);
/*****************************************************************************
 ** \brief	 EPWM_DisableSoftwareBreake
 **			 关闭软件刹车
 ** \param [in] none
 ** \return  none
 ** \note   
*****************************************************************************/
void  EPWM_DisableSoftwareBrake(void);
/*****************************************************************************
 ** \brief	 EPWM_EnableReverseOutput
 **			 开启反向输出
 ** \param [in] ChannelMask : EPWM_CH_0_MSK ~ EPWM_CH_5_MSK	
 ** \return  none
 ** \note  正常模式： EPWM 开始从初值开始计数时，PWM输出口输出为低电平, 
 **	当EPWM计数值与向上、向下计数匹配时，输出翻转
*****************************************************************************/
void  EPWM_EnableReverseOutput( uint8_t ChannelMask);
/*****************************************************************************
 ** \brief	 EPWM_DisableReverseOutput
 **			 关闭反向输出
 ** \param [in]ChannelMask: EPWM_CH_0_MSK ~ EPWM_CH_5_MSK	
 ** \return  none
 ** \note   
*****************************************************************************/
void  EPWM_DisableReverseOutput( uint8_t ChannelMask);
/*****************************************************************************
 ** \brief	 EPWM_ConfigFBBrake
 **			 设置FB刹车
 ** \param [in] FBBrakeLevel : 触发条件(1)EPWM_BRK_FB_LOW
 **                           		    (2)EPWM_BRK_FB_HIGH	
 ** \return  none
 ** \note   
*****************************************************************************/
void  EPWM_ConfigFBBrake(uint8_t FBBrakeLevel);

/*****************************************************************************
 ** \brief	 EPWM_AllIntEnable
 **			 使能EPWM总中断
 ** \param [in] none
 ** \return  none
 ** \note   
*****************************************************************************/
void  EPWM_AllIntEnable(void);
/*****************************************************************************
 ** \brief	 EPWM_AllIntDisable
 **			 关闭EPWM总中断
 ** \param [in] none
 ** \return  none
 ** \note   
*****************************************************************************/
void  EPWM_AllIntDisable(void);

/*****************************************************************************/
/* Global function prototypes ('extern', definition in C source) */
/*****************************************************************************/

#endif /* __EPWM_H__ */







