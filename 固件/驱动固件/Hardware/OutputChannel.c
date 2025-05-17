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
	return buf>100?100:buf;
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
	//��ʼ��bit
  AUXEN=0;
	BOOSTRUN=0;
	PWMDACEN=0;
	SYSHBLED=0; //����bit��Ϊ0
	//��ʼ����IO	
	GPIO_ConfigGPIOMode(PWMDACENIOG,GPIOMask(PWMDACENIOx),&OCInitCfg);	
	GPIO_ConfigGPIOMode(AUXENIOG,GPIOMask(AUXENIOx),&OCInitCfg);			
	GPIO_ConfigGPIOMode(BOOSTRUNIOG,GPIOMask(BOOSTRUNIOx),&OCInitCfg);		
	GPIO_ConfigGPIOMode(LEDNegMOSIOG,GPIOMask(LEDNegMOSIOx),&OCInitCfg);
  GPIO_ConfigGPIOMode(SYSHBLEDIOG,GPIOMask(SYSHBLEDIOx),&OCInitCfg);		
	//���ø�λ������������״̬
  OutputChannel_DeInit();
	}

//������������������LED
//void SetHBLEDState(bit State)
//	{
//	SYSHBLED=State;
//	}	
	
//Ԥ���״̬����ʱ��
void OCFSM_TIMHandler(void)
	{
	//����LED����	
	if(CurrentMode->ModeIdx==Mode_Fault) //��������ʱHB����
		SYSHBLED=SYSHBLED?0:1; //��תLED
	else if(GetIfOutputEnabled())//��������ã�LED����Ϊ1
		{
		//���ϵͳ���ڴ���״̬�����LED����
		if(OutputFSMState==OutCH_OutputIdle)
			{
			SYSHBLED=HBTimer==3?1:0; //����״̬��ÿ���������һ��
			if(HBTimer<4)HBTimer++;
			else HBTimer=0;
			}
		//����״̬��LED����
		else SYSHBLED=1;
		}			
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
	if(OutputFSMState==OutCH_OutputEnabled)return 1;
	if(OutputFSMState==OutCH_SubmitDuty)return 1;
	//���򷵻�0
	return 0;
	}

//��ȡϵͳ�Ƿ��ڰ�ȫ�ػ��׶�
bit GetIfSystemInPOFFSeq(void)
	{
	//���ϵͳ������ػ��ĵȴ��׶�������ر�	
	if(OutputFSMState==OutCH_WaitVOUTDecay)return 1;
  if(OutputFSMState==OutCH_GracefulShut)return 1;		
	//ϵͳ�Ѿ��رգ�����0
	return 0;
	}	
	
//���ͨ��������
void OutputChannel_TestRun(void)
	{
	char retry=100;
	bit IsDCDCOV=0;
	//�򿪸�����Դ��PWMDAC
	AUXEN=1;
	PWMDACEN=1;
	PWM_ForceEnableOut(1);
	//��ʱ40mS�����ѹ�������ѹ����8V�������������м��
	delay_ms(40);
	SystemTelemHandler();
	if(Data.RawBattVolt>8)BOOSTRUN=1; //��3787EN=1���������
	else 
		{
		PWM_ForceEnableOut(0);
		OutputChannel_DeInit(); //�ر�PWM�������λ���ͨ��
		return;
		}
	//������״̬
	do
		{
		SystemTelemHandler();
		if(Data.OutputVoltage>16.5)IsDCDCOV=1; //��ǳ��ֹ�ѹ
		else if(Data.OutputVoltage>14.0)
			{
			//���ͨ�����ر�DCDC
			BOOSTRUN=0;
			delay_ms(5);
			//��λPWMDAC
			PWMDACEN=0;
			PWM_ForceEnableOut(0);
			//�رո�����Դ
			AUXEN=0;	
			return;
			}
	  //���ʧ��
		delay_ms(5);
		retry--;
		}
	while(retry>0);
	//DCDC��鳬ʱ���������
	ReportError(IsDCDCOV?Fault_DCDCPreChargeFailed:Fault_DCDCFailedToStart);
	}

