#ifndef _ADC_
#define _ADC_

#include "stdbool.h"

//ת������ṹ��
typedef struct
	{
	bool IsNTCOK; //NTC�Ƿ�����
	float Systemp; //ϵͳ�¶�
  float Vbatt; //��ص�ѹ
	float Ibatt; //��ص���
	float IVREF; //������ⲿ�ֵ�VREF����
	}ADCOutTypeDef;

//ADC��������
#define VREF 2.50 //��׼��ѹ
#define ADCAvg 5  //ADCƽ������
#define ADCConvTimeOut 2000 //ADCת����ʱʱ��(��λ2mS)
#define MinimumCurrentFactor 0.05 //��־ϵͳ��ͳ�Ƶĵ���	
	
//�¶ȵ�ѹ�͵����������	
#define SenseAmpGain 50 //�����Ŵ��������棬��λΪ(V/V)	
#define SenseShuntmOhm 1.0 //����������ֵ(mR)	
#define VsenseUpRes 100 //
#define VsenseLowRes 10 //��ص�ѹ�����ķ�ѹ������ֵ(K)	
#define NTCUpperResValueK	10 //NTC��������������ֵ(K)	
#define NTCT0 25 //NTC��׼��ֵ�궨�¶�
#define NTCT0ResK 15 //NTC�ڱ궨�¶��µ���ֵ(K)
#define NTCBValue 3450 //NTC Bֵ
#define NTCTRIM -3 //�¶�����ֵ(��)
	
#if (BATTCOUNT == 4)	
	
#define BattCellCount 4
	
#elif (BATTCOUNT == 3)

#define BattCellCount 3

#elif (BATTCOUNT == 2)

#define BattCellCount 2

#else
  #error "Undefined Battery Cell Count"
#endif
	
/*������Զ�Define������ADC��IO��ADCͨ�����壬�������޸ģ�*/
#define ISenseOut_IOP STRCAT2(AFIO_PIN_,ISenseOut_IOPN)
#define ISenseREF_IOP STRCAT2(AFIO_PIN_,ISenseREF_IOPN) 
#define TempVBatt_IOP STRCAT2(AFIO_PIN_,TempVBatt_IOPN) 

#define _ISenseOut_Ch STRCAT2(ADC_CH_,ISenseOut_IOPN)
#define _ISenseREF_Ch STRCAT2(ADC_CH_,ISenseREF_IOPN)
#define _TempVBatt_Ch STRCAT2(ADC_CH_,TempVBatt_IOPN)

//�ⲿ�ο�
extern ADCOutTypeDef ADCO;

//����
bool ADC_GetResult(void);
void InternalADC_Init(void); //ADC��ʼ��

#endif
