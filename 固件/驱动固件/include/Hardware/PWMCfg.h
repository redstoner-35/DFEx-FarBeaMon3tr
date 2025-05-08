#ifndef _PWM_
#define _PWM_

//PWM参数设置
#define SysFreq 48000000 //系统时钟频率(单位Hz)
#define PWMFreq 6000 //PWM频率(单位Hz)	
	
//PWM计数器配置	
#define PWMStepConstant (SysFreq/PWMFreq)-1 //PWM周期自动定义
#define iabsf(x) x>0?x:-x //整数绝对值

//PWM使能操作
#define PWM_Enable() 	PWMFBKC=0x00;PWMCNTE=0x1D //使能通道0的计数器，PWM开始运行
	 
	
//PWM输出配置结构体
extern xdata float PWMDuty;	
extern bit IsNeedToUploadPWM; //需要更新PWM寄存器应用输出
extern xdata int PreChargeDACDuty; //预充电PWMDAC的输出
	
//函数
void PWM_Init(void);
void PWM_DeInit(void);
void PWM_OutputCtrlHandler(void);	
void PWM_ForceEnableOut(bit IsEnable);	
	
#endif
