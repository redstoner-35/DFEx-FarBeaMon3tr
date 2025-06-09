#include "cms8s6990.h"
#include "PinDefs.h"
#include "GPIO.h"
#include "PWMCfg.h"

//ȫ�ֱ���
xdata float PWMDuty;
xdata int PreChargeDACDuty; //Ԥ���PWMDAC�����
static bit IsPWMLoading; //PWM���ڼ�����
static bit IsNeedToEnableOutput; //�Ƿ���Ҫ�������
static bit IsNeedToEnableMOS; //�Ƿ���Ҫʹ��MOS��
bit IsNeedToUploadPWM; //�Ƿ���Ҫ����PWM

//�ر�PWM��ʱ��
void PWM_DeInit(void)
	{
	//����Ϊ��ͨGPIO
  GPIO_SetMUXMode(PWMDACIOG,PWMDACIOx,GPIO_AF_GPIO);	
	//�ر�PWMģ��
	PWMOE=0x00;
	PWMCNTE=0x00;
	PWM45PSC=0x00;
	PWM01PSC=0x00;  //�ر�PWM������
	}

//�ϴ�PWMֵ
static void UploadPWMValue(void)	
	{
	PWMLOADEN=0x11; //����ͨ��0��PWMֵ
	while(PWMLOADEN&0x11); //�ȴ����ؽ���
	}
		
//PWM��ʱ����ʼ��
void PWM_Init(void)
	{
	GPIOCfgDef PWMInitCfg;
	//���ýṹ��
	PWMInitCfg.Mode=GPIO_Out_PP;
  PWMInitCfg.Slew=GPIO_Fast_Slew;		
	PWMInitCfg.DRVCurrent=GPIO_High_Current; //��PWMDAC������Ҫ�ܸߵ�����б��
	//����GPIO
	GPIO_WriteBit(PreChargeDACIOG,PreChargeDACIOx,0);
  GPIO_WriteBit(PWMDACIOG,PWMDACIOx,0);
	GPIO_ConfigGPIOMode(PreChargeDACIOG,GPIOMask(PreChargeDACIOx),&PWMInitCfg); 
	GPIO_ConfigGPIOMode(PWMDACIOG,GPIOMask(PWMDACIOx),&PWMInitCfg); 
	//���ø��ù���
	GPIO_SetMUXMode(PWMDACIOG,PWMDACIOx,GPIO_AF_PWMCH0);
  GPIO_SetMUXMode(PreChargeDACIOG,PreChargeDACIOx,GPIO_AF_PWMCH4);
	//����PWM������
	PWMCON=0x00; //PWMͨ��Ϊ��ͨ������ģʽ�����¼������رշǶԳƼ�������	
	PWMOE=0x1D; //��PWM���ͨ��0 2 3 4
	PWM01PSC=0x01;  
	PWM45PSC=0x01;  //��Ԥ��Ƶ���ͼ�����ʱ�� 
  PWM0DIV=0xff;   
	PWM4DIV=0xff;   //��Fpwmcnt=Fsys=48MHz(����Ƶ)
  PWMPINV=0x00; //����ͨ��������Ϊ�������ģʽ
	PWMCNTM=0x1D; //ͨ��0 2 3 4����Ϊ�Զ�����ģʽ
	PWMCNTCLR=0x1D; //��ʼ��PWM��ʱ��λͨ��0 2 3 4�Ķ�ʱ��
	PWMDTE=0x00; //�ر�����ʱ��
	PWMMASKD=0x00; 
	PWMMASKE=0x1D; //PWM���빦�����ã�Ĭ��״̬�½�ֹͨ��0 2 3 4���
	//������������
	PWMP0H=(PWMStepConstant>>8)&0xFF;
	PWMP0L=PWMStepConstant&0xFF;	
	PWMP4H=0x09;
	PWMP4L=0x5F; //PWMͨ������(48MHz/20KHz)-1=2399(0x95F)
	//����ռ�ձ�����
  PWMD0H=0;
  PWMD4H=0x0;
	PWMD0L=0;	
	PWMD4L=0x0;
	//��ʼ������
	PWMDuty=0;
	PreChargeDACDuty=0;
	IsPWMLoading=0; 
	IsNeedToUploadPWM=0;
	//����PWM
	PWM_Enable();
	UploadPWMValue();
	}

//��ʱ������PWM����Ĺ���
void PWM_ForceEnableOut(bit IsEnable)	
	{
	PWMD0L=IsEnable?0xFF:0;	
	PWMD4H=IsEnable?0x08:0;
	PWMD4L=IsEnable?0x2A:0x0;	  //0x82A=87.128%=11.29-0.4815*14.4->(0.87128*5)
	UploadPWMValue();
	if(IsEnable)PWMMASKE&=0xEE;
	else PWMMASKE|=0x11;   //����PWMMASKE�Ĵ����������״̬���ö�Ӧ��ͨ��
	}

//����PWM�ṹ���ڵ����ý������
void PWM_OutputCtrlHandler(void)	
	{
	int value;
	float buf;
	//�ж��Ƿ���Ҫ���ص��߼�����
	if(!IsNeedToUploadPWM)return; //����Ҫ����
	else if(IsPWMLoading) //���μ����ѿ�ʼ�����н������
		{
	  if(PWMLOADEN&0x11)return;//���ؼĴ�����λΪ0����ʾ���سɹ�
	  //���ؽ���
		if(IsNeedToEnableMOS)PWMMASKE&=0xEF;
		else PWMMASKE|=0x10;
		if(IsNeedToEnableOutput)PWMMASKE&=0xFE;
		else PWMMASKE|=0x01;   //����PWMMASKE�Ĵ����������״̬���ö�Ӧ��ͨ��
		IsNeedToUploadPWM=0;
		IsPWMLoading=0;  //���ڼ���״̬Ϊ���
		return;
		}
	//PWMռ�ձȲ�������
	if(PWMDuty>100)PWMDuty=100;
	if(PWMDuty<0)PWMDuty=0;
	if(PreChargeDACDuty>2399)PreChargeDACDuty=2399;
	if(PreChargeDACDuty<0)PreChargeDACDuty=0;
	//����PWM��ֵѡ��MASK�Ĵ����Ƿ�����
	IsNeedToEnableOutput=PWMDuty>0?1:0; //�Ƿ���Ҫ�������
	IsNeedToEnableMOS=PreChargeDACDuty?1:0;  //�����Ƿ���Ҫʹ��FET
	//���üĴ���װ��PWM������ֵ
	buf=PWMDuty*(float)PWMStepConstant;
	buf/=(float)100;
	value=(int)buf;
	PWMD4H=(PreChargeDACDuty>>8)&0xFF;
	PWMD4L=PreChargeDACDuty&0xFF;
	PWMD0H=(value>>8)&0xFF;
	PWMD0L=value&0xFF;			
	//PWM�Ĵ�����ֵ��װ�룬Ӧ����ֵ		
	IsPWMLoading=1; //��Ǽ��ع��̽�����
	PWMLOADEN|=0x11; //��ʼ����
	}
