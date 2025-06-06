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

//内部寄存器宏定义
#define LEDBrightnessHalfMSB (LEDBrightnessHalf>>8)&0xFF
#define LEDBrightnessHalfLSB LEDBrightnessHalf&0xFF         //亮度一半的LSB和MSB
#define LEDBrightnessFullMSB (LEDBrightnessFull>>8)&0xFF
#define LEDBrightnessFullLSB LEDBrightnessFull&0xFF         //亮度开满的LSB和MSB

//设置LED亮度
static void SetLEDBrightNess(void)
	{
	//设置占空比寄存器
	if(IsHalfBrightness)
		{
		PWMD2H=LEDBrightnessHalfMSB;
		PWMD3H=LEDBrightnessHalfMSB;
		PWMD2L=LEDBrightnessHalfLSB;
		PWMD3L=LEDBrightnessHalfLSB;
		}
	else
		{
		PWMD2H=LEDBrightnessFullMSB;
		PWMD3H=LEDBrightnessFullMSB;
		PWMD2L=LEDBrightnessFullLSB;
		PWMD3L=LEDBrightnessFullLSB;		
		}
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
	PWMP3H=0x09;
	PWMP2L=0x5F;
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
	
//根据传入的指令，设置PWM寄存器控制侧按LED开启与否
static void SetLEDONOFF(LEDCommandDef Command)
	{
	unsigned char buf;
  //读取寄存器并设置
	buf=PWMMASKE&0xF3;
	PWMMASKE=buf|(unsigned char)Command;
	}		

//将匹配的LED管理器显示模式转换为对应的侧按LED寄存器配置命令的内部转换函数
static LEDCommandDef ConvertLEDModeToCmd(LEDStateDef Mode)
	{
  unsigned char buf;
	/******************************************
	这里是利用了一个骚招。
	当传入的Mode=1（红灯）时，结果如下：
	~(0x03(0000 0011)<< 1) = 0xF9(1111 1001)
	使用0x0C mask掉其他位之后=0x08，相当于输出
	LED_ROnly。
	当传入的Mode=2（黄灯）时，结果如下：
	~(0x03(0000 0011)<< 2) = 0xF3(1111 0011)
	使用0x0C mask掉其他位之后=0x00，相当于输出
	LED_RPlusG。
	当传入的Mode=3（绿灯）时，结果如下：
	~(0x03(0000 0011)<< 3) = 0xE7(1110 0111)
	使用0x0C mask掉其他位之后=0x04，相当于输出
	LED_GOnly。
	******************************************/
	buf=0x03<<Mode;
	buf=(~buf)&0x0c;
	//返回运算结果
	return (LEDCommandDef)buf;
	}
	
//LED控制函数
void LEDControlHandler(void)
	{
	bit IsLEDON=0;
	LEDCommandDef Command=LED_BothOFF;
	//执行特殊的逻辑（战术模式指示等）
	if(LEDMode<LED_RedBlinkFifth) //非一次性状态，进行降档和战术模式指示判断
		{
		if(DisplayTacModeEnabled()) //战术显示启动，降低LED亮度
			{
			IsHalfBrightness=1;
			Command=LED_GOnly;
			IsLEDON=1;
			}
		else if(ShowThermalStepDown())IsLEDON=1; //标记需要跳过状态机运行
		}
	//根据index设置LED状态
	if(!IsLEDON)switch(LEDMode)
		{
		case LED_OFF:timer=0;break; //LED关闭
		case LED_Amber:
		case LED_Green:
		case LED_Red:
			//常规颜色使用内部转换函数直接转换为命令
			Command=ConvertLEDModeToCmd(LEDMode);
			break;
		case LED_RedBlink: //红色闪烁
		  //如果计时变量bit2=1说明已经计数到4
			if(timer&0x04)
				{
				timer&=0x80; //复位掉计数部分
				timer^=0x80; //和0x80 XOR取反bit7
				}
			//否则继续计数
			else timer++;
			if(timer&0x80)Command=LED_ROnly; //根据bit 7载入LED控制位
			break;
		case LED_GreenBlinkThird:
		case LED_RedBlinkThird: //LED红色闪烁3次
		case LED_RedBlinkFifth: //LED红色闪烁5次
			timer&=0x7F; //去掉最上面的位
			if(timer>((LEDMode==LED_RedBlinkThird||LEDMode==LED_GreenBlinkThird)?6:10))LEDMode=LED_OFF; //时间到，关闭识别
			else if((timer++)%2)//继续计时,符合条件则点亮LED
				{
				if(LEDMode==LED_GreenBlinkThird)Command=LED_ROnly;
				else Command=LED_GOnly;
				}		
		  break;
		}
	//设置LED亮度
  SetLEDBrightNess();
	SetLEDONOFF(Command);
	}
	
//制造一次快闪
void MakeFastStrobe(LEDStateDef Mode)
	{
	//参数检查，其余传入值都无效
	if(Mode<1||Mode>3)return;
	//打开LED
	SetLEDONOFF(ConvertLEDModeToCmd(Mode));
	delay_ms(20);
	//关闭LED
	SetLEDONOFF(LED_BothOFF);
	}	
