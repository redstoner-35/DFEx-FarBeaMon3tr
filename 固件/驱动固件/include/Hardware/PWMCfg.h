/************************************************************************************/
/** \file PWMCfg.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个头文件为系统ePWM模块硬件驱动的外部声明文件，负责声明按键控制器的初
始化、特殊操作和事件处理函数，并声明PWM控制器控制参数的外部变量。

**	History: Initial Release
**	
/************************************************************************************/
#ifndef _PWM_
#define _PWM_

/************************************************************************************/
/* Extern Functions definition - Initialization */
/************************************************************************************/
void PWM_Init(void);
void PWM_DeInit(void);	

/************************************************************************************/
/* Extern Functions definition - Specialized PWM Controller Operation */
/************************************************************************************/
void PWM_ForceEnableOut(bit IsEnable);	

/************************************************************************************/
/* Extern Functions definition - PWM Controller Logic Handler */
/************************************************************************************/
void PWM_OutputCtrlHandler(void);

/************************************************************************************/
/* Extern Flags and Variable definition */
/************************************************************************************/
extern xdata float PWMDuty;									//恒流环路基准的PWMDAC输出
extern xdata unsigned int PreChargeDACDuty; //预充电PWMDAC的输出
extern bit IsNeedToUploadPWM; 							//指令bit,置位该bit以更新PWM寄存器应用输出	

#endif /* _PWM_ */

/*********************************  End Of File  ************************************/
