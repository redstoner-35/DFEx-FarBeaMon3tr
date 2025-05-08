#include "Config.h"
#include "INA226.h"
#include "lcd.h"
#include "GUI.h"
#include "Config.h"
#include "delay.h"

//�ڲ��ַ���
const char *A226ErrorStr[]=
{
"SMBUS_NACK",
"CalReg_OVF",
"Write_Calibration",
"Write_Config",	
"Write_AlertCfg",
"NotGenuineDevice"
};

//ȫ�ֱ������Ƿ����߾��Ȳ���ģ��
bool IsEnableHPGauge=false;

void HPPowerGuage_Start(void)
	{
	INAinitStrdef INAConf;
	extern bool IsEnable17AMode;
	INA226InitStatDef Result;
	INADoutSreDef TestResult;
	bool SelfTestResult;
	int retry=0;
	char WakeMsg[]={"����ID:0x0"};
	//������
	if(!CfgData.EnableHPGauge)return;
	//׼������INA226
	ShowPostInfo(95,"���ø߾��ȹ��ʼ�\0","73",Msg_Statu);
	INAConf.ConvMode=INA226_Cont_Both; //ͬʱת����ѹ�͵�������������
	INAConf.IBUSConvTime=INA226_Conv_588US;
	INAConf.VBUSConvTime=INA226_Conv_588US;
	INAConf.AvgCount=INA226_AvgCount_128;    //����ƽ������=128������ת��ʱ��0.588mS���ܸ���ʱ��=0.588*75.264mS��С��ϵͳ125mS����ѯ���
	INAConf.IsAlertPinInverted=false;
	INAConf.IsEnableAlertLatch=false;
	INAConf.AlertConfig=A226_AlertDisable; //�ر����о�������ʹ�þ�����ع���
	INAConf.ShuntValue=IsEnable17AMode?2.50:5.00; //����������ֵ����Ĭ��ֵ����
	//��������
	Result=INA226_INIT(&INAConf);	
	if(Result!=A226_Init_OK)
		{
		ShowPostInfo(95,"���ʼƳ�ʼ��ʧ��\0","7A",Msg_Warning);
		delay_Second(1);
		WakeMsg[9]='0'+(char)Result;
		ShowPostInfo(95,WakeMsg,"78",Msg_Warning);
		delay_Second(1);
		ShowPostInfo(95,A226ErrorStr[(char)Result-1],"78",Msg_Warning);	
		delay_Second(1);
		}			
	else IsEnableHPGauge=true;
	delay_ms(100);
	//����һ�β�������
	ShowPostInfo(97,"���ʼ��Լ�...\0","74",Msg_Statu);		
	SelfTestResult=INA226_SetAlertRegister(0);	
	//ѭ���ȴ�ֱ��CVRF���𣬱�ʾ���Զ�ȡ���
	if(SelfTestResult)do
		{
		//CNVR���𣬱���Ѿ��ɹ���ʼ��
		if(INA226_QueueIfGaugeCanReady())break;
		//�����ȴ�
		delay_ms(10);
		retry++;
		}
	while(retry<40);
	//���þ����Ĵ���ʧ�ܣ�ֱ�ӱ���
	else retry=40;
	//�ж��Ƿ��Լ�ʧ��
	if(retry==40||!INA226_GetBusInformation(&TestResult))
		{
		ShowPostInfo(95,"���ʼ��Լ��쳣\0","7A",Msg_Warning);
		delay_Second(1);
		IsEnableHPGauge=false;
		}
	}
