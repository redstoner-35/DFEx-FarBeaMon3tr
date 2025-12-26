/****************************************************************************/
/** \file ADCCfg.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition
/** \Description 这个文件负责驱动系统的ADC完成系统的各项模拟量遥测任务，包含异步
非阻塞转换ADC引擎的实现
**
**	History:

				2025年12月26日 10:05 移除掉浪费空间的GPIO_Writebit()函数实现的电池和输
														 出检测GPIO置零流程，改为声明sfr直接写sbit节约ROM
														 空间。
														 
				2025年12月20日 Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "cms8s6990.h"
#include "PinDefs.h"
#include "ADCCfg.h"
#include "delay.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

//ADC基准电压和特殊基准通道定义
#define ADCVREF 2.00 //ADC片内基准LDO的电压
#define ADC_INTVREFCh 31 //ADC连通到片内带隙基准的特殊通道定义	
#define ADCBGVREF 1.20 //ADC特殊通道带隙基准的电压	
#define ADCWaitChannelSelTime 160 //ADC等待通道选通的延时	
	
//ADC寄存器操作宏定义	
#define ADC_StartConv() ADCON0|=0x02 //ADC启动转换
#define ADC_GetIfStillConv()	ADCON0&0x02  //检查ADC是否仍然在转换需要继续等
#define ADC_ReadConvResult()	(ADRESL|(ADRESH<<8)) //读取ADC转换的寄存器结果
#define ADC_EnableCmd() ADCON1|=0x80  //使能ADC IP
#define ADC_DisableCmd() ADCON1&=0x7F  //关闭ADC IP	
#define ADC_SetVREFReg(IsVDD) ADCLDO=(!IsVDD?0xA0:0x00) //设置基准
#define ADC_IsUsingIVREF() ADCLDO&0x80 //检测ADC是否在使用片内基准	
#define ADC_CheckIfChInvalid(Ch) (Ch<0||(Ch>22&&Ch<ADC_INTVREFCh)) //检查通道参数是否合法	
	
//ADC外部采集的参数配置
#define VoutUpperResK 680
#define VoutLowerResK 56 //输出检测分压的上下拉电阻
#define VBattUpperResK 680
#define VBattLowerResK 100 //电池检测分压的上下拉电阻
#define NTCUpperResValueK 330 //NTC热敏电阻的上拉阻值
	
//ADC异步引擎配置
#define ADCConvertQueueDepth 5 //ADC转换任务队列深度	
#define ADCAverageCount 10 //ADC对于每个转换任务的平均次数		
	
/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

//ADC异步引所需的枚举值
typedef enum
	{
	ADC_SubmitQueue, //提交转换队列	
  ADC_SubmitChFromQueue, //向ADC转换线程提交队列内的任务
	ADC_WaitMissionDone, //等待任务完成
	ADC_ConvertComplete //转换完毕	
	}ADCAsyncStateDef; //ADC异步转换状态机处理

typedef struct
	{
	long avgbuf;
	int Count;
	char Ch;
	bool IsMissionProcessing; //是否正在处理任务
	}ADCConvertTemp;

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/
bit IsNotAllowAsync;	 //是否允许ADC引擎运行在异步模式
ADCResultStrDef Data;	 //ADC结果输出

/****************************************************************************/
/*	Local variable and special Register definitions('static')

Note: 以下函数为ADC异步转换引擎实现功能所需的内部处理函数以及所需的内部全局变
量。请勿在除了本文件内的其他任何地方调用，否则会导致ADC引擎工作异常！	
****************************************************************************/
static xdata ADCConvertTemp ADCTemp;
static ADCAsyncStateDef ADCState;	
static xdata char ADCConvertQueue[ADCConvertQueueDepth];	

/****************************************************************************/
/*	Local special function reg definitions('sbit' or 'sfr')
****************************************************************************/	
sbit NTCPullUpEN=NTCENIOP^NTCENIOx; //NTC Enable
sbit VBATIN=VBATInputIOP^VBATInputIOx; //Battery Input Feedback
sbit VOUTFB=VOUTFBIOP^VOUTFBIOx;  //VOUT FB	
	
/****************************************************************************/
/*	Local constant value definitions('static')	
****************************************************************************/

