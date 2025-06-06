#include "cms8s6990.h"
#include "PinDefs.h"
#include "GPIO.h"
#include "PWMCfg.h"
#include "delay.h"
#include "SpecialMode.h"
#include "TempControl.h"
#include "OutputChannel.h"
#include "ADCCfg.h"
#include "SelfTest.h"

//�ڲ�SFR
sbit AUXEN=AUXENIOP^AUXENIOx; //����6V DCDCʹ��
sbit PWMDACEN=PWMDACENIOP^PWMDACENIOx; //PWMDACʹ��
sbit SYSHBLED=SYSHBLEDIOP^SYSHBLEDIOx; //����LED
sbit BOOSTRUN=BOOSTRUNIOP^BOOSTRUNIOx; //LTC3787��EN
sbit LEDMOS=LEDNegMOSIOP^LEDNegMOSIOx; //LEDMOSFET

//�ⲿ�������òο�
xdata volatile int Current; //Ŀ�����(mA)
xdata int CurrentBuf; //�洢��ǰ�Ѿ��ϴ��ĵ���ֵ 
bit IsCurrentRampUp;  //�����������������еı��λ�����ں�MPPT��̽������

//�ڲ�����
static bit IsEnableSlowILEDRamp; //��־λ���Ƿ��������ٵ���б�ʿ���
static xdata char PreChargeFSMTimer; //Ԥ���״̬����ʱ��
static xdata OutChFSMStateDef OutputFSMState; //�������״̬��
static xdata char HBTimer; //������ʱ��

//�ڲ�״̬��
static code OutChFSMStateDef NeedsOFFStateTable[]=
	{
	//��״̬���¼�˿���ͨ����Ŀ���������Ϊ0ʵ�ֹػ���״̬
	OutCH_1LumenOpenRun,	
	OutCH_ReleasePreCharge,
	OutCH_SubmitDuty,
	OutCH_OutputEnabled,
	OutCH_OutputIdle
	};

/*********************************************************************************************************************
���ͨ����������ʹ�õ��ڲ������������ڸ��ļ��ڵ��á�
*********************************************************************************************************************/

//�ڲ����ڼ���PWMDACռ�ձȵĺ���	
static float Duty_Calc(int CurrentInput)
	{
	float buf;
	//����ʵ��ռ�ձ�
	buf=(float)CurrentInput*(float)MainChannelShuntmOhm; //���봫�����ĵ���(mA)�����Լ���������ֵ(mR)�õ��������账��Ŀ���ѹ(uV)
	buf*=(float)0.0015; //uVתmV������1.5mA per LSB����õ�ʵ�ʵĵ���ֵ
	buf*=CurrentSenseOpAmpGain; //���������账��Ŀ���ѹ(mV)���Լ����Ŵ���������õ��˷Ŷ˵�����ֵ
	buf/=Data.MCUVDD*(float)1000; //�����Ŀ��DAC�����ѹ��PWMDAC�����������ѹ(MCUVDD)֮��ı�ֵ
	buf*=102; //ת��Ϊ�ٷֱ�(����102������ϵͳ�Ļ������)
	//������	
	return buf;
	}
	
/*********************************************************************************************************************
���ͨ����������ʹ�õ��ⲿ�����������������ط����á�
*********************************************************************************************************************/

//��ʼ������
void OutputChannel_Init(void)
	{
	GPIOCfgDef OCInitCfg;
	//���ýṹ��
	OCInitCfg.Mode=GPIO_Out_PP;
  OCInitCfg.Slew=GPIO_Fast_Slew;		
	OCInitCfg.DRVCurrent=GPIO_High_Current; //��MOSFET,��Ҫ������б��
	//���ø�λ������������״̬
  OutputChannel_DeInit();
	//��ʼ����IO	
	GPIO_ConfigGPIOMode(PWMDACENIOG,GPIOMask(PWMDACENIOx),&OCInitCfg);	
	GPIO_ConfigGPIOMode(AUXENIOG,GPIOMask(AUXENIOx),&OCInitCfg);			
	GPIO_ConfigGPIOMode(BOOSTRUNIOG,GPIOMask(BOOSTRUNIOx),&OCInitCfg);		
	GPIO_ConfigGPIOMode(LEDNegMOSIOG,GPIOMask(LEDNegMOSIOx),&OCInitCfg);
  GPIO_ConfigGPIOMode(SYSHBLEDIOG,GPIOMask(SYSHBLEDIOx),&OCInitCfg);		
	}

