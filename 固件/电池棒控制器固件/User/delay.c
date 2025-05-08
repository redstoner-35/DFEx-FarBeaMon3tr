#include "ht32.h"
#include "delay.h"

//定义
#define systick_ms_step 6000 //Systick跑1ms的step
#define systick_us_step 6 //systick定时器跑1us的step

#pragma push
#pragma O0  //delay不用任何优化

char DelaySeconds;

//延时初始化
void delay_init(void)
 {
	SYSTICK_ClockSourceConfig(SYSTICK_SRC_STCLK);//使用48MHz AHB
	 SYSTICK_IntConfig(DISABLE);//禁止systick中断 
	SYSTICK_CounterCmd(SYSTICK_COUNTER_DISABLE);//关闭定时器
	SYSTICK_CounterCmd(SYSTICK_COUNTER_CLEAR);//清除定时器数据
	DelaySeconds=0; 
 }

//ms级别延时
void delay_ms(u16 ms)
{	
  #if (ms > 2790)
  #error Delayms()-ERROR: Delay time exceeded maxmium value
	#endif
	u32 temp;		   
	SysTick->LOAD=(u32)ms*systick_ms_step;				//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;							//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0X00;       					//清空计数器	  	    
}

//us级别延时
void delay_us(u16 us)
{	 		  	  
	u32 temp;	
	__disable_irq(); //为了避免影响us级别定时的精度，需要屏蔽所有中断
  #if (ms > 2796000)
  #error Delayus()-ERROR: Delay time exceeded maxmium value
	#endif	
	SysTick->LOAD=(u32)us*systick_us_step;				//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;							//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0x00;       					//清空计数器	  	    
	__enable_irq(); //定时结束，开启所有中断
}

//s级别延时
void delay_Second(u8 sec)
{	   
	SysTick->LOAD=(u32)systick_ms_step*1000;				//时间加载(延迟1秒钟)
	SysTick->VAL =0x00;							//清空计数器
	DelaySeconds=sec; //存放倒数秒数
  SYSTICK_IntConfig(ENABLE);//启动systick中断
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数  
  while(DelaySeconds);//等待计时结束 
	SYSTICK_IntConfig(DISABLE);//禁止systick中断
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0X00;       					//清空计数器	  	    
}
#pragma pop
