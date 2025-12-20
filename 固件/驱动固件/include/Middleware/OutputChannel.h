/************************************************************************************/
/** \file OutputChannel.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个头文件为系统的高级输出电流合成器（输出通道模块）的声明文件。该文件提
供了系统的中层电流合成虚拟设备的驱动并声明输出通道的初始化函数、参数配置相关变量和模块处理
逻辑以及状态查询的函数。同时该文件提供了系统上电自检时对驱动输出进行自检的函数。

**	History: Initial Release
**	
/*************************************************************************************/
#ifndef _OutputChannel_
#define _OutputChannel_

/************************************************************************************/
/* Extern Functions definition - Initialization */
/************************************************************************************/
void OutputChannel_Init(void);
void OutputChannel_DeInit(void); //输出通道硬件初始化以及复位

/************************************************************************************/
/* Extern Functions definition - Call Back For OuputChannel Handler */
/************************************************************************************/
void OutputChannel_Calc(void);
void OCFSM_TIMHandler(void);   

/************************************************************************************/
/* Extern Functions definition - Power-On Self Test related */
/************************************************************************************/
void OutputChannel_WaitVBattReady(void); 	//等待电池电压就绪
void OutputChannel_TestRun(void); 				//输出通道试运行

/************************************************************************************/
/* Extern Functions definition - Query System Status */
/************************************************************************************/
bit GetIfOutputEnabled(void);		//外部获取输出是否正常启用的函数
bit GetIfSystemInPOFFSeq(void);	//获取系统是否在安全关机阶段

/************************************************************************************/
/* Extern Flags and Variable definition */
/************************************************************************************/
extern xdata volatile int Current; 	//输出通道目标的电流值
extern xdata int CurrentBuf; 				//当前输出通道已应用的电流值
extern bit IsCurrentRampUp;  				//电流正在上升过程中的标记位（用于和MPPT试探联动）

//输出通道是否已经启动成功的标志位（特殊功能挡位下用于确保系统已启动再执行特殊功能）
extern bit IsOutputStarted;  				

/************************************************************************************/
/* Extern Fast Operation Macro definition */
/************************************************************************************/

//输出通道电流参考和PWMDAC整定计算宏（绝对不要修改！会爆炸！）	
#define CalcIREFValue(x) ((x/2)+(x/6))

#endif /* _OutputChannel_ */

/*********************************  End Of File  ************************************/