//Ԥ���״̬����ʱ��
void OCFSM_TIMHandler(void)
	{
	//����LED����	
	if(CurrentMode->ModeIdx==Mode_Fault) //��������ʱHB����
		SYSHBLED=SYSHBLED?0:1; //��תLED
	else if(CurrentMode->ModeIdx==Mode_1Lumen)SYSHBLED=0; //��������������LEDʡ��
	else if(GetIfOutputEnabled())SYSHBLED=1;//��������ã�LED����Ϊ1		
	else //����״̬������
		{
	  if(HBTimer<4)HBTimer++;
	  else
			{
			SYSHBLED=SYSHBLED?0:1; //��תLED
			HBTimer=0;
			}
		}
	//״̬����ʱ��
	if(PreChargeFSMTimer>0)PreChargeFSMTimer--;
	}	
	
//���ͨ����λ
void OutputChannel_DeInit(void)
	{
	BOOSTRUN=0;
	AUXEN=0;
	LEDMOS=0;
	PWMDACEN=0;
	SYSHBLED=0; //����bit��Ϊ0
	//ϵͳ�ϵ�ʱ��������Ϊ0
	Current=0;
	CurrentBuf=0;
	IsCurrentRampUp=0;
	IsEnableSlowILEDRamp=0;
	//��λ״̬��
	HBTimer=0;
	OutputFSMState=OutCH_Standby;
	}	

//�ⲿ��ȡ����Ƿ��������õĺ���
bit GetIfOutputEnabled(void)	
	{
	/**********************************************	
	ϵͳ�ڿ������С�Ӧ��ռ�ձȺ�������������õ�ʱ
	�򣬷�������״̬������Ϊʲô��������if���߲��
	������������״̬����������λ�ã�
	OutCH_SubmitDutyΪID5
	OutCH_OutputEnabledΪID6
	OutCH_1LumenOpenRunΪID7
	�����ӱ�����If����switchҪʡ�ܶ�ռ䡣
	**********************************************/
	if(OutputFSMState>4&&OutputFSMState<8)return 1;
	//���򷵻�0
	return 0;
	}

//��ȡϵͳ�Ƿ��ڰ�ȫ�ػ��׶�
bit GetIfSystemInPOFFSeq(void)
	{
	/************************************************
	���ϵͳ������ػ��ĵȴ��׶��򷵻�1��������ϵͳ��
	���κ��µ��û�������ֱ������Ѿ���ɷŵ磬�����
	������û�ж���ĵ��ܺ��������������û�������
	************************************************/
	if(OutputFSMState==OutCH_GracefulShut||
		 OutputFSMState==OutCH_WaitVOUTDecay)return 1;
	//ϵͳ�Ѿ��رգ�����0
	return 0;
	}	
	
//���ͨ��������
void OutputChannel_TestRun(void)
	{
	char retry=100;
	//�򿪸�����Դ��PWMDAC
	AUXEN=1;
	PWMDACEN=1;
	PWM_ForceEnableOut(1);
	//��ʱ40mS�����ѹ�������ѹ����8V�������������м��
	delay_ms(40);
	SystemTelemHandler();
	//��ص�ѹ��������3787��ʼ���У�����������
	if(Data.RawBattVolt>8)
		{
		//��3787EN=1���������
	  BOOSTRUN=1; 
		//���������ѭ����ȡDCDC�������ѹ���DCDCģ�飬Ԥ��ϵͳ�Ƿ�����
		do
			{
			SystemTelemHandler();
			//DCDC�����ѹ�������ر�ϵͳ������
			if(Data.OutputVoltage>16.5)
				{
				ReportError(Fault_DCDCPreChargeFailed);
				break;
				}
			//DCDC��������������˳�
			else if(Data.OutputVoltage>14.0)break;
			//���ʧ�ܣ���ʱ5mS������
			delay_ms(5);
			}
		while(--retry);		
		}
	//���������ر�DCDC����λPWMDAC
	PWM_ForceEnableOut(0);
	OutputChannel_DeInit();
	//���ݳ�ʱ����ж��Ƿ��쳣
	if(!retry)ReportError(Fault_DCDCFailedToStart);
	}
	
