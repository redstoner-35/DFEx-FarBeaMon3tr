#include "delay.h"
#include "LEDMgmt.h"
#include "GPIO.h"
#include "PinDefs.h"
#include "cms8s6990.h"

//全局变量
volatile LEDStateDef LEDMode; 
static char timer;
bit IsHalfBrightness;

//函数
bit ShowThermalStepDown(void);	//显示温度控制启动	
bit DisplayTacModeEnabled(void); //显示战术模式启动

//设置LED亮度
static void SetLEDBrightNess(void)
	{
	//设置占空比寄存器
	PWMD2H=IsHalfBrightness?(LEDBrightnessHalf>>8)&0xFF:(LEDBrightnessFull>>8)&0xFF;
	PWMD2L=IsHalfBrightness?LEDBrightnessHalf&0xFF:LEDBrightnessFull&0xFF; 
	PWMD3H=IsHalfBrightness?(LEDBrightnessHalf>>8)&0xFF:(LEDBrightnessFull>>8)&0xFF;
	PWMD3L=IsHalfBrightness?LEDBrightnessHalf&0xFF:LEDBrightnessFull&0xFF;  
  //应用占空比
	PWMLOADEN|=0x0C; //加载通道0的PWM值
	}
	
//LED配置函数
void LED_Init(void)
	{
	GPIOCfgDef LEDInitCfg;
	//设置结构体
	LEDInitCfg.Mode=GPIO_Out_PP;
  LEDInitCfg.Slew=GPIO_Slow_Slew;		
	LEDInitCfg.DRVCurrent=GPIO_High_Current; //配置为低斜率大电流的推挽输出
	//初始化模式设置
	LEDMode=LED_OFF;
	//配置PWM发生器
	PWM23PSC=0x01;  //打开预分频器和计数器时钟 
  PWM2DIV=0xff;   
	PWM3DIV=0xff;   //令Fpwmcnt=Fsys=48MHz(不分频)	
	//配置周期数据
	PWMP2H=0x09;
	PWMP2L=0x5F; 
	PWMP3H=0x09;
	PWMP3L=0x5F; //CNT=(48MHz/20Khz)-1=2399
  //启用PWM
	PWMCNTE|=0x0C; //使能通道2 3的计数器，PWM开始运作
	//配置占空比数据
	SetLEDBrightNess();
  while(PWMLOADEN&0x0C); //等待PWM完成应用
	//复位IO
	GPIO_WriteBit(GreenLEDIOG,GreenLEDIOx,0);	
	GPIO_WriteBit(RedLEDIOG,RedLEDIOx,0);	
	//配置GPIO（将配置GPIO拉到最后是因为避免PWM发生器工作异常引起闪烁）
	GPIO_ConfigGPIOMode(RedLEDIOG,GPIOMask(RedLEDIOx),&LEDInitCfg); //红色LED(推挽输出)
	GPIO_ConfigGPIOMode(GreenLEDIOG,GPIOMask(GreenLEDIOx),&LEDInitCfg); //绿色LED(推挽输出)
	GPIO_SetMUXMode(RedLEDIOG,RedLEDIOx,GPIO_AF_PWMCH2);
	GPIO_SetMUXMode(GreenLEDIOG,GreenLEDIOx,GPIO_AF_PWMCH3); //为了控制侧按LED的亮度改为PWM模式
	}	
	
//设置LED亮度	
static void SetLEDONOFF(bit RLED,bit GLED)
	{
	unsigned char buf;
  buf=PWMMASKE;
	if(RLED)buf&=0xFB;
	else buf|=0x04; //控制PWM通道2是否正常输出来打开关闭红色LED
	if(GLED)buf&=0xF7;
	else buf|=0x08; //控制PWM通道3是否正常输出来打开关闭绿色LED
	PWMMASKE=buf;
	}		

//LED控制函数
void LEDControlHandler(void)
	{
	char buf;
	bit IsLEDON,RLED=0,GLED=0;
	//执行特殊的逻辑（战术模式指示等）
	if(LEDMode<LED_RedBlinkFifth) //非一次性状态，进行降档和战术模式指示判断
		{
		if(DisplayTacModeEnabled()) //战术显示启动，降低LED亮度
			{
			IsHalfBrightness=1;
			GLED=1;  //战术模式的指示使用半亮度配置，点亮绿灯作为提示
			RLED=1; //标记需要跳过状态机运行
			}
		else if(ShowThermalStepDown())RLED=1; //标记需要跳过状态机运行
		}
	//根据index设置LED状态
	if(!RLED)switch(LEDMode)
		{
		case LED_OFF:timer=0;break; //LED关闭
		case LED_Amber:RLED=1;GLED=1;break;//黄色LED
		case LED_Green:GLED=1;break;//绿色LED
		case LED_Red:RLED=1;break;//红色LED
		case LED_RedBlink_Fast: //红色快闪	
		case LED_RedBlink: //红色闪烁
		  buf=timer&0x7F; //读取当前定时器的控制位
			if(buf<(LEDMode==LED_RedBlink?3:0))
				{
				buf++;
			  timer&=0x80;
				timer|=buf; //时间没到，继续计时
				}
			else timer=timer&0x80?0x00:0x80; //翻转bit 7并重置定时器
			RLED=timer&0x80?1:0; //根据bit 7载入LED控制位
			break;
		case LED_GreenBlinkThird:
		case LED_RedBlinkThird: //LED红色闪烁3次
		case LED_RedBlinkFifth: //LED红色闪烁5次
			timer&=0x7F; //去掉最上面的位
			if(timer>((LEDMode==LED_RedBlinkThird||LEDMode==LED_GreenBlinkThird)?6:10))LEDMode=LED_OFF; //时间到，关闭识别
			else //继续计时
				{
				IsLEDON=(timer%2)?1:0; //通过余2判断实现检测
				if(LEDMode==LED_GreenBlinkThird)GLED=IsLEDON;
				else RLED=IsLEDON;
				timer++;
				}		
		  break;
		}
	//特殊指示操作需要跳过状态机，清零RLED标记位
	else RLED=0;
	//设置LED亮度
  SetLEDBrightNess();
	SetLEDONOFF(RLED,GLED);
	}
	
//制造一次快闪
void MakeFastStrobe(LEDStateDef LEDMode)
	{
	//打开LED
	switch(LEDMode)
		{
		case LED_Green:SetLEDONOFF(0,1);break;//绿色LED
		case LED_Red:SetLEDONOFF(1,0);break;//红色LED
		case LED_Amber:SetLEDONOFF(1,1);break;//黄色LED
		default:return; //非法值
		}
	delay_ms(20);
	//关闭LED
	SetLEDONOFF(0,0);
	}	