//���ͨ������
void OutputChannel_Calc(void)
	{
	int TargetCurrent,ILIM;
	extern bit IsBatteryAlert;
	//��ȡĿ�����
	TargetCurrent=Current;
	if(Current>0) //��������0˵������Ч���ִ���¿ؼ���
		{
		//�¿ؼ���
		ILIM=ThermalILIMCalc();
		if(TargetCurrent>ILIM)TargetCurrent=ILIM; //�¿ط��������е�������Ŀ��ֵ����������
		}
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
			 CurrentBuf=TargetCurrent>CalcIREFValue(1500)?CalcIREFValue(1500):TargetCurrent;
			 PWMDuty=Duty_Calc(CurrentBuf);  //���������������ǰ��������1.5A����ǯλ��1.5A��Ȼ���������ֵ����PWMDAC
       //����CV��ѹ��DAC
       PreChargeDACDuty=0x82A; //0x82A=87.128%=11.29-0.4815*14.4->(0.87128*5)
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
		  if(Data.OutputVoltage>14.0)OutputFSMState=OutCH_ReleasePreCharge; //��ѹ�����ˣ�������������
		  //�ȴ���ʱ�󱨴�
		  if(PreChargeFSMTimer>0)break;
		  ReportError(Fault_DCDCFailedToStart);
		  OutputFSMState=OutCH_PreChargeFailed;
			break;
		//��������4�����µ�Ԥ��PWMDAÇ�������ѹ
		case OutCH_ReleasePreCharge:
			//��ͨLED����FET��LED��ʼ����
			LEDMOS=1;
		  //�������Ϊ0��ʼ����ŵ�׶�
			if(TargetCurrent==0)OutputFSMState=OutCH_GracefulShut;
			//���ռ�ձȵ�����0��������������������Ŀ��ֵ�ĳ���
		  if(!PreChargeDACDuty)
				{
				//���Ԥ�����֮����Ӧ�õĵ�����Ŀ��ֵͬ����ֱ����ת���������״̬
				OutputFSMState=(CurrentBuf==TargetCurrent)?OutCH_OutputEnabled:OutCH_SubmitDuty;
				break;
				}
		  //��ʼ���µ�Ԥ��ռ�ձȰ������ѹ�����ֵ
		  if(IsNeedToUploadPWM)break; //�ϴε�����δ���
			ILIM=TargetCurrent/25;
			if(ILIM>200)ILIM=200; //�����ÿ��PWMDAC�ݼ���ֵ
			PreChargeDACDuty-=ILIM+1;
		  if(PreChargeDACDuty<0)PreChargeDACDuty=0; //��ֹռ�ձ�Ϊ����
			IsNeedToUploadPWM=1;
		  break;
		//Ӧ��ռ�ձ�
		case OutCH_SubmitDuty:
			if(IsNeedToUploadPWM)break; //PWM����Ӧ���У��ȴ�
		  if(TargetCurrent==0)OutputFSMState=OutCH_GracefulShut; //ϵͳ��������Ϊ0��˵����Ҫ�������У���ת������
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
		//���ͨ���������н׶�
		case OutCH_OutputEnabled:
			if(!TargetCurrent)OutputFSMState=OutCH_GracefulShut;  //ϵͳ��������Ϊ0��������ػ��׶ο�ʼ�µ������ѹ
			else if(TargetCurrent==-1)OutputFSMState=OutCH_EnterIdle;	//ϵͳ��������Ϊ-1��˵����Ҫ��ͣLED��������ת����ͣ����
			else if(TargetCurrent!=CurrentBuf)OutputFSMState=OutCH_SubmitDuty; //ռ�ձȷ����������ʼ���д���
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
		  PreChargeDACDuty=0x82A; //0x82A=87.128%=11.29-0.4815*14.4->(0.87128*5)
		  IsNeedToUploadPWM=1;
			//�ȴ������ѹ�½�
		  if(CurrentMode->ModeIdx!=Mode_Beacon&&Data.OutputVoltage>17.3)break;
		  LEDMOS=0; //�Ͽ�LEDMOS�жϵ���
		  OutputFSMState=OutCH_OutputIdle; //����idle״̬
		  break;
		//��ʱ�ر�LED�ĵȴ�
		case OutCH_OutputIdle:
			if(TargetCurrent==0)OutputFSMState=OutCH_GracefulShut;  //ϵͳ��������Ϊ0��������ػ��׶�
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
