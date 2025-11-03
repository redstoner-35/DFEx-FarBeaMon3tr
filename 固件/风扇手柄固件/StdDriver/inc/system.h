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
/** \file system.h
**
** 
**
** History:
** 
*****************************************************************************/
#ifndef __SYSTEM_H__
#define __SYSTEM_H__
/*****************************************************************************/
/* Include files */
/*****************************************************************************/
#include "cms8s6990.h"
/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define') */
/*****************************************************************************/
/*----------------------------------------------------------------------------
 **System 系统时钟常用分频系数
---------------------------------------------------------------------------*/
#define		SYS_CLK_DIV_1 		(0x00)		
#define 	SYS_CLK_DIV_2		(0x01)
#define		SYS_CLK_DIV_4 		(0x02)
#define 	SYS_CLK_DIV_6		(0x03)
#define		SYS_CLK_DIV_8 		(0x04)
#define 	SYS_CLK_DIV_10		(0x05)
#define		SYS_CLK_DIV_12 		(0x06)
#define 	SYS_CLK_DIV_14		(0x07)
#define		SYS_CLK_DIV_16 		(0x08)
#define 	SYS_CLK_DIV_18		(0x09)
#define 	SYS_CLK_DIV_20		(0x0a)


/*----------------------------------------------------------------------------
 **System 系统LVD电压监测点
---------------------------------------------------------------------------*/
#define 	SYS_LVD_2_0V		(0x00)							/*LVD电压监测点2.0V*/
#define 	SYS_LVD_2_2V		(0x01<<LVD_LVDCON_LVDSEL_Pos)	/*LVD电压监测点2.2V*/
#define 	SYS_LVD_2_4V		(0x02<<LVD_LVDCON_LVDSEL_Pos)	/*LVD电压监测点2.4V*/
#define 	SYS_LVD_2_7V		(0x03<<LVD_LVDCON_LVDSEL_Pos)	/*LVD电压监测点2.7V*/
#define 	SYS_LVD_3_0V		(0x04<<LVD_LVDCON_LVDSEL_Pos)	/*LVD电压监测点3.0V*/
#define 	SYS_LVD_3_7V		(0x05<<LVD_LVDCON_LVDSEL_Pos)	/*LVD电压监测点3.7V*/
#define 	SYS_LVD_4_0V		(0x06<<LVD_LVDCON_LVDSEL_Pos)	/*LVD电压监测点4.0V*/
#define 	SYS_LVD_4_3V		(0x07<<LVD_LVDCON_LVDSEL_Pos)	/*LVD电压监测点4.3V*/

/*----------------------------------------------------------------------------
 ** 定时唤醒 时钟分频系数
---------------------------------------------------------------------------*/
#define		WUT_CLK_DIV_1 			(0x00)		// 1分频
#define		WUT_CLK_DIV_8 			(0x01)		// 8分频
#define		WUT_CLK_DIV_32 			(0x02)		// 32分频
#define		WUT_CLK_DIV_256 		(0x03)		// 256分频

/*****************************************************************************
 ** \brief	 SYS_SET_SYSTEM_CLK
 **			 配置系统时钟
 ** \param [in] SystemClkDiv: 系统时钟分频(0x00~0xff)
 ** \return  none
 ** \note	 00H:Fsys=Fosc; 其他：Fsys =Fosc/(2*CLKDIV)(2,4..510)	
*****************************************************************************/
//操作TA时序时不允许被打断，因而需要关闭中断。在CPU_WAITCLOCK选择1T的模式时，在EA=0后必须加nop,选择多T时不加。
#define		SYS_SET_SYSTEM_CLK(SystemClkDiv)	do{\
														if(EA == 1)\
															{\    
																EA = 0;\		
																_nop_();\
																TA = 0xAA;\
																TA = 0x55;\
																CLKDIV = SystemClkDiv;\
																EA = 1;\
															}\
															else\
															{\
																TA = 0xAA;\
																TA = 0x55;\
																CLKDIV = SystemClkDiv;\
															}\				
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
 ** \brief	 SYS_EnableLVD
 **			 开启电压监测功能
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SYS_EnableLVD(void);
/*****************************************************************************
 ** \brief	 SYS_DisableLVD
 **			  关闭电压监测功能
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SYS_DisableLVD(void);
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
void SYS_ConfigLVD(uint8_t LVDValue);

/*****************************************************************************
 ** \brief	 SYS_EnableLVDInt
 **			 开启电压监测中断功能
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SYS_EnableLVDInt(void);
/*****************************************************************************
 ** \brief	 SYS_DisableLVDInt
 **			  关闭电压监测中断功能
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SYS_DisableLVDInt(void);

/*****************************************************************************
 ** \brief	 SYS_GetLVDIntFlag
 **			 获取LVD中断标志位
 ** \param [in] none
 ** \return  0:无中断 1：有中断
 ** \note	 
*****************************************************************************/
uint8_t SYS_GetLVDIntFlag(void);