//ADC异步转换引擎的转换队列声明
code char ADCChQueue[ADCConvertQueueDepth]=
	{
	ADC_INTVREFCh, //先转换VREF
	NTCInputAIN,//然后转换温度	
	OPFBAIN, //FB注入恒流运放的输出电压
	VBATInputAIN, //电池电压
	VOUTFBAIN //最后转换输出电压
	};	
	
/****************************************************************************/
/*	Local function implantation('static')
****************************************************************************/	

//向ADC提交任务	
static void ADC_SubmitMisson(char Ch)	
	{
	unsigned char i=ADCWaitChannelSelTime;
	//检查传入的通道参数是否合法
	if(ADCTemp.IsMissionProcessing)return;
	if(ADC_CheckIfChInvalid(Ch))return; 
	//进行初始化
	ADCTemp.avgbuf=0;
	ADCTemp.Count=0;
	ADCTemp.Ch=Ch;
	ADCTemp.IsMissionProcessing=true;
	//配置ADC通道		
	if(Ch&0x10)ADCON0|=0x80;
	else ADCON0&=0x7F; //设置ADCHS[4]
	ADCON1&=0xF0;
	ADCON1|=(Ch&0x0F); //设置ADCHS[3:0]					
	//启动转换
	while(--i);  			//延时等待通道选通后开始采样
	ADC_StartConv();
	}	

//读取数据
static int ADC_ReadBackResult(int *Result,char *Queue)	
	{
	//ADC未完成本次转换
	if(ADC_GetIfStillConv())return 0; 
	//收取结果
	ADCTemp.Count++; //数值+1
	ADCTemp.avgbuf+=(long)ADC_ReadConvResult(); //从AD寄存器收取结果并进行平均累加
	if(ADCTemp.Count<ADCAverageCount) 
		{
		ADC_StartConv();
		return 0;//平均次数未到，重新启动ADC进行新一轮的处理
		}
	//完成转换，返回结果并准备可以提交新的任务
	ADCTemp.avgbuf/=(long)ADCAverageCount;
	*Result=(int)ADCTemp.avgbuf; //返回结果
	*Queue=ADCTemp.Ch; //返回转换的队列
	ADCTemp.IsMissionProcessing=false; //任务已处理完毕
  return 1;	
	}

//ADC设置电压参考	
static void ADC_SetVREF(bit IsUsingVDD)
	{
	ADC_DisableCmd(); //转换ADC基准需要暂时关闭ADC	
	_nop_();
	ADC_SetVREFReg(IsUsingVDD); //设置芯片内部基准
	_nop_();
	ADC_EnableCmd(); //基准切换完毕，重新启动
	}
	
//转换完毕后写输出引擎
int	CalcNTCTemp(bool *IsNTCOK,unsigned long NTCRes); //函数声明		

static void ADC_WriteOutputBuf(int ADCResult,char Ch)
	{
	float Buf,Rt,Vadc;
	unsigned long NTCRES;
	//进行ADC
	Rt=ADC_IsUsingIVREF()?ADCVREF:Data.MCUVDD; //根据基准设置得到AD当前的基准电压
	Vadc=(float)ADCResult*(Rt/(float)4096);//将AD值转换为原始电压
	//状态机
  switch(Ch)
		{
		//计算FB注入运放的输出电压
		case OPFBAIN:
			Data.FBInjectVolt=Vadc; 
		  break;
		//计算参考电压
		case ADC_INTVREFCh:
			Data.MCUVDD=ADCBGVREF*(float)4096/(float)ADCResult; //计算出MCUVDD(VREF)
		  break; 
		//计算电池电压
		case VBATInputAIN:
			#define VBatTotalResistor (VBattLowerResK+VBattUpperResK)
			Buf=(float)VBattLowerResK/(float)VBatTotalResistor;//计算出分压电阻的系数
			Data.RawBattVolt=Vadc/Buf; //根据分压系数反推出电池电压
		  Data.BatteryVoltage=Data.RawBattVolt/(float)3; //将3节电池的总电压转换为单节电池的电压
		  #undef VBatTotalResistor
		  break;
	  //计算输出电压
		case VOUTFBAIN:		
			#define VoutTotalResistor (VoutLowerResK+VoutUpperResK)
			Buf=(float)VoutLowerResK/(float)VoutTotalResistor;//计算出分压电阻的系数
			Data.OutputVoltage=Vadc/Buf; //根据分压系数反推出DCDC输出电压
		  #undef VoutTotalResistor
		  break;
    //计算温度
		case NTCInputAIN:
			Rt=((float)NTCUpperResValueK*Vadc)/(Data.MCUVDD-Vadc);//得到NTC+单片机IO导通电阻的传感器阻值
			Rt*=1000; //将阻值从K欧转为Ω
			NTCRES=(unsigned long)Rt; //取整
			Data.Systemp=CalcNTCTemp(&Data.IsNTCOK,NTCRES); //计算温度
			break;
		}
  }

