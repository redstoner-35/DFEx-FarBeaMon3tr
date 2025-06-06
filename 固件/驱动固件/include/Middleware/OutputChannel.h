#ifndef _OC_
#define _OC_

//���ͨ��״̬��
typedef enum
	{
	//���ͨ�����׹رգ�����״̬
	OutCH_Standby=0,
	//���ͨ��������������
	OutCH_PWMDACPreCharge=1,  //PWMDACԤƫ��
	OutCH_StartAUXPSU=2, //��������PSU
	OutCH_EnableBoost=3, //������Boost
	OutCH_ReleasePreCharge=4, //�𲽸�λPreCharge DAC�������ѹ������������CV״̬���ɵ�FBע���CC״̬
	OutCH_SubmitDuty=5, //Ӧ��ռ�ձȣ�LED������Ŀ�����
	//���ͨ���������н׶�
	OutCH_OutputEnabled=6,
	OutCH_1LumenOpenRun=7,
	//��ȫ�رս׶�
	OutCH_GracefulShut=8,
	OutCH_WaitVOUTDecay=9,
	//���ͨ���ڱ����Ƚ׶ν���idle(LED�Ͽ����������Ϊ14.7V)
	OutCH_EnterIdle=10,	
	OutCH_OutputIdle=11,
  //���ͨ������ʧ��
	OutCH_PreChargeFailed=12
	}OutChFSMStateDef;

//���ͨ����������
#define MainChannelShuntmOhm 1.00 //��ͨ���ļ���������ֵ(mR)
#define CurrentSenseOpAmpGain 100 //�������Ŵ���������

//���ͨ�������ο���PWMDAC��������꣨���Բ�Ҫ�޸ģ��ᱬը����	
#define CalcIREFValue(x) ((x/2)+(x/6))
#define CalcPWMDACDuty(x) (((1129000UL-(4815UL*x))*24UL)/5000UL)  //ʹ��������ʽ����PWMDACԤ���ѹ

//���PWMDACԤ���ѹ����
#define PWMDACPreCharge 144 	//PWMDAC����������µ�Ԥ���ѹ(LSB=0.1V)
#define OneLumenOut 145 //PWMDAC��1������λ�µ�Ԥ���ѹ(LSB=0.1V��FV7212D=14500mV�������ĵ�����Ҫ�Լ���)
	
/*�Զ�����ϵͳPWMDAC��Ԥ��������ֵ�������޸ģ�������*/	
#define CVPreChargeDACVal CalcPWMDACDuty(PWMDACPreCharge)
#define OneLumenCVDACVal CalcPWMDACDuty(OneLumenOut)
	
#if (OneLumenCVDACVal>CVPreChargeDACVal)
	 //1LM��λ��Ŀ�������ѹ����Ҫ��PWMDAC��ֵ����Ԥ�����������ֵ����������˸������ƫ�õĽ��ʹ��1LM��DACֵ
	 #define CVPreStartDACVal OneLumenCVDACVal
	 #define OneLMDACVal OneLumenCVDACVal
#else
   //1LM��λ��Ŀ�������ѹ����Ҫ��PWMDAC��ֵ����Ԥ�����������ֵ�����Էֿ�ʹ��
	 #define CVPreStartDACVal CVPreChargeDACVal
	 #define OneLMDACVal OneLumenCVDACVal
#endif

#if (OneLumenCVDACVal < 0 | OneLumenCVDACVal > 2399)
   #error "Error 009: Invalid CV PWMDAC Output Config Value for One Lumen(Ultra Low mode)Output!"
#endif

#if (CVPreChargeDACVal < 0 | CVPreChargeDACVal > 2399)
   #error "Error 00A: Invalid CV PWMDAC Output Config Value for System StartUp!"
#endif


//�ⲿ�ο�
extern xdata volatile int Current; //����ֵ
extern xdata int CurrentBuf; //��ǰ��Ӧ�õĵ���ֵ
extern bit IsCurrentRampUp;  //�����������������еı��λ�����ں�MPPT��̽������
	
//����
void OutputChannel_Init(void);
void OutputChannel_Calc(void);
void OCFSM_TIMHandler(void);
void OutputChannel_TestRun(void); //���ͨ��������
void OutputChannel_DeInit(void); //���ͨ����λ
bit GetIfOutputEnabled(void);		//�ⲿ��ȡ����Ƿ��������õĺ���
bit GetIfSystemInPOFFSeq(void);	//��ȡϵͳ�Ƿ��ڰ�ȫ�ػ��׶�
	
#endif