/*****************************************************************************
 ** \brief	 SYS_ClearLVDIntFlag
 **			 清除LVD中断标志位
 ** \param [in] none
 ** \return  none
 ** \note	 
*****************************************************************************/
void SYS_ClearLVDIntFlag(void);


/*****************************************************************************
 ** \brief	 SYS_EnableWDTReset
 **			 使能WDT复位System
 ** \param [in] none
 ** \return none
 ** \note
 *****************************************************************************/
void SYS_EnableWDTReset(void);
/*****************************************************************************
 ** \brief	 SYS_DisableWDTReset
 **			 关闭WDT复位System
 ** \param [in] none
 ** \return none
 ** \note
 *****************************************************************************/
void SYS_DisableWDTReset(void);
/*****************************************************************************
 ** \brief	 SYS_GetWDTResetFlag
 **			 获取WDT复位System的标志
 ** \param [in] none
 ** \return 0：复位不由WDT溢出引起    1：复位由WDT溢出引起
 ** \note
 *****************************************************************************/
uint8_t SYS_GetWDTResetFlag(void);
/*****************************************************************************
 ** \brief	 SYS_ClearWDTResetFlag
 **			 清除WDT复位System的标志
 ** \param [in] none
 ** \return   none
 ** \note
 *****************************************************************************/
void SYS_ClearWDTResetFlag(void);

/*****************************************************************************
 ** \brief	 SYS_EnableSoftwareReset
 **			 使能软件复位System
 ** \param [in] none
 ** \return none
 ** \note
 *****************************************************************************/
void SYS_EnableSoftwareReset(void);
/*****************************************************************************
 ** \brief	 SYS_DisableSoftwareReset
 **			 关闭软件复位System
 ** \param [in] none
 ** \return none
 ** \note
 *****************************************************************************/
void SYS_DisableSoftwareReset(void);

/*****************************************************************************
 ** \brief	 SYS_GetPowerOnResetFlag
 **			 获取上电复位System的标志
 ** \param [in] none
 ** \return   0：复位不由上电复位引起    1：复位由上电复位引起
 ** \note
 *****************************************************************************/
uint8_t SYS_GetPowerOnResetFlag(void);


/*****************************************************************************
 ** \brief	 SYS_ClearPowerOnResetFlag
 **			 清除PowerOn复位System的标志
 ** \param [in] none
 ** \return   none
 ** \note
 *****************************************************************************/
void SYS_ClearPowerOnResetFlag(void);

/*****************************************************************************
 ** \brief	 SYS_EnableWakeUp
 **			 使能休眠唤醒
 ** \param [in] none
 **
 ** \return none
 ** \note	 可由外部中断唤醒
 *****************************************************************************/
void SYS_EnableWakeUp(void);
/*****************************************************************************
 ** \brief	 SYS_EnableWakeUp
 **			 关闭休眠唤醒
 ** \param [in] none
 **
 ** \return none
 ** \note	
 *****************************************************************************/
void SYS_DisableWakeUp(void);

/*****************************************************************************
 ** \brief	 SYS_EnterStop
 **			 进入休眠中STOP状态
 ** \param [in] none
 **
 ** \return none
 ** \note	
 *****************************************************************************/
void SYS_EnterStop(void);
/*****************************************************************************
 ** \brief	 SYS_EnterIdle
 **			 进入休眠模式中Idle状态
 ** \param [in] none
 **
 ** \return none
 ** \note   
 *****************************************************************************/
void SYS_EnterIdle(void);


/*****************************************************************************
 ** \brief	 SYS_EnableWakeUpTrig
 **			 使能定时唤醒功能
 ** \param [in] none
 **
 ** \return none
 ** \note   
 *****************************************************************************/
void SYS_EnableWakeUpTrig(void);
/*****************************************************************************
 ** \brief	 SYS_DisableWakeUpTrig
 **			 关闭定时唤醒功能
 ** \param [in] none
 **
 ** \return none
 ** \note   
 *****************************************************************************/
void SYS_DisableWakeUpTrig(void);

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
void SYS_ConfigWUTCLK(uint8_t ckkdiv );

/*****************************************************************************
 ** \brief	 SYS_ConfigWUTTime
 **			设置定时唤醒时间
 ** \param [in] time: 12it（0x0 ~ 0xFFF）
 ** \return none
 ** \note   
 *****************************************************************************/
void SYS_ConfigWUTTime(uint16_t time );



#endif /* __SYSTEM_H__ */



