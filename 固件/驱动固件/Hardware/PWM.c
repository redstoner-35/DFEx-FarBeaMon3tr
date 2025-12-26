/****************************************************************************/
/** \file PWM.c
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个文件负责实现系统对外输出PWM的配置以实现PWMDAC功能控制LED的
的电流和亮度，并且实现输出FSM所需的预偏置受控斜率启动功能。

**	History:
				2025年12月26日 10:05 新增根据目标的LTC3787自检输出电压计算PWMDAC预充占
														 空比配置数据的宏定义，便于用户在需要的时候可以直接
														 修改自检输出电压而不需要重新计算占空比。
														 
				2025年12月20日 Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "cms8s6990.h"
#include "PinDefs.h"
#include "GPIO.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros for Parameter definition ('#define')
****************************************************************************/ 
#define SysFreq 48000000 //系统时钟频率(单位Hz)
#define PWMFreq 6000 //PWM频率(单位Hz)	

//系统在测试运行时LTC3787输出的电压，LSB=100mV（不要随便动！）
#define PreRunDACVoltage 144 

/****************************************************************************/
/*	Local pre-processor symbols/macros for Parameter Processing and Fast Op-
/*  eration with Register Operation('#define')
****************************************************************************/ 
#define CalcPreDACDuty(x) (((1129000UL-(4815UL*x))*24UL)/5000UL)  //使用整数方式计算PWMDAC预充电压
#define iabsf(x) (x>0?x:-x) 												//整数绝对值

#define PreRunDACDutyMSB ((CalcPreDACDuty(PreRunDACVoltage)>>8)&0xFF)
#define PreRunDACDutyLSB (CalcPreDACDuty(PreRunDACVoltage)&0xFF)  		//预充PWMDAC的PWM寄存器参数值计算
#define PWMStepConstant (SysFreq/PWMFreq)-1 				//主输出PWM周期自动定义
#define PWM_Enable() 	do{PWMCNTE=0x1D;}while(0) 		//PWM运行使能

#if (CalcPreDACDuty(PreRunDACVoltage) > 2399 | CalcPreDACDuty(PreRunDACVoltage) < 0)
   //自动检测预充DAC参数是否合法，如果不合法则禁止编译通过
   #error "Pre-charge voltage level is illegal which causing PWM Counter to overflow!"
#endif

#if (PWMStepConstant > 0xFFFE)
  //自动检测PWM的数值是否合法
	#error "PWM Frequency is too low which causing PWM Counter to overflow!"
#endif
/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/
xdata float PWMDuty;
xdata unsigned int PreChargeDACDuty; //预充电PWMDAC的输出
bit IsNeedToUploadPWM; //是否需要更新PWM

/****************************************************************************/
/*	Local variable definitions('static')
****************************************************************************/
static bit IsPWMLoading; //PWM正在加载中
static bit IsNeedToEnableOutput; //是否需要启用输出
static bit IsNeedToEnableMOS; //是否需要使能MOS管

/****************************************************************************/
/*	Local Special Register definitions('sfr' and 'sbit')
****************************************************************************/
sbit PWMDACPin=PWMDACIOP^PWMDACIOx;
sbit PreChargeDACPin=PreChargeDACIOP^PreChargeDACIOx;

/****************************************************************************/
/*	Local Function implementation ('static')
****************************************************************************/	

//上传PWM值（阻塞等待处理）
static void UploadPWMValue(void)	
	{
	PWMLOADEN=0x11; //加载通道0的PWM值
	while(PWMLOADEN&0x11); //等待加载结束
	}

/****************************************************************************/
/*	Global Function implementation - Initialization and De-Initialization
****************************************************************************/	

//关闭PWM定时器
void PWM_DeInit(void)
	{
	//配置为普通GPIO
	GPIO_SetMUXMode(PWMDACIOG,PWMDACIOx,GPIO_AF_GPIO);
  GPIO_SetMUXMode(PreChargeDACIOG,PreChargeDACIOx,GPIO_AF_GPIO);
	//关闭PWM模块
	PWMOE=0x00;
	PWMCNTE=0x00;		//关闭PWM计数器
	PWM45PSC=0x00;
	PWM01PSC=0x00;  //关闭PWM分频器时钟
	}

