#include "cms8s6990.h"
#include "PinDefs.h"
#include "ADCCfg.h"
#include "delay.h"
#include "GPIO.h"

//ADC参数配置
#define VBattUpperResK 470
#define VBattLowerResK 100 //电池检测分压的上下拉电阻
#define ADCVREF 2.00 //ADC片内VREF为2V

//ADC数据输出结构体
ADCResultStrDef Data;

//关闭ADC
void ADC_DeInit(void)
	{
	GPIOCfgDef ADCDeInitCfg;	
	//配置寄存器关闭ADC
	ADCON1=0x00; //关闭ADC
	ADCLDO=0x00; //关闭片内基准
	//将GPIO设置为普通模式
	GPIO_SetMUXMode(VBATInputIOG,VBATInputIOx,GPIO_AF_GPIO);	
	//设置为推挽输出
	ADCDeInitCfg.Mode=GPIO_Out_PP;
  ADCDeInitCfg.Slew=GPIO_Slow_Slew;		
	ADCDeInitCfg.DRVCurrent=GPIO_Low_Current; //配置为低电流推挽输出
  GPIO_ConfigGPIOMode(VBATInputIOG,GPIOMask(VBATInputIOx),&ADCDeInitCfg); //设置对应的输出
	//全部输出0
	GPIO_WriteBit(VBATInputIOG,VBATInputIOx,0);
	}

//ADC初始化
void ADC_Init(void)
	{
	GPIOCfgDef ADCInitCfg;
	//初始化GPIO
	ADCInitCfg.Mode=GPIO_Input_Floating;
  ADCInitCfg.Slew=GPIO_Slow_Slew;		
	ADCInitCfg.DRVCurrent=GPIO_Low_Current; //配置为浮空输入	
	
	GPIO_ConfigGPIOMode(VBATInputIOG,GPIOMask(VBATInputIOx),&ADCInitCfg); 	
	GPIO_SetMUXMode(VBATInputIOG,VBATInputIOx,GPIO_AF_Analog); //将GPIO复用设置为模拟输入
  //配置ADC
	ADCON0=0x40; //AN31=内部1.2V基准，结果右对齐
	ADCON1=0x60; //Fadc=Fsys/128=375KHz
	ADCON2=0x00; //关闭ADC硬件触发功能，使用软件命令启动ADC
	ADCMPC=0x00; //关闭ADC比较器触发刹车功能
	ADDLYL=0x00; //将ADC硬件启动触发延时设置为0
	ADCMPH=0x0F;
	ADCMPL=0xFF; //ADC比较器默认值设置为0x0FFF
  ADCLDO=0xA0; //使能芯片内置ADC基准，输出2.0V
  ADCON1|=0x80; //令ADEN=1，启动芯片的ADC模块		
	//获取一遍初始的系统数据
  SystemTelemHandler();
	}

//从指定通道获取AD值
static int ADC_GetADValFromCh(char Ch)
	{
	int buf,i;
	unsigned char wait=0xFF;
	long avgbuf;
	//判断通道是否合法
	if(Ch<0||(Ch>22&&Ch<31))return 0;
	avgbuf=0; //缓存buf=0		
	//配置ADC通道		
	if(Ch&0x10)ADCON0|=0x80;
	else ADCON0&=0x7F; //设置ADCHS[4]
	ADCON1&=0xF0;
	ADCON1|=(Ch&0x0F); //设置ADCHS[3:0]				
	//开始转换前先等待200微秒进行采样
	while(--wait); 
	//开始转换
	for(i=0;i<ADCAverageCount;i++)
		{
		//启动ADC并等待
		ADCON0|=0x02; //令ADGO=1，开始转换
		while(ADCON0&0x02);//等待ADGO=0标志着转换结束
		//读取数据并进行平均值累加
		buf=ADRESL;
		buf|=ADRESH<<8;
		avgbuf+=(long)buf;
		}
	//计算平均值并返回结果
	avgbuf/=(long)ADCAverageCount;
	return avgbuf;
	}
//获取指定通道的电压
float GetChannelVoltage(char Ch)
	{
	float VREF,buf;
	//计算VREF
	if(ADCLDO&0x80)VREF=2.00; //启用ADCLDO
	else VREF=Data.MCUVDD;   //使用的是单片机的VDD
	//返回电压
	buf=(float)ADC_GetADValFromCh(Ch)*(float)VREF;
	return buf/(float)4096;
	}	
	
//系统数据获取处理模块	
void SystemTelemHandler(void)
	{
	float Buf;
	//获取电池电压
	Buf=(float)VBattLowerResK/(float)(VBattLowerResK+VBattUpperResK);//计算出分压电阻的系数
	Data.BatteryVoltage=GetChannelVoltage(VBATInputAIN)/Buf; //根据分压系数反推出电池电压

	//配置ADC的VREF为VDD，通过内部1.2V基准获得MCU的供电电压	
	ADCON1&=0x7F; //令ADEN=0		
	ADCLDO=0x00; //禁止芯片内部ADC基准
	ADCON1|=0x80; //令ADEN=1，重新打开ADC通道
  Buf=1.2*(float)4096/(float)ADC_GetADValFromCh(31);
	Data.MCUVDD=Buf; //计算出MCUVDD(VREF)
  //温度计算结束，将ADC基准调为2V
	ADCON1&=0x7F; //令ADEN=0		
	ADCLDO=0xA0; //使能芯片内置ADC基准，输出2.0V
	ADCON1|=0x80; //令ADEN=1，重新打开ADC
	}
