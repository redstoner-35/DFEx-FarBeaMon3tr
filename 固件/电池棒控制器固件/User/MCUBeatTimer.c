#include "HT32.h"
#include "delay.h"
#include "GUI.h"

//启用系统内的0.125秒心跳定时器
void EnableHBTimer(void)
 {
 CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};  
 TM_TimeBaseInitTypeDef TimeBaseInit;
 ShowPostInfo(3,"Starting HBTIM","01",Msg_Statu);
 //重新配置定时器用于产生0.125秒的定时中断
 CKCUClock.Bit.GPTM0 = 1;
 CKCU_PeripClockConfig(CKCUClock, ENABLE);
 TimeBaseInit.Prescaler = 479;                         // 48MHz->100KHz
 TimeBaseInit.CounterReload = 12499;                   // 100KHz->8Hz
 TimeBaseInit.RepetitionCounter = 0;
 TimeBaseInit.CounterMode = TM_CNT_MODE_UP;
 TimeBaseInit.PSCReloadTime = TM_PSC_RLD_IMMEDIATE;
 TM_TimeBaseInit(HT_GPTM0, &TimeBaseInit);
 TM_ClearFlag(HT_GPTM0, TM_FLAG_UEV);
 //配置好中断然后让定时器运行起来
 NVIC_EnableIRQ(GPTM0_IRQn);
 TM_IntConfig(HT_GPTM0,TM_INT_UEV,ENABLE);
 TM_Cmd(HT_GPTM0, ENABLE);
 }

void CheckIfHBTIMStart(void)
{
int retry=300;
extern bool SensorRefreshFlag;
ShowPostInfo(40,"测试心跳定时器\0","7A",Msg_Statu);
//检测是否有WDT Reset
if(RSTCU_GetResetFlagStatus(RSTCU_FLAG_WDTRST))
	{
	ShowPostInfo(40,"检测到系统卡死\0","7B",Msg_Warning);
	delay_Second(2);
	}
//开始测试定时器
SensorRefreshFlag=0;
do
	{
	delay_ms(1);
	if(SensorRefreshFlag)return; //成功完成检测
	retry--;
	}
while(retry>0);
//检测不通过
ShowPostInfo(40,"心跳定时器异常\0","7E",Msg_Fault);	
SelfTestErrorHandler();	
}	
 
//关闭系统内的0.125秒心跳定时器
void DisableHBTimer(void)
 {
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};  
 //禁用定时器并关闭中断
 TM_Cmd(HT_GPTM0, DISABLE);
 TM_ClearFlag(HT_GPTM0, TM_FLAG_UEV);
 NVIC_DisableIRQ(GPTM0_IRQn);
 //关闭定时器时钟
 CKCUClock.Bit.GPTM0 = 1;
 CKCU_PeripClockConfig(CKCUClock, DISABLE);	 
 }	
