#ifndef _Key_
#define _Key_

#include <stdbool.h>

//长按计时
#define HoldTime 10 //长按时间

//按键定时器结构体
typedef struct
	{
	char PressTime;
	bool EnablePressTimer;
	}KeyTimerDef;
	
typedef enum
	{
	KeyEvent_None,
	KeyEvent_Up,     //向上
	KeyEvent_Down,   //向下
	KeyEvent_Enter,  //确认
	KeyEvent_ESC,     //退出
	KeyEvent_BothEnt, //上下同时按住
	}KeyEventDef;	

	
typedef struct
	{
	unsigned short KeyShift[2]; //用来做去抖的处理
	bool IsUpHold; //向上键是否持续按住
	bool IsDownHold; //向下键是否持续按住
	KeyEventDef KeyEvent; //按键事件
	}KeyStateStor;	
	
/*负责上按键的自动Define，不允许修改！*/
#define KeyUp_IOB STRCAT2(GPIO_P,KeyUp_IOBank)
#define KeyUp_IOG STRCAT2(HT_GPIO,KeyUp_IOBank)
#define KeyUp_IOP STRCAT2(GPIO_PIN_,KeyUp_IOPN) 	
	
#define KeyUp_EXTI_CHANNEL  STRCAT2(EXTI_CHANNEL_,KeyUp_IOPN)
#define _KeyUp_EXTI_IRQn STRCAT2(EXTI,KeyUp_IOPN)
#define KeyUp_EXTI_IRQn  STRCAT2(_KeyUp_EXTI_IRQn,_IRQn)

/*负责下按键的自动Define，不允许修改！*/
#define KeyDown_IOB STRCAT2(GPIO_P,KeyDown_IOBank)
#define KeyDown_IOG STRCAT2(HT_GPIO,KeyDown_IOBank)
#define KeyDown_IOP STRCAT2(GPIO_PIN_,KeyDown_IOPN) 	
	
#define KeyDown_EXTI_CHANNEL  STRCAT2(EXTI_CHANNEL_,KeyDown_IOPN)
#define _KeyDown_EXTI_IRQn STRCAT2(EXTI,KeyDown_IOPN)
#define KeyDown_EXTI_IRQn  STRCAT2(_KeyDown_EXTI_IRQn,_IRQn)	

//回调处理
void SideKey_TIMCallback(void);	//侧部按键定时器处理
void SideKey_IntCallback(void); //侧部按键中断处理
void SideKey_LogicHandler(void); //逻辑处理

//外部引用
extern KeyStateStor KeyState; //按键按下处理

//函数
void SideKey_Init(void); //侧按初始化

#endif