//���ͨ������
void OutputChannel_Calc(void)
	{
	int TargetCurrent;
	char i;
	//��ȡĿ�������Ӧ���¿ؼ�Ȩ����
	if(Current>0)
		{
		//ȡ���¿���������
		TargetCurrent=ThermalILIMCalc();
		//���Ŀ�����С�ڵ�ǰ��λ���¿�����ֵ����Ӧ�õ�ǰ���õĵ���ֵ
		if(Current<TargetCurrent)TargetCurrent=Current;
		}
	//����ֵΪ0����-1��ֱ�Ӷ�ȡĿ�����ֵ
	else TargetCurrent=Current;
	//���ϵͳ�Ƿ���Ҫ�ػ�������ػ�״̬	
	i=sizeof(NeedsOFFStateTable);
	do
	  {
		//������״̬����ǰ��״̬Ϊ���ڶ�Ӧֵ����ϵͳĿ�����Ϊ0�������ػ�����
		if(OutputFSMState==NeedsOFFStateTable[i-1]&&!TargetCurrent)
			{	
			OutputFSMState=OutCH_GracefulShut;
			//���ѭ����ֹ��������
			break;
			}
		}
	while(--i);
	//�������ͨ��״̬������
	switch(OutputFSMState)
		{
		//���ͨ������	
		case OutCH_PreChargeFailed:
			 OutputChannel_DeInit(); //ִ�������λ
		   break; 
		//���ͨ������״̬
    case OutCH_Standby:
       //��λDCDC����
		   BOOSTRUN=0;
			 AUXEN=0;
			 LEDMOS=0;
			 PWMDACEN=0;
	     //��λ���λ
	     IsCurrentRampUp=0;
			 //��λPWMDAC
		   if(PreChargeDACDuty||PWMDuty>0)
					{
					PreChargeDACDuty=0;
					PWMDuty=0;
					IsNeedToUploadPWM=1;
					}
			 //�����������������������״̬
			 if(TargetCurrent>0)OutputFSMState=OutCH_PWMDACPreCharge;
			 break;
		//��������1���ͳ�PWMDAC
		case OutCH_PWMDACPreCharge:
       //������������DAC
		   PWMDACEN=1;
		   //����PWMDACռ�ձ�
		   if(CurrentMode->ModeIdx==Mode_1Lumen)CurrentBuf=CalcIREFValue(25); //1LM��λ��Ϊ�˱����˷�ͬ������ӵص���CC����������������������һ����ֵ
			 else CurrentBuf=TargetCurrent>CalcIREFValue(1500)?CalcIREFValue(1500):TargetCurrent;
			 PWMDuty=Duty_Calc(CurrentBuf);  //���������������ǰ��������1.5A����ǯλ��1.5A��Ȼ���������ֵ����PWMDAC
       //����CV��ѹ��DAC
       PreChargeDACDuty=CVPreStartDACVal;
       //�ϴ�ռ�ձȲ���ת����һ��(������DCDC����PSU)
			 IsNeedToUploadPWM=1;
		   OutputFSMState=OutCH_StartAUXPSU;
		   break;
		//��������2����������PSU����LTC3787����ĵ�Դ��
		case OutCH_StartAUXPSU:
			 //�ȴ�PWM���
		   if(IsNeedToUploadPWM)break;
		   delay_ms(20); //��ʱ20mS
		   //����������Դ����ת���¸�״̬
			 AUXEN=1;
		   PreChargeFSMTimer=16; //���ü�ʱ�����ȴ�2��
		   OutputFSMState=OutCH_EnableBoost;
		   break;
		//��������3��������DCDC���������Ƿ�����
		case OutCH_EnableBoost:
			//��3787EN=1����Boost��ʼ���Ȼ�����ѹ״̬
			BOOSTRUN=1;
			if(Data.OutputVoltage>14.0)OutputFSMState=CurrentMode->ModeIdx==Mode_1Lumen?OutCH_1LumenOpenRun:OutCH_ReleasePreCharge; //��ѹ�����ˣ�������������
		  //�ȴ���ʱ�󱨴�
		  if(PreChargeFSMTimer>0)break;
		  ReportError(Fault_DCDCFailedToStart);
		  OutputFSMState=OutCH_PreChargeFailed;
			break;
		//��������4�����µ�Ԥ��PWMDAÇ�������ѹ
		case OutCH_ReleasePreCharge:
			//��ͨLED����FET��LED��ʼ����
			LEDMOS=1;
			//��ʼ���µ�Ԥ��ռ�ձȰ������ѹ�����ֵ
		  if(IsNeedToUploadPWM)break; //�ϴε�����δ���
		  if(PreChargeDACDuty>0)
				{
				//�������е������µ�ռ�ձ�
				TargetCurrent=1+(TargetCurrent/25);
				if(TargetCurrent>200)TargetCurrent=200; //�����ÿ��PWMDAC�ݼ���ֵ
				PreChargeDACDuty-=TargetCurrent;
				if(PreChargeDACDuty<0)PreChargeDACDuty=0; //��ֹռ�ձ�Ϊ����
				//Ԥ��״̬����ʱΪ20����Ҫ20����ѭ��ȷ������ֵ�ѱ�Ӧ�òż���
				PreChargeFSMTimer=20;
				//���ռ�ձ��Ѹ��£���Ҫ�ϴ�����ֵ	
				IsNeedToUploadPWM=1;
				}
			//Ԥ��PWMDAC���=0��˵��Ԥ����ɡ�ϵͳ��ʼ����Ԥ��״̬������ʱ
			else if(PreChargeFSMTimer>0)PreChargeFSMTimer--;	
			//����ʱ������ϵͳ�Ѿ�Ӧ���������������ʱ������������Ƿ�ƥ����ת��Ŀ��״̬
			else OutputFSMState=(CurrentBuf==TargetCurrent)?OutCH_OutputEnabled:OutCH_SubmitDuty;
		  break;
		//��������5��Ӧ������PWMDACռ�ձ�̧�����������Ŀ��ֵ
		case OutCH_SubmitDuty:
			if(IsNeedToUploadPWM)break; //PWM����Ӧ���У��ȴ�
			//����LED�ĵ���б��������
			if(TargetCurrent-CurrentBuf>CalcIREFValue(6000))IsEnableSlowILEDRamp=1; //��⵽�ǳ���ĵ���˲̬������屬�����������
			if(!SysMode&&IsEnableSlowILEDRamp)
				{
				switch(CurrentMode->ModeIdx)
					{
					case Mode_Turbo:CurrentBuf+=IsInputLimited?0:10;break;  //����MPPTϵͳ���������澯���ʹ��
					case Mode_Beacon:CurrentBuf+=5000;break;
					case Mode_Strobe:CurrentBuf+=1500;break;
					case Mode_SOS:CurrentBuf+=500;break;
					default:CurrentBuf+=15;
					}
				if(CurrentBuf>=TargetCurrent)
					{
					IsEnableSlowILEDRamp=0;
					CurrentBuf=TargetCurrent; //�޷���������Ŀ�������������ֵ
					}
				}
			else CurrentBuf=TargetCurrent; //ֱ��ͬ��		
		  //����ռ�ձ�
			IsNeedToUploadPWM=1;
			PWMDuty=Duty_Calc(CurrentBuf);
			//ռ�ձ���ͬ������ת���������н׶�
			if(TargetCurrent==CurrentBuf)
				{
				IsCurrentRampUp=1; //��ǵ�����������
				OutputFSMState=OutCH_OutputEnabled;
				}
	    break;
		//�������У�1�����������е�λ
		case OutCH_1LumenOpenRun:
			//��ͨLED����FET��LED��ʼ����
			LEDMOS=1;
		  //������ѹ��DAC������ֵ
	    IsNeedToUploadPWM=1;
		  PreChargeDACDuty=OneLMDACVal;
			//ϵͳ���Խ�����ͨ�¹⣬���ص��µ�ռ�ձ�ģʽ
		  if(TargetCurrent>2)OutputFSMState=OutCH_ReleasePreCharge;
		  break;
		//���ͨ���������н׶�
		case OutCH_OutputEnabled:
			if(TargetCurrent==2)OutputFSMState=OutCH_1LumenOpenRun; //����1������������ģʽ
			if(TargetCurrent==-1)OutputFSMState=OutCH_EnterIdle;	//ϵͳ��������Ϊ-1��˵����Ҫ��ͣLED��������ת����ͣ����
			if(TargetCurrent!=CurrentBuf)OutputFSMState=OutCH_SubmitDuty; //ռ�ձȷ����������ʼ���д���
			break;
		//���ͨ����ػ�����
		case OutCH_GracefulShut:
			//��LEDMOS���ر�DCDC
			LEDMOS=1;
		  BOOSTRUN=0;
			//��λPWMDAC
		  PWMDACEN=0;
		  PreChargeDACDuty=2399;
			PWMDuty=0;
		  IsNeedToUploadPWM=1;
		  //��ת���ȴ������ѹ˥���Ĺ���
		  PreChargeFSMTimer=24; //�ȴ������ѹ˥���Ĺ������ȴ�3��
		  OutputFSMState=OutCH_WaitVOUTDecay;
		  break;
		//DCDC�رգ��ȴ������ѹ˥��
		case OutCH_WaitVOUTDecay:
		  //�ȴ������ѹ˥��
		  if(Data.OutputVoltage>15.6&&PreChargeFSMTimer)break;
			//�ر�Ԥ��PWMDAC
		  PreChargeDACDuty=0;
		  IsNeedToUploadPWM=1;
			//�����ѹ˥���������ر�LEDMOS�͸�����Դ���ص�����״̬
		  LEDMOS=0;
			AUXEN=0;
		  PreChargeFSMTimer=0; //��λ��ʱ��
	    OutputFSMState=OutCH_Standby;
			break;
		//��Ҫ��ʱ�ر�LED���ڽ���idle֮ǰ��׼��
		case OutCH_EnterIdle:
			//������Ԥ��PWMDAC�ѵ�ѹǯס
		  PreChargeDACDuty=CVPreStartDACVal;
		  IsNeedToUploadPWM=1;
			//�ȴ������ѹ�½�
		  if(CurrentMode->ModeIdx!=Mode_Beacon&&Data.OutputVoltage>17.3)break;
		  LEDMOS=0; //�Ͽ�LEDMOS�жϵ���
		  OutputFSMState=OutCH_OutputIdle; //����idle״̬
		  break;
		//��ʱ�ر�LED�ĵȴ�
		case OutCH_OutputIdle:
			if(TargetCurrent>0) //LED�������´򿪣���Ҫ�������
				{
				LEDMOS=1; //��LEDMOS����ͨ����
				PreChargeDACDuty=0;
				IsNeedToUploadPWM=1; //��PWMDAC��ʼSysDown��LED����
			  OutputFSMState=OutCH_SubmitDuty; //Ӧ�����µ�ռ�ձ�
				}
			break;
		//�������ķǷ�״̬�ص�Ĭ�ϴ���
		default:OutputFSMState=OutCH_PreChargeFailed;
		}
	}
