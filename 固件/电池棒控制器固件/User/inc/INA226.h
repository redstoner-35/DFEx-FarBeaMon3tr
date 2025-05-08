#ifndef _INA226_
#define _INA226_

//�궨��
#define BusVoltLSB 1.25 //���ߵ�ѹ��1LSB��ֵ����λmV
#define MaxmiumCurrent 15 //Ԥ�ڵ���������15A��
#define CurrentLSB (double)MaxmiumCurrent/PMBUS_2NPowCalc(15) //LSB=��������������2��15�η�
#define PowerLSB (float)25*CurrentLSB //���ʶ�����LSB��Ϊ25���ĵ���LSB

//�ڲ�����
#include <stdbool.h>

typedef enum
	{
	INA226_AvgCount_1,
  INA226_AvgCount_4,
  INA226_AvgCount_16,
  INA226_AvgCount_64,
  INA226_AvgCount_128,
  INA226_AvgCount_256,
  INA226_AvgCount_512,
  INA226_AvgCount_1024   
	}INA226AvgCountDef;

typedef enum
	{
	INA226_Conv_140uS,
  INA226_Conv_204US,
  INA226_Conv_332US,
  INA226_Conv_588US,
  INA226_Conv_1100US,
  INA226_Conv_2116US,
  INA226_Conv_4156US,
  INA226_Conv_8244US,
	}INA226ConvTimeDef;

//ģʽö��
typedef enum
{
INA226_PowerDown,
INA226_Trig_BCurrent,
INA226_Trig_BVoltage,
INA226_Trig_Both,	
INA226_ADC_Off,
INA226_Cont_BCurrent,
INA226_Cont_BVoltage,
INA226_Cont_Both	
}INA226ModeDef;

//��ʼ���������
typedef enum
{
A226_Init_OK=0,
A226_Error_SMBUS_NACK=1,
A226_Error_CalibrationReg=2,
A226_Error_ProgramCalReg=3,
A226_Error_ProgramReg=4,
A226_Error_SetAlertCfg=5,
A226_Error_NotGenuineDevice=6
}INA226InitStatDef;

//��������
typedef enum
{
A226_AlertDisable=0, //�ر����о���
A226_EnableOCP=0x8000,  //�����������߱���[SOL=1]
A226_EnableUCP=0x4000,  //�����������ͱ���[SUL=1]
A226_EnableOVP=0x2000,  //������ѹ���߱���[BOL=1]
A226_EnableUVP=0x1000,  //������ѹ���ͱ���[BUL=1]
A226_EnableOPP=0x800, //���������ʱ���[BUL=0]
}INA226AlertDef;

//��ȡ�����õĽṹ��
typedef struct
{
float BusVolt;
float BusCurrent;
float BusPower;
}INADoutSreDef;

//��ʼ��226�õĽṹ��
typedef struct
{
float ShuntValue;//�����������ֵ����λΪ��ŷ��mR��
INA226ModeDef ConvMode;//ת��ģʽ
INA226ConvTimeDef VBUSConvTime;
INA226ConvTimeDef IBUSConvTime;	  //���ߵ�ѹ�����Ĳ���ʱ��
INA226AvgCountDef AvgCount; //ƽ��ʱ��	
bool IsEnableAlertLatch; //�Ƿ����������ŵ�����
bool IsAlertPinInverted; //�Ƿ��򱨾�����ļ���
INA226AlertDef AlertConfig; //�澯����
}INAinitStrdef;

//����
INA226InitStatDef INA226_INIT(INAinitStrdef * INAConf);
bool INA226_GetBusInformation(INADoutSreDef *INADout);
bool INA226_SetAlertRegister(unsigned int Value);
bool INA226_QueueIfGaugeCanReady(void);

#endif
