#include "delay.h"
#include "LEDMgmt.h"
#include "GPIO.h"
#include "PinDefs.h"
#include "cms8s6990.h"
#include "BattDisplay.h"
#include "LVDCtrl.h"

//函数声明
bit QueryIsSystemInBoostMode(void);

//全局变量
volatile LEDStateDef LEDMode; 
static xdata char timer;
static xdata char BoostModeInfoTIM;

//内部sfr
sbit RLED=RedLEDIOP^RedLEDIOx;
sbit GLED=GreenLEDIOP^GreenLEDIOx;

//LED关闭函数
void LED_DeInit(void)
	{
	GPIOCfgDef LEDInitCfg;
	extern bit IsEnableIdleLED;
	//令所有LED熄灭
	RLED=0;
	GLED=0;
	//复位LED管理器的相关变量
	LEDMode=LED_OFF;
	timer=0;
	BoostModeInfoTIM=0;
	//如果电池电压低于2.9V为了避免导致电池彻底饿死，禁止打开定位LED（当然的话还有就是用户主动关闭）
	if(CellVoltage<2900||!IsEnableIdleLED)return;
	//设置结构体
	LEDInitCfg.Mode=GPIO_IPU;
  LEDInitCfg.Slew=GPIO_Slow_Slew;		
	LEDInitCfg.DRVCurrent=GPIO_High_Current; //配置为弱上拉令红色灯珠发出微光
	//配置GPIO并启动LVD
	LVD_Start();
	GPIO_ConfigGPIOMode(GreenLEDIOG,GPIOMask(GreenLEDIOx),&LEDInitCfg);
	}

//LED配置函数
void LED_Init(void)
	{
	GPIOCfgDef LEDInitCfg;
	//设置结构体
	LEDInitCfg.Mode=GPIO_Out_PP;
  LEDInitCfg.Slew=GPIO_Slow_Slew;		
	LEDInitCfg.DRVCurrent=GPIO_High_Current; //配置为低斜率大电流的推挽输出
	//初始化寄存器
	RLED=0;
	GLED=0;
	//配置GPIO
	GPIO_SetMUXMode(RedLEDIOG,RedLEDIOx,GPIO_AF_GPIO);
	GPIO_SetMUXMode(GreenLEDIOG,GreenLEDIOx,GPIO_AF_GPIO);
	GPIO_ConfigGPIOMode(RedLEDIOG,GPIOMask(RedLEDIOx),&LEDInitCfg); //红色LED(推挽输出)
	GPIO_ConfigGPIOMode(GreenLEDIOG,GPIOMask(GreenLEDIOx),&LEDInitCfg); //绿色LED(推挽输出)
	//初始化模式设置和变量
	timer=0;
	BoostModeInfoTIM=0;
	LEDMode=LED_OFF;
	}

//LED控制函数
void LEDControlHandler(void)
	{
	char buf;
	//进行狂暴模式工作指示
	if(QueryIsSystemInBoostMode())
		{
		if(BoostModeInfoTIM<12)BoostModeInfoTIM++;
		else
			{
			BoostModeInfoTIM=0;
			RLED=0;
			GLED=0;
			return;            //清零定时器，让LED短暂熄灭并退出
			}
		}
  else BoostModeInfoTIM=0;		
	//据目标模式设置LED状态
	switch(LEDMode)
		{
		case LED_OFF:RLED=0;GLED=0;timer=0;break; //LED关闭
		case LED_Green:RLED=0;GLED=1;break;//绿色LED
		case LED_Red:RLED=1;GLED=0;break;//红色LED
		case LED_Amber:RLED=1;GLED=1;break;//黄色LED
		case LED_AmberBlinkFast:
		  //黄色快闪
			timer=timer&0x80?0x00:0x80; //翻转bit 7并重置定时器
			if(!(timer&0x80))
				{
				RLED=0;
				GLED=0;
				}
			else
				{
				RLED=1;
				GLED=1;				
				}
			break;
		case LED_RedBlink_Fast: //红色快闪	
		case LED_RedBlink: //红色闪烁
			GLED=0;
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
			else if((timer++)%2)//继续计时,符合条件则点亮LED
				{
				//根据LED颜色输出对应的指令，点亮对应的LED
				if(LEDMode==LED_GreenBlinkThird)GLED=1;
				else RLED=1;
				}		
			else 
				{
				//不符合条件LED熄灭
				RLED=0;
				GLED=0;
				}
		  break;
		}
	}

//制造一次快闪
void MakeFastStrobe(LEDStateDef LEDMode)
	{
	//打开LED
	switch(LEDMode)
		{
		case LED_Green:RLED=0;GLED=1;break;//绿色LED
		case LED_Red:RLED=1;GLED=0;break;//红色LED
		case LED_Amber:RLED=1;GLED=1;break;//黄色LED
		default:return; //非法值
		}
	delay_ms(20);
	//关闭LED
	RLED=0;
	GLED=0;
	}	
