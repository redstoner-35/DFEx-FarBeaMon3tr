#include "ht32.h"
#include "delay.h"
#include "GUI.h"
#include "ADC.h"
#include "Pindefs.h"
#include <math.h>

//电压电流选择引脚自动定义
#define TVSEL_IOB STRCAT2(GPIO_P,TVSEL_IOBank)
#define TVSEL_IOG STRCAT2(HT_GPIO,TVSEL_IOBank)
#define TVSEL_IOP STRCAT2(GPIO_PIN_,TVSEL_IOPinNum) 


//变量
static bool ADCEOCFlag=false;
ADCOutTypeDef ADCO;

//ADC结束转换回调
void ADC_EOC_interrupt_Callback(void)
  {
	ADC_ClearIntPendingBit(HT_ADC0, ADC_FLAG_CYCLE_EOC);//清除中断
  ADCEOCFlag=true;
	}

//ADC测量函数
bool ADC_GetResult(void)
  {
	float Rt;
  int retry,avgcount,i;
	unsigned int ADCResult[3]={0};
  float buf,Comp;
	//开始测量
	for(avgcount=0;avgcount<ADCAvg;avgcount++)
		{
		//初始化变量
		retry=0;
		ADCEOCFlag=false;
		//设置单次转换的组别后启动转换
		ADC_RegularGroupConfig(HT_ADC0,DISCONTINUOUS_MODE, 3, 0);//单次触发模式,一次完成3个数据的转换
		ADC_RegularChannelConfig(HT_ADC0, _ISenseOut_Ch,ISenseOut_IOPN, 0);
		ADC_RegularChannelConfig(HT_ADC0, _ISenseREF_Ch,ISenseREF_IOPN, 0);	
		ADC_RegularChannelConfig(HT_ADC0, _TempVBatt_Ch,TempVBatt_IOPN, 0);	
	  ADC_SoftwareStartConvCmd(HT_ADC0, ENABLE); 
		//等待转换结束
		while(!ADCEOCFlag)
		  {
			retry++;//重试次数+1
			delay_us(2);
			if(retry>=ADCConvTimeOut)return false;//转换超时
			}
		//获取数据
		for(i=0;i<3;i++)ADCResult[i]+=HT_ADC0->DR[i]&0x0FFF;//采集四个规则通道的结果		
		}
	//转换完毕，求平均
  for(i=0;i<3;i++)ADCResult[i]/=avgcount;
	//计算电池电流
	Comp=(float)ADCResult[ISenseREF_IOPN]*(VREF/(float)4096);//将AD值转换为基准电压
	ADCO.IVREF=Comp; //记录基准电压
	buf=((float)ADCResult[ISenseOut_IOPN]*(VREF/(float)4096))-Comp; //VINA=IOUT-IREF
	buf/=SenseAmpGain; //Vshunt=VINA/Sense
	ADCO.Ibatt=(buf*(-1000))/SenseShuntmOhm; //IBatt=Vshunt/Rshunt(因为硬件上检流NP对调了所以输出电流要反相)
	ADCO.Ibatt*=1.012; //实际电流是1.2%误差
	//根据采样结果更新温度和电压
  buf=(float)ADCResult[TempVBatt_IOPN]*(VREF/(float)4096); //得到引脚对应电压
  if(GPIO_ReadOutBit(TVSEL_IOG,TVSEL_IOP)==SET)		
		{
		//当前测量结果为电压
		Comp=(float)VsenseLowRes/(float)(VsenseUpRes+VsenseLowRes); //计算比例值
		ADCO.Vbatt=buf/Comp; //原始电压值/上下桥分压系数=电池电压
		GPIO_ClearOutBits(TVSEL_IOG,TVSEL_IOP); //令TVSEL=0，选择温度测量	
		}
	else
		{
		//当前测量值为温度
		Rt=((float)NTCUpperResValueK*buf)/(VREF-buf);//得到NTC+单片机IO导通电阻的传感器温度值
		buf=1/((1/(273.15+(float)NTCT0))+log(Rt/(float)NTCT0ResK)/(float)NTCBValue);//计算出温度
		buf-=273.15;//减去开氏温标常数变为摄氏度
		buf+=(float)NTCTRIM;//加上修正值	
		if(buf<(-40)||buf>125)	//温度传感器异常
			{
			ADCO.IsNTCOK=false;
			ADCO.Systemp=0;
			}
		else  //温度传感器正常
			{
			ADCO.IsNTCOK=true;
			ADCO.Systemp=buf;
			}
		GPIO_SetOutBits(TVSEL_IOG,TVSEL_IOP); //令TVSEL=0，选择温度测量	
		}
	//测量完毕，返回结果
	return true;
	}		
	
