#include "ht32.h"
#include "delay.h"
#include "GUI.h"
#include "ADC.h"
#include "Pindefs.h"
#include <math.h>

//��ѹ����ѡ�������Զ�����
#define TVSEL_IOB STRCAT2(GPIO_P,TVSEL_IOBank)
#define TVSEL_IOG STRCAT2(HT_GPIO,TVSEL_IOBank)
#define TVSEL_IOP STRCAT2(GPIO_PIN_,TVSEL_IOPinNum) 


//����
static bool ADCEOCFlag=false;
ADCOutTypeDef ADCO;

//ADC����ת���ص�
void ADC_EOC_interrupt_Callback(void)
  {
	ADC_ClearIntPendingBit(HT_ADC0, ADC_FLAG_CYCLE_EOC);//����ж�
  ADCEOCFlag=true;
	}

//ADC��������
bool ADC_GetResult(void)
  {
	float Rt;
  int retry,avgcount,i;
	unsigned int ADCResult[3]={0};
  float buf,Comp;
	//��ʼ����
	for(avgcount=0;avgcount<ADCAvg;avgcount++)
		{
		//��ʼ������
		retry=0;
		ADCEOCFlag=false;
		//���õ���ת������������ת��
		ADC_RegularGroupConfig(HT_ADC0,DISCONTINUOUS_MODE, 3, 0);//���δ���ģʽ,һ�����3�����ݵ�ת��
		ADC_RegularChannelConfig(HT_ADC0, _ISenseOut_Ch,ISenseOut_IOPN, 0);
		ADC_RegularChannelConfig(HT_ADC0, _ISenseREF_Ch,ISenseREF_IOPN, 0);	
		ADC_RegularChannelConfig(HT_ADC0, _TempVBatt_Ch,TempVBatt_IOPN, 0);	
	  ADC_SoftwareStartConvCmd(HT_ADC0, ENABLE); 
		//�ȴ�ת������
		while(!ADCEOCFlag)
		  {
			retry++;//���Դ���+1
			delay_us(2);
			if(retry>=ADCConvTimeOut)return false;//ת����ʱ
			}
		//��ȡ����
		for(i=0;i<3;i++)ADCResult[i]+=HT_ADC0->DR[i]&0x0FFF;//�ɼ��ĸ�����ͨ���Ľ��		
		}
	//ת����ϣ���ƽ��
  for(i=0;i<3;i++)ADCResult[i]/=avgcount;
	//�����ص���
	Comp=(float)ADCResult[ISenseREF_IOPN]*(VREF/(float)4096);//��ADֵת��Ϊ��׼��ѹ
	ADCO.IVREF=Comp; //��¼��׼��ѹ
	buf=((float)ADCResult[ISenseOut_IOPN]*(VREF/(float)4096))-Comp; //VINA=IOUT-IREF
	buf/=SenseAmpGain; //Vshunt=VINA/Sense
	ADCO.Ibatt=(buf*(-1000))/SenseShuntmOhm; //IBatt=Vshunt/Rshunt(��ΪӲ���ϼ���NP�Ե��������������Ҫ����)
	ADCO.Ibatt*=1.012; //ʵ�ʵ�����1.2%���
	//���ݲ�����������¶Ⱥ͵�ѹ
  buf=(float)ADCResult[TempVBatt_IOPN]*(VREF/(float)4096); //�õ����Ŷ�Ӧ��ѹ
  if(GPIO_ReadOutBit(TVSEL_IOG,TVSEL_IOP)==SET)		
		{
		//��ǰ�������Ϊ��ѹ
		Comp=(float)VsenseLowRes/(float)(VsenseUpRes+VsenseLowRes); //�������ֵ
		ADCO.Vbatt=buf/Comp; //ԭʼ��ѹֵ/�����ŷ�ѹϵ��=��ص�ѹ
		GPIO_ClearOutBits(TVSEL_IOG,TVSEL_IOP); //��TVSEL=0��ѡ���¶Ȳ���	
		}
	else
		{
		//��ǰ����ֵΪ�¶�
		Rt=((float)NTCUpperResValueK*buf)/(VREF-buf);//�õ�NTC+��Ƭ��IO��ͨ����Ĵ������¶�ֵ
		buf=1/((1/(273.15+(float)NTCT0))+log(Rt/(float)NTCT0ResK)/(float)NTCBValue);//������¶�
		buf-=273.15;//��ȥ�����±곣����Ϊ���϶�
		buf+=(float)NTCTRIM;//��������ֵ	
		if(buf<(-40)||buf>125)	//�¶ȴ������쳣
			{
			ADCO.IsNTCOK=false;
			ADCO.Systemp=0;
			}
		else  //�¶ȴ���������
			{
			ADCO.IsNTCOK=true;
			ADCO.Systemp=buf;
			}
		GPIO_SetOutBits(TVSEL_IOG,TVSEL_IOP); //��TVSEL=0��ѡ���¶Ȳ���	
		}
	//������ϣ����ؽ��
	return true;
	}		
	