//ADC异步引擎的主处理函数
static void ADCEngineHandler(void)
	{
	int result;
	char Ch,i;
	//转换循环
  do
		{
		if(ADCState==ADC_ConvertComplete)ADCState=ADC_SubmitQueue; //如果一轮转换完成则重新开始
		switch(ADCState)
			{		
			//开始提交转换队列
			case ADC_SubmitQueue: 
				ADC_SetVREF(1); //每次提交队列之前，设置基准使用MCUVDD来转换温度和MCUVDD电压
				for(i=0;i<ADCConvertQueueDepth;i++)ADCConvertQueue[i]=ADCChQueue[i]; //把转换队列里面的数据复制过去
			  ADCState=ADC_SubmitChFromQueue;
			  break;
		  //向ADC转换线程提交任务
			case ADC_SubmitChFromQueue: 
			  i=0;
			  while(i<ADCConvertQueueDepth)
					{
					if(!ADC_CheckIfChInvalid(ADCConvertQueue[i]))break; //找到队列中未完成的合法转换项目
					i++;
					}
				//有转换项目未完成
				if(i<ADCConvertQueueDepth)	
					{
					Ch=ADCConvertQueue[i]; //检测目标的通道值
          if(Ch==VBATInputAIN||Ch==VOUTFBAIN)ADC_SetVREF(0); //电池和输出电压转换使用内部精密通道
					ADC_SubmitMisson(Ch); //提交项目
					ADCState=ADC_WaitMissionDone;
					}
				//所有转换已完成，跳转到完成阶段
  			else ADCState=ADC_ConvertComplete;		
			  break;	
			//提交线程任务后等待本次任务完成
      case ADC_WaitMissionDone:
          if(!ADC_ReadBackResult(&result,&Ch))break; //尝试读取结果，转换未完成则继续
			    Data.RandADResult=result;     //将随机取到的result值写进去
			    ADC_WriteOutputBuf(result,Ch);
			    for(i=0;i<ADCConvertQueueDepth;i++)if(ADCConvertQueue[i]==Ch)ADCConvertQueue[i]=-2; //将当前已经完成转换的任务通道设置为-2标记转换完毕
			    ADCState=ADC_SubmitChFromQueue; //重新回到提交任务的阶段
			    break;
			//所有任务已完成
			case ADC_ConvertComplete:break;
			//其余任何非法状态跳转到初始阶段
			default:ADCState=ADC_SubmitQueue;
			}
		}
	while(IsNotAllowAsync&&ADCState!=ADC_ConvertComplete);
	}	
	
/****************************************************************************/
/* Global Function implementation
	
注意：以下函数为ADC异步转换引擎以及ADC的初始化和除能操作和驱动引擎获取外部通
道的电压数据所需的外部函数调用。您可以在初始化阶段和主函数内调用以下区
域的函数对ADC进行初始化和除能操作，以及启动引擎对ADC进行异步采样。
****************************************************************************/	
	
//进行数据获取	
void SystemTelemHandler(void)
	{
  //调用ADC异步引擎
	ADCEngineHandler();
	}	
	
//复位ADC异步引擎
static void ResetADCAsyncEngine(void)	
	{
	unsigned char i;	
	for(i=0;i<ADCConvertQueueDepth;i++)ADCConvertQueue[i]=-2;	
	ADCState=ADC_SubmitQueue;
	ADCTemp.avgbuf=0;
	ADCTemp.Count=0;
	ADCTemp.Ch=0;
	ADCTemp.IsMissionProcessing=false;
	IsNotAllowAsync=1; //初始化时禁止异步功能	
	}

