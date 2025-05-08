#include "ht32.h"
#include "Key.h"
#include "Pindefs.h"
#include "delay.h"

//整数绝对值定义
#define iabs(x) x>0?x:-x

//内部和全局变量
extern short SleepTimer;
KeyTimerDef KeyTimer[2]; //按键定时器，[0]=上，[1]=下
KeyStateStor KeyState; //按键状态存储

//侧部按键定时器处理
void SideKey_TIMCallback(void)
	{
	char i;
	KeyTimerDef *Timer;
	//循环处理两个定时器
	if(SleepTimer>0)SleepTimer--; //倒计时
	for(i=0;i<2;i++)
		{
		Timer=&KeyTimer[i]; //取地址
		if(Timer->EnablePressTimer&&Timer->PressTime<HoldTime)Timer->PressTime++;
		else if(!Timer->EnablePressTimer)Timer->PressTime=0;
		}
	}

//侧部按键中断处理
void SideKey_IntCallback(void)
	{
	//向上键按下
	if(EXTI_GetEdgeFlag(KeyUp_EXTI_CHANNEL))
		{
    EXTI_ClearEdgeFlag(KeyUp_EXTI_CHANNEL);
    KeyTimer[0].EnablePressTimer=true; //启动计时器
    EXTI_IntConfig(KeyUp_EXTI_CHANNEL,DISABLE); //禁用中断
		KeyState.KeyShift[0]=0x0000;
		}
	//向下键按下
	if(EXTI_GetEdgeFlag(KeyDown_EXTI_CHANNEL))
		{
		EXTI_ClearEdgeFlag(KeyDown_EXTI_CHANNEL);
    KeyTimer[1].EnablePressTimer=true; //启动计时器
    EXTI_IntConfig(KeyDown_EXTI_CHANNEL,DISABLE); //禁用中断
		KeyState.KeyShift[1]=0x0000;
		}
	}	
	
//侧按逻辑处理
void SideKey_LogicHandler(void)
	{
	//向上键按下
	if(KeyTimer[0].EnablePressTimer)
		{
		delay_ms(4);
		//移位去抖等待松开
		SleepTimer=480; //休眠时间复位为一分钟
		KeyState.KeyShift[0]<<=1;
		if(GPIO_ReadInBit(KeyUp_IOG,KeyUp_IOP)==SET)KeyState.KeyShift[0]++;
		else KeyState.KeyShift[0]&=0xFFFE;
		//按键已松开
		if(KeyState.KeyShift[0]==0xFFFF)
			{
			KeyTimer[0].EnablePressTimer=false;
			if(KeyState.KeyEvent==KeyEvent_None&&!KeyState.IsUpHold)KeyState.KeyEvent=KeyEvent_Up; //单击显示为Up
			KeyState.IsUpHold=false; //按键松开清掉false
			EXTI_ClearEdgeFlag(KeyUp_EXTI_CHANNEL);
			EXTI_IntConfig(KeyUp_EXTI_CHANNEL, ENABLE); //启用对应的按键中断	
			}
		}
	//向下键按下
	if(KeyTimer[1].EnablePressTimer)
		{
		delay_ms(4);
		SleepTimer=480; //休眠时间一分钟
		//移位去抖等待松开
		KeyState.KeyShift[1]<<=1;
		if(GPIO_ReadInBit(KeyDown_IOG,KeyDown_IOP)==SET)KeyState.KeyShift[1]++;
		else KeyState.KeyShift[1]&=0xFFFE;
		//按键已松开
		if(KeyState.KeyShift[1]==0xFFFF)
			{
			KeyTimer[1].EnablePressTimer=false;
			if(KeyState.KeyEvent==KeyEvent_None&&!KeyState.IsDownHold)KeyState.KeyEvent=KeyEvent_Down; //单击显示为Down
			KeyState.IsDownHold=false; //按键松开清掉flag
		  EXTI_ClearEdgeFlag(KeyDown_EXTI_CHANNEL);
			EXTI_IntConfig(KeyDown_EXTI_CHANNEL, ENABLE); //启用对应的按键中断	
			}
		}	
	//向上键按下足够时间
	if(KeyTimer[0].PressTime==HoldTime)
		{
		KeyState.IsUpHold=true;
		if(!KeyTimer[1].EnablePressTimer)KeyState.KeyEvent=KeyEvent_Enter; //只有一个按键按下，判定为确认
		else if(iabs(KeyTimer[0].PressTime-KeyTimer[1].PressTime)<3)KeyState.KeyEvent=KeyEvent_BothEnt; //两个按键都按下且按下的时间几乎一致，判定为同时按下
		KeyTimer[0].PressTime++; //时间加1确保这里只跑一次
		}
	//向下键按下足够时间	
	else if(KeyTimer[1].PressTime==HoldTime)
		{
		KeyState.IsDownHold=true;
		if(!KeyTimer[0].EnablePressTimer)KeyState.KeyEvent=KeyEvent_ESC; //只有一个按键按下，判定为退出
		KeyTimer[1].PressTime++; //时间加1确保这里只跑一次
		}		
	}
	