//�ڲ�ADC��ʼ��
void InternalADC_Init(void)
  {
	 int i;
	 ShowPostInfo(5,"����ADCģ��","02",Msg_Statu);
	 //��LED Vf�������ź��¶���������ΪADģʽ
	 AFIO_GPxConfig(GPIO_PA, ISenseOut_IOP,AFIO_FUN_ADC0);
	 AFIO_GPxConfig(GPIO_PA,ISenseREF_IOP,AFIO_FUN_ADC0);
	 AFIO_GPxConfig(GPIO_PA,TempVBatt_IOP,AFIO_FUN_ADC0);
	 //��ADC����ѡ����������ΪGPIO		
	 AFIO_GPxConfig(TVSEL_IOB,TVSEL_IOP, AFIO_FUN_GPIO);
   GPIO_DirectionConfig(TVSEL_IOG,TVSEL_IOP,GPIO_DIR_OUT);//����Ϊ��� 
	 GPIO_DriveConfig(TVSEL_IOG,TVSEL_IOP,GPIO_DV_16MA);	//����Ϊ16mA������		
	 GPIO_ClearOutBits(TVSEL_IOG,TVSEL_IOP); //��TVSEL=0��ѡ���¶Ȳ���
   //����ת��������ͣ�ת��ʱ�䣬ת��ģʽ      
	 CKCU_SetADCnPrescaler(CKCU_ADCPRE_ADC0, CKCU_ADCPRE_DIV16);//ADCʱ��Ϊ��ʱ��16��Ƶ=3MHz                                               
   ADC_RegularGroupConfig(HT_ADC0,DISCONTINUOUS_MODE, 3, 0);//���δ���ģʽ,һ�����3�����ݵ�ת��
   ADC_SamplingTimeConfig(HT_ADC0,25); //����ʱ�䣨25��ADCʱ�ӣ�
   ADC_RegularTrigConfig(HT_ADC0, ADC_TRIG_SOFTWARE);//�������	
	 //���õ���ת�������
   ADC_RegularChannelConfig(HT_ADC0, _ISenseOut_Ch,ISenseOut_IOPN, 0);
   ADC_RegularChannelConfig(HT_ADC0, _ISenseREF_Ch,ISenseREF_IOPN, 0);	
	 ADC_RegularChannelConfig(HT_ADC0, _TempVBatt_Ch,TempVBatt_IOPN, 0);	
	 //����ADC�ж�    
   ADC_IntConfig(HT_ADC0,ADC_INT_CYCLE_EOC,ENABLE);                                                                            
   NVIC_EnableIRQ(ADC0_IRQn);
	 //��λ�������ݲ�����ADC
	 ADCO.Ibatt=0;
	 ADCO.IsNTCOK=false;
	 ADCO.Systemp=0;
	 ADCO.Vbatt=0;
	 ADC_Cmd(HT_ADC0, ENABLE);
	 //����ADCת��
	 ShowPostInfo(8,"���NTC","03",Msg_Statu);
   for(i=0;i<5;i++)if(!ADC_GetResult())
		{
		ShowPostInfo(8,"ADCת���쳣","E0",Msg_Fault);
		SelfTestErrorHandler();
		}
	 //��׼��ѹ����
	 if(ADCO.IVREF<1.2||ADCO.IVREF>1.3)
		{
		ShowPostInfo(8,"IREF��ѹ����","8A",Msg_Fault);
		SelfTestErrorHandler();
		
		}
	 //����¶�״̬
	 if(!ADCO.IsNTCOK)
	  {
		ShowPostInfo(8,"�����������","E1",Msg_Fault);
		SelfTestErrorHandler();
		}
	}