//关闭ADC
void ADC_DeInit(void)
	{
	GPIOCfgDef ADCDeInitCfg;	
	//配置寄存器关闭ADC
	ADCON1=0x00; //关闭ADC
	ADCLDO=0x00; //关闭片内基准
	
	//清空队列并复位异步引擎
  ResetADCAsyncEngine();
	//将需要禁用的ADC输入GPIO设置为普通GPIO模式
	GPIO_SetMUXMode(VOUTFBIOG,VOUTFBIOx,GPIO_AF_GPIO);
	GPIO_SetMUXMode(VBATInputIOG,VBATInputIOx,GPIO_AF_GPIO);
	//将需要禁用的ADC输入设置为推挽输出
	ADCDeInitCfg.Mode=GPIO_Out_PP;
  ADCDeInitCfg.Slew=GPIO_Slow_Slew;		
	ADCDeInitCfg.DRVCurrent=GPIO_Low_Current; //配置为低电流推挽输出
	
  GPIO_ConfigGPIOMode(VOUTFBIOG,GPIOMask(VOUTFBIOG),&ADCDeInitCfg); 
	GPIO_ConfigGPIOMode(VBATInputIOG,GPIOMask(VBATInputIOx),&ADCDeInitCfg); 
	//将需要禁用的ADC输入GPIO全部输出0
  VBATIN=0;
	VOUTFB=0;
	//令NTC偏压供电输出=0关闭NTC电源
	NTCPullUpEN=0;
	}

//ADC初始化
void ADC_Init(void)
	{
	GPIOCfgDef ADCInitCfg;
	//初始化GPIO
	ADCInitCfg.Mode=GPIO_Input_Floating;
  ADCInitCfg.Slew=GPIO_Slow_Slew;		
	ADCInitCfg.DRVCurrent=GPIO_Low_Current; //配置为浮空输入	

  GPIO_ConfigGPIOMode(VOUTFBIOG,GPIOMask(VOUTFBIOG),&ADCInitCfg); 
	GPIO_ConfigGPIOMode(VBATInputIOG,GPIOMask(VBATInputIOx),&ADCInitCfg); 
	GPIO_ConfigGPIOMode(NTCInputIOG,GPIOMask(NTCInputIOx),&ADCInitCfg); 	//将对应的IO设置为指定的模式
	GPIO_ConfigGPIOMode(OPFBIOG,GPIOMask(OPFBIOx),&ADCInitCfg); //运放反馈也设为浮空输入
	
	GPIO_SetMUXMode(OPFBIOG,OPFBIOx,GPIO_AF_Analog);
  GPIO_SetMUXMode(NTCInputIOG,NTCInputIOx,GPIO_AF_Analog);
	GPIO_SetMUXMode(VOUTFBIOG,VOUTFBIOx,GPIO_AF_Analog);
	GPIO_SetMUXMode(VBATInputIOG,VBATInputIOx,GPIO_AF_Analog); //将GPIO复用设置为模拟输入
	//配置并打开NTC分压电阻的供电
	ADCInitCfg.Mode=GPIO_Out_PP;
	ADCInitCfg.DRVCurrent=GPIO_High_Current; 	 //大电流推挽输出
		
	GPIO_SetMUXMode(NTCENIOG,NTCENIOx,GPIO_AF_GPIO);
  GPIO_ConfigGPIOMode(NTCENIOG,GPIOMask(NTCENIOx),&ADCInitCfg);		
  NTCPullUpEN=1; //令供电输出=1打开NTC电源
	
	//配置ADC
	ADCON0=0x40; //AN31=内部1.2V基准，结果右对齐
	ADCON1=0x60; //Fadc=Fsys/128=375KHz
	ADCON2=0x00; //关闭ADC硬件触发功能，使用软件命令启动ADC
	ADCMPC=0x00; //关闭ADC比较器触发刹车功能
	ADDLYL=0x00; //将ADC硬件启动触发延时设置为0
	ADCMPH=0x0F;
	ADCMPL=0xFF; //ADC比较器默认值设置为0x0FFF
  ADCLDO=0xA0; //使能芯片内置ADC基准，输出2.0V
	
	//初始化异步ADC引擎
	ResetADCAsyncEngine();
	//ADC配置完毕，使能ADC模块
	ADC_EnableCmd(); 
	}
/*************************  End Of File  ***********************/