//内部ADC初始化
void InternalADC_Init(void)
  {
	 int i;
	 ShowPostInfo(5,"配置ADC模块","02",Msg_Statu);
	 //将LED Vf测量引脚和温度引脚配置为AD模式
	 AFIO_GPxConfig(GPIO_PA, ISenseOut_IOP,AFIO_FUN_ADC0);
	 AFIO_GPxConfig(GPIO_PA,ISenseREF_IOP,AFIO_FUN_ADC0);
	 AFIO_GPxConfig(GPIO_PA,TempVBatt_IOP,AFIO_FUN_ADC0);
	 //将ADC输入选择引脚配置为GPIO		
	 AFIO_GPxConfig(TVSEL_IOB,TVSEL_IOP, AFIO_FUN_GPIO);
   GPIO_DirectionConfig(TVSEL_IOG,TVSEL_IOP,GPIO_DIR_OUT);//配置为输出 
	 GPIO_DriveConfig(TVSEL_IOG,TVSEL_IOP,GPIO_DV_16MA);	//设置为16mA最大输出		
	 GPIO_ClearOutBits(TVSEL_IOG,TVSEL_IOP); //令TVSEL=0，选择温度测量
   //设置转换组别类型，转换时间，转换模式      
	 CKCU_SetADCnPrescaler(CKCU_ADCPRE_ADC0, CKCU_ADCPRE_DIV16);//ADC时钟为主时钟16分频=3MHz                                               
   ADC_RegularGroupConfig(HT_ADC0,DISCONTINUOUS_MODE, 3, 0);//单次触发模式,一次完成3个数据的转换
   ADC_SamplingTimeConfig(HT_ADC0,25); //采样时间（25个ADC时钟）
   ADC_RegularTrigConfig(HT_ADC0, ADC_TRIG_SOFTWARE);//软件启动	
	 //设置单次转换的组别
   ADC_RegularChannelConfig(HT_ADC0, _ISenseOut_Ch,ISenseOut_IOPN, 0);
   ADC_RegularChannelConfig(HT_ADC0, _ISenseREF_Ch,ISenseREF_IOPN, 0);	
	 ADC_RegularChannelConfig(HT_ADC0, _TempVBatt_Ch,TempVBatt_IOPN, 0);	
	 //启用ADC中断    
   ADC_IntConfig(HT_ADC0,ADC_INT_CYCLE_EOC,ENABLE);                                                                            
   NVIC_EnableIRQ(ADC0_IRQn);
	 //复位传感数据并启用ADC
	 ADCO.Ibatt=0;
	 ADCO.IsNTCOK=false;
	 ADCO.Systemp=0;
	 ADCO.Vbatt=0;
	 ADC_Cmd(HT_ADC0, ENABLE);
	 //触发ADC转换
	 ShowPostInfo(8,"检查NTC","03",Msg_Statu);
   for(i=0;i<5;i++)if(!ADC_GetResult())
		{
		ShowPostInfo(8,"ADC转换异常","E0",Msg_Fault);
		SelfTestErrorHandler();
		}
	 //基准电压故障
	 if(ADCO.IVREF<1.2||ADCO.IVREF>1.3)
		{
		ShowPostInfo(8,"IREF电压故障","8A",Msg_Fault);
		SelfTestErrorHandler();
		
		}
	 //监测温度状态
	 if(!ADCO.IsNTCOK)
	  {
		ShowPostInfo(8,"热敏电阻故障","E1",Msg_Fault);
		SelfTestErrorHandler();
		}
	}
