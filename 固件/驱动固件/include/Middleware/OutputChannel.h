#ifndef _OC_
#define _OC_

//���ͨ��״̬��
typedef enum
	{
	//���ͨ�����׹رգ�����״̬
	OutCH_Standby,
	//���ͨ��������������
	OutCH_PWMDACPreCharge,  //PWMDACԤƫ��
	OutCH_StartAUXPSU, //��������PSU
	OutCH_EnableBoost, //������Boost
	OutCH_ReleasePreCharge, //�𲽸�λPreCharge DAC���������SysUp
	OutCH_SubmitDuty, //Ӧ��ռ�ձ�
	//���ͨ���������н׶�
	OutCH_OutputEnabled,
	//��ȫ�رս׶�
	OutCH_GracefulShut,
	OutCH_WaitVOUTDecay,
	//���ͨ���ڱ����Ƚ׶ν���idle(LED�Ͽ����������Ϊ14.7V)
	OutCH_EnterIdle,	
	OutCH_OutputIdle,
  //���ͨ������ʧ��
	OutCH_PreChargeFailed
	}OutChFSMStateDef;

//���ͨ����������
#define MainChannelShuntmOhm 1.00 //��ͨ���ļ���������ֵ(mR)
#define CurrentSenseOpAmpGain 100 //�������Ŵ���������

//���ͨ�������ο���	
#define CalcIREFValue(x) ((x/2)+(x/6))
	
	
//�ⲿ�ο�
extern xdata volatile int Current; //����ֵ
extern xdata int CurrentBuf; //��ǰ��Ӧ�õĵ���ֵ
extern bit IsCurrentRampUp;  //�����������������еı��λ�����ں�MPPT��̽������
	
//����
void SetHBLEDState(bit State); //��������LED
void OutputChannel_Init(void);
void OutputChannel_Calc(void);
void OCFSM_TIMHandler(void);
void OutputChannel_TestRun(void); //���ͨ��������
void OutputChannel_DeInit(void); //���ͨ����λ
bit GetIfOutputEnabled(void);		//�ⲿ��ȡ����Ƿ��������õĺ���
bit GetIfSystemInPOFFSeq(void);	//��ȡϵͳ�Ƿ��ڰ�ȫ�ػ��׶�
	
#endif