//PWM定时器初始化
void PWM_Init(void)
	{
	GPIOCfgDef PWMInitCfg;
	//设置结构体
	PWMInitCfg.Mode=GPIO_Out_PP;
  PWMInitCfg.Slew=GPIO_Fast_Slew;		
	PWMInitCfg.DRVCurrent=GPIO_High_Current; //推PWMDAC，不需要很高的上升斜率
	//配置GPIO
	PreChargeDACPin=0;
  PWMDACPin=0; 				//令PWMDAC的输出在非PWM模式下始终输出0
	GPIO_ConfigGPIOMode(PreChargeDACIOG,GPIOMask(PreChargeDACIOx),&PWMInitCfg); 
	GPIO_ConfigGPIOMode(PWMDACIOG,GPIOMask(PWMDACIOx),&PWMInitCfg); 
	//配置PWM发生器
	PWMCON=0x00; //PWM通道为六通道独立模式，向下计数，关闭非对称计数功能	
	PWMOE=0x1D; //打开PWM输出通道0 2 3 4
	PWM01PSC=0x01;  
	PWM45PSC=0x01;  //打开预分频器和计数器时钟 
  PWM0DIV=0xff;   
	PWM4DIV=0xff;   //令Fpwmcnt=Fsys=48MHz(不分频)
  PWMPINV=0x00; //所有通道均设置为正常输出模式
	PWMCNTM=0x1D; //通道0 2 3 4配置为自动加载模式
	PWMCNTCLR=0x1D; //初始化PWM的时候复位通道0 2 3 4的定时器
	PWMDTE=0x00; //关闭死区时间
	PWMMASKD=0x00; 
	PWMMASKE=0x1D; //PWM掩码功能启用，默认状态下禁止通道0 2 3 4输出
	//配置周期数据
	PWMP0H=(PWMStepConstant>>8)&0xFF;
	PWMP0L=PWMStepConstant&0xFF;	
	PWMP4H=0x09;
	PWMP4L=0x5F; //PWM通道周期(48MHz/20KHz)-1=2399(0x95F)
	//配置占空比数据
  PWMD0H=0;
  PWMD4H=0x0;
	PWMD0L=0;	
	PWMD4L=0x0;
	//初始化变量
	PWMDuty=0;
	PreChargeDACDuty=0;
	IsPWMLoading=0; 
	IsNeedToUploadPWM=0;
	//启用PWM
	PWM_Enable();
	UploadPWMValue();	
	//PWM初始化完毕，将引脚启用为复用功能
	GPIO_SetMUXMode(PWMDACIOG,PWMDACIOx,GPIO_AF_PWMCH0);
  GPIO_SetMUXMode(PreChargeDACIOG,PreChargeDACIOx,GPIO_AF_PWMCH4);
	}

/****************************************************************************/
/*	Global Function implementation - Special Control
****************************************************************************/		
	
//在特定场景下（例如硬件初始化自我测试时）强制启用PWM输出的功能
void PWM_ForceEnableOut(bit IsEnable)	
	{
	PWMD0L=IsEnable?0xFF:0;	
	PWMD4H=IsEnable?PreRunDACDutyMSB:0;
	PWMD4L=IsEnable?PreRunDACDutyLSB:0;	  //填写算出来的参数
	UploadPWMValue();
	if(IsEnable)PWMMASKE&=0xEE;
	else PWMMASKE|=0x11;   //更新PWMMASKE寄存器根据输出状态启用对应的通道
	}

/****************************************************************************/
/*	Global Function implementation - Logic Handler
****************************************************************************/		
	
//根据PWM结构体内的配置进行输出
void PWM_OutputCtrlHandler(void)	
	{
	int value;
	float buf;
	//当前系统未请求加载
	if(!IsNeedToUploadPWM)return; //不需要加载
	//当次加载已开始，进行结束监测
	else if(IsPWMLoading) 
		{
	  if(PWMLOADEN&0x11)return;//加载寄存器复位为0，表示加载成功
	  //加载结束
		if(IsNeedToEnableMOS)PWMMASKE&=0xEF;
		else PWMMASKE|=0x10;
		if(IsNeedToEnableOutput)PWMMASKE&=0xFE;
		else PWMMASKE|=0x01;   //更新PWMMASKE寄存器根据输出状态启用对应的通道
		IsNeedToUploadPWM=0;
		IsPWMLoading=0;  //正在加载状态为清除
		}
	//当次加载已被请求开始，进行加载处理
	else
		{
		//PWM占空比参数限制
		if(PWMDuty>100)PWMDuty=100;
		if(PWMDuty<0)PWMDuty=0;
		if(PreChargeDACDuty>2399)PreChargeDACDuty=2399;
		//根据PWM数值选择MASK寄存器是否启用
		IsNeedToEnableOutput=PWMDuty>0?1:0; //是否需要启用输出
		IsNeedToEnableMOS=PreChargeDACDuty?1:0;  //配置是否需要使能FET
		//配置寄存器装载PWM设置数值
		buf=PWMDuty*(float)PWMStepConstant;
		buf/=(float)100;
		value=(int)buf;
		PWMD4H=(PreChargeDACDuty>>8)&0xFF;
		PWMD4L=PreChargeDACDuty&0xFF;
		PWMD0H=(value>>8)&0xFF;
		PWMD0L=value&0xFF;			
		//PWM寄存器数值已装入，应用数值		
		IsPWMLoading=1; //标记加载过程进行中
		PWMLOADEN|=0x11; //开始加载
		}
	}
/*********************************  End Of File  ************************************/