//侧部按键配置
void SideKey_Init(void)
	{
	EXTI_InitTypeDef EXTI_InitStruct;
	char i;
	//配置外部中断系统基本参数
	EXTI_InitStruct.EXTI_Debounce = EXTI_DEBOUNCE_ENABLE; 
  EXTI_InitStruct.EXTI_DebounceCnt = 5;  //启用去抖
  EXTI_InitStruct.EXTI_IntType = EXTI_NEGATIVE_EDGE; //负边沿触发
	//配置向上按键的GPIO
  AFIO_GPxConfig(KeyUp_IOB,KeyUp_IOP, AFIO_FUN_GPIO);//GPIO功能
  GPIO_DirectionConfig(KeyUp_IOG,KeyUp_IOP,GPIO_DIR_IN);//配置为输入
	GPIO_PullResistorConfig(KeyUp_IOG,KeyUp_IOP,GPIO_PR_UP); //启用内部上拉
	GPIO_InputConfig(KeyUp_IOG,KeyUp_IOP,ENABLE);//启用IDR
	//配置向上按键的中断
	AFIO_EXTISourceConfig(KeyUp_IOPN,KeyUp_IOB); //配置中断源
	EXTI_InitStruct.EXTI_Channel = KeyUp_EXTI_CHANNEL; //通道选择为对应的通道
  EXTI_Init(&EXTI_InitStruct);  
  EXTI_IntConfig(KeyUp_EXTI_CHANNEL, ENABLE); //启用对应的按键中断	
  NVIC_EnableIRQ(KeyUp_EXTI_IRQn); //启用IRQ
	//配置向下按键的GPIO
  AFIO_GPxConfig(KeyDown_IOB,KeyDown_IOP, AFIO_FUN_GPIO);//GPIO功能
  GPIO_DirectionConfig(KeyDown_IOG,KeyDown_IOP,GPIO_DIR_IN);//配置为输入
	GPIO_PullResistorConfig(KeyDown_IOG,KeyDown_IOP,GPIO_PR_UP); //启用内部上拉
	GPIO_InputConfig(KeyDown_IOG,KeyDown_IOP,ENABLE);//启用IDR
	//配置向上按键的中断
	AFIO_EXTISourceConfig(KeyDown_IOPN,KeyDown_IOB); //配置中断源
	EXTI_InitStruct.EXTI_Channel = KeyDown_EXTI_CHANNEL; //通道选择为对应的通道
  EXTI_Init(&EXTI_InitStruct);  
  EXTI_IntConfig(KeyDown_EXTI_CHANNEL, ENABLE); //启用对应的按键中断	
  NVIC_EnableIRQ(KeyDown_EXTI_IRQn); //启用IRQ	
	//初始化定时器
	for(i=0;i<2;i++)KeyTimer[i].EnablePressTimer=false;
  //初始化按键结构体
  KeyState.IsDownHold=false;
	KeyState.IsUpHold=false;
	KeyState.KeyEvent=KeyEvent_None;
	for(i=0;i<2;i++)KeyState.KeyShift[i]=0xFFFF;
	}
