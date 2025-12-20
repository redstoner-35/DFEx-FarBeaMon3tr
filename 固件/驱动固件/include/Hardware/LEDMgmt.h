/************************************************************************************/
/** \file LEDMgmt.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个头文件为系统按键指示LED控制模块的外部声明文件。该文件声明了LED控制
器的初始化函数、特殊控制和逻辑处理函数并声明了LED控制器的操作全局变量以及控制器的可能类
型。

**	History: Initial Release
**	
/*************************************************************************************/
#ifndef _LEDMgmt_
#define _LEDMgmt_
/*************************************************************************************/
/*	Global type definitions('typedef')
**************************************************************************************/
typedef enum
	{
	LED_OFF=0, //关闭
	//常亮
	LED_Red=1, //红色常亮
	LED_Amber=2, //黄色常亮
	LED_Green=3, //绿色常亮
	//持续闪烁
	LED_RedBlink=4, //红色闪烁
	LED_AmberBlink=5, //黄色快闪三次
	//一次性快速闪烁
	LED_RedBlinkFifth=6, //红色快闪五次
	LED_GreenBlinkThird=7, //绿色快闪三次
	LED_RedBlinkThird=8, //红色快闪三次
	
	}LEDStateDef;
/************************************************************************************/
/* Extern Functions definition */
/************************************************************************************/	
void LED_Init(void);									//初始化侧按LED管理器
void LEDControlHandler(void);					//执行侧按LED管理控制
void MakeFastStrobe(LEDStateDef Mode);//令侧按LED强制快闪一次
	
/************************************************************************************/
/* Extern Flags and Variable definition */
/************************************************************************************/
extern volatile LEDStateDef LEDMode;	//设置侧按LED的动作模式
extern bit IsHalfBrightness; 					//设置侧按LED是否开启减半亮度模式

/************************************************************************************/
/* Extern Fast Operation Macro definition */
/************************************************************************************/
	
//判断是否为一次性闪烁的宏（利用了enum值的特性）
#define IsOneTimeStrobe() LEDMode>LED_AmberBlink 

#endif /* _LEDMgmt_ */

/*********************************  End Of File  ************************************/
