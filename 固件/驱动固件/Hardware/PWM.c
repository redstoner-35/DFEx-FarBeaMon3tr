#include "cms8s6990.h"
#include "PinDefs.h"
#include "GPIO.h"
#include "PWMCfg.h"

//全局变量
xdata float PWMDuty;
xdata int PreChargeDACDuty; //预充电PWMDAC的输出
static bit IsPWMLoading; //PWM正在加载中
static bit IsNeedToEnableOutput; //是否需要启用输出
static bit IsNeedToEnableMOS; //是否需要使能MOS管
bit IsNeedToUploadPWM; //是否需要更新PWM

//关闭PWM定时器
void PWM_DeInit(void)
	{
	//配置为普通GPIO
  GPIO_SetMUXMode(PWMDACIOG,PWMDACIOx,GPIO_AF_GPIO);	
	//关闭PWM模块
	PWMOE=0x00;
	PWMCNTE=0x00;
	PWM45PSC=0x00;
	PWM01PSC=0x00;  //关闭PWM计数器
	}

//上传PWM值
static void UploadPWMValue(void)	
	{
	PWMLOADEN=0x11; //加载通道0的PWM值
	while(PWMLOADEN&0x11); //等待加载结束
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
	GPIO_WriteBit(PreChargeDACIOG,PreChargeDACIOx,0);
  GPIO_WriteBit(PWMDACIOG,PWMDACIOx,0);
	GPIO_ConfigGPIOMode(PreChargeDACIOG,GPIOMask(PreChargeDACIOx),&PWMInitCfg); 
	GPIO_ConfigGPIOMode(PWMDACIOG,GPIOMask(PWMDACIOx),&PWMInitCfg); 
	//启用复用功能
	GPIO_SetMUXMode(PWMDACIOG,PWMDACIOx,GPIO_AF_PWMCH0);
  GPIO_SetMUXMode(PreChargeDACIOG,PreChargeDACIOx,GPIO_AF_PWMCH4);
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
	}

//短时间启用PWM输出的功能
void PWM_ForceEnableOut(bit IsEnable)	
	{
	PWMD0L=IsEnable?0xFF:0;	
	PWMD4H=IsEnable?0x08:0;
	PWMD4L=IsEnable?0x2A:0x0;	  //0x82A=87.128%=11.29-0.4815*14.4->(0.87128*5)
	UploadPWMValue();
	if(IsEnable)PWMMASKE&=0xEE;
	else PWMMASKE|=0x11;   //更新PWMMASKE寄存器根据输出状态启用对应的通道
	}

//根据PWM结构体内的配置进行输出
void PWM_OutputCtrlHandler(void)	
	{
	int value;
	float buf;
	//判断是否需要加载的逻辑运算
	if(!IsNeedToUploadPWM)return; //不需要加载
	else if(IsPWMLoading) //当次加载已开始，进行结束监测
		{
	  if(PWMLOADEN&0x11)return;//加载寄存器复位为0，表示加载成功
	  //加载结束
		if(IsNeedToEnableMOS)PWMMASKE&=0xEF;
		else PWMMASKE|=0x10;
		if(IsNeedToEnableOutput)PWMMASKE&=0xFE;
		else PWMMASKE|=0x01;   //更新PWMMASKE寄存器根据输出状态启用对应的通道
		IsNeedToUploadPWM=0;
		IsPWMLoading=0;  //正在加载状态为清除
		return;
		}
	//PWM占空比参数限制
	if(PWMDuty>100)PWMDuty=100;
	if(PWMDuty<0)PWMDuty=0;
	if(PreChargeDACDuty>2399)PreChargeDACDuty=2399;
	if(PreChargeDACDuty<0)PreChargeDACDuty=0;
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
