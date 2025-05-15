#include "ADCCfg.h"
#include "LEDMgmt.h"
#include "delay.h"
#include "ModeControl.h"
#include "TempControl.h"
#include "BattDisplay.h"
#include "OutputChannel.h"
#include "PWMCfg.h"
#include "LowVoltProt.h"
#include "SelfTest.h"

//�ڲ�����
static xdata int TempIntegral;
static xdata int TempProtBuf;
static char StepDownTIM;  //������ʾ��ʱ
static unsigned char StepUpLockTIM; //��ʱ��

//�ڲ�״̬λ
static bit IsThermalStepDown; //���λ���Ƿ񽵵�
static bit IsTempLIMActive;  //�¿��Ƿ��Ѿ�����
static bit IsSystemShutDown; //�Ƿ񴥷��¿�ǿ�ƹػ�

//�ⲿ״̬λ
bit IsDisableTurbo;  //��ֹ�ٶȽ��뵽������
bit IsForceLeaveTurbo; //�Ƿ�ǿ���뿪������

//�ڲ��궨��
#define MinumumILED CalcIREFValue(ILEDStepDown)


//������ʱ����ݵ�ǰ���µĵ�������PIֵ
void RecalcPILoop(int LastCurrent)	
	{
	int buf,ModeCur;
	//Ŀ�굲λ����Ҫ����
	if(!CurrentMode->IsNeedStepDown)return;
	//��ȡ��ǰ��λ����
	ModeCur=QueryCurrentGearILED();
	//����Pֵ����
	buf=TempProtBuf+(TempIntegral/IntegralFactor); //��������ۼ�ֵ
	if(buf<0)buf=0; //�����ۼ�ֵ����С��0
  buf=LastCurrent-buf; //�ɵ�λ������ȥ�ۼ�ֵ�õ�ʵ�ʵ���(mA)
	TempProtBuf=ModeCur-LastCurrent; //Pֵ��������µ�λ�ĵ���-�ɵ�λʵ�ʵ���(mA)
	if(TempProtBuf<0)TempProtBuf=0; //�������������С��0
	TempIntegral=0; //���ֻ���=0
	}
	
//�����ǰ�¿ص�����ֵ
int ThermalILIMCalc(void)
	{
	int result;
	//�ж��¿��Ƿ���Ҫ���м���
	if(!IsTempLIMActive)result=Current; //�¿ر��رգ��������ƽ������ٷ���ȥ����
	//��ʼ�¿ؼ���
	else
		{
		result=TempProtBuf+(TempIntegral/IntegralFactor); //���ݻ��������
		if(result<0)result=0; //������ֵ����
		result=Current-result; //��������ֵ���
		if(result<MinumumILED) //�Ѿ��������ˣ���ֹPID�����ۼ�
			{
		  TempProtBuf=Current-MinumumILED; //�������������޷�Ϊ��С����
		  TempIntegral=0;
		  result=MinumumILED; //�������Ʋ�����С����͵���
			}
		}
	//���ؽ��	
	IsThermalStepDown=result==Current?0:1; //����������������򽵵�û����
	return result; 
	}
//��ȡ�¿ػ�·�ĺ���ֵ
static int QueryConstantTemp(void)	
	{
	//������ʱ��ʹ�ø��ߵ��¿���������ʱ��
	return CurrentMode->ModeIdx==Mode_Turbo?TurboConstantTemperature:ConstantTemperature;
	}

//�¿�PI������
void ThermalPILoopCalc(void)	
	{
	int ProtFact,Err;
	//PI���رգ���λ��ֵ
	if(!IsTempLIMActive)
		{
		TempIntegral=0;
		TempProtBuf=0;
		IsThermalStepDown=0;
		}
	//����PI���ļ���(�������������ʱ����л��߱���ģʽ���й�����ǿ�ƽ���)
	else if(GetIfOutputEnabled()||CurrentMode->ModeIdx==Mode_Strobe)
		{
		//��ȡ�����¶�ֵ
		ProtFact=QueryConstantTemp();
		//�¶����Ϊ��
		if(Data.Systemp>ProtFact)
			{
			//�������
			Err=Data.Systemp-ProtFact;  //���ֵ����Ŀ���¶�-�����¶�
			//������(P)
			StepUpLockTIM=24; //����֮���¶ȹ�����֮��ֹͣ3��
			if(Err>2)
				{
				ProtFact=(CurrentBuf/2300)+1;
			  if(Data.Systemp>(ForceDisableTurboTemp-5))ProtFact*=5; //�¶ȹ��ߣ����ű���ϵ��
				TempProtBuf+=(ProtFact*Err);	//��buf�ύ������
				if(TempProtBuf>(CurrentMode->Current-MinumumILED))TempProtBuf=(CurrentMode->Current-MinumumILED); 
				StepUpLockTIM=60; //�������������ͣ7.5��
				}
			//������(I)
			if(TempIntegral<IntegrateFullScale)TempIntegral++;
			}
		//�¶�С�ں���ֵ
		else if(Data.Systemp<ProtFact)
			{
			//�������
			Err=ProtFact-Data.Systemp;	 //�����ڱ���������	
			//������
			if(StepUpLockTIM)StepUpLockTIM--; //��ǰ����������û�ﵽ����������ʱ��
			else
				{
				if(Err&0x7E)TempProtBuf-=Err; //��������
				if(TempProtBuf<0)TempProtBuf=0;
				}
			//������
			if(TempIntegral>(-IntegrateFullScale))TempIntegral--;		
			}
		}
	}
//��ʾ�¶ȿ�������
bit ShowThermalStepDown(void)	
	{
	StepDownReasonDef Reason;
	//�ж�ϵͳ�Ƿ��ڽ���
	if(VshowFSMState!=BattVdis_Waiting)Reason=StepDown_OFF; //��ǰ���ڵ�����ʾ״̬��������
	else if(IsThermalStepDown)Reason=StepDown_Thermal; //�¿ؽ�������
	else if(CurrentMode->ModeIdx==Mode_Turbo&&TurboILIM<QueryCurrentGearILED())Reason=StepDowm_BattAlert; //��سŲ�ס
	else Reason=StepDown_OFF;
	//���н�������
  switch(Reason)		
		{
		case StepDown_OFF:StepDownTIM=0;break; //��ʾδ����
		case StepDowm_BattAlert: //��ؾ���
		  //����ʱ��=10ʱ����һ�������������˸
			if(StepDownTIM==10)
				{
				StepDownTIM++;
				return 1;
				}
		case StepDown_Thermal: //����
			StepDownTIM++;
			if(StepDownTIM==13)
				{
				StepDownTIM=0;
				return 1;
				}
			break;
		}
	//����0
	return 0;
	}

//�����¶�ʹ�ܿ��Ƶ�ʩ���ش�����
static bit TempSchmittTrigger(bit ValueIN,char HighThreshold,char LowThreshold)	
	{
	if(Data.Systemp>HighThreshold)return 1;
	if(Data.Systemp<LowThreshold)return 0;
	//��ֵ���֣�û�иı�
	return ValueIN;
	}

//�¶ȹ�����
void ThermalMgmtProcess(void)
	{
	bit ThermalStatus;
	//�¶ȴ���������
	if(!Data.IsNTCOK)
		{
		ReportError(Fault_NTCFailed);
		return;
		}
	//�ֵ��¶ȹ���ʱ�Լ�����������
	IsForceLeaveTurbo=TempSchmittTrigger(IsForceLeaveTurbo,ForceOffTemp-10,ForceDisableTurboTemp-10);	//�¶Ⱦ���ػ������ļ�಻��10�ȣ������˳�����
	IsDisableTurbo=TempSchmittTrigger(IsDisableTurbo,ForceDisableTurboTemp,ForceDisableTurboTemp-10); //�¶ȴﵽ�رռ���������ֵ���رռ���
	//���ȹػ�����
	IsSystemShutDown=TempSchmittTrigger(IsSystemShutDown,ForceOffTemp,ConstantTemperature-10);
  if(IsSystemShutDown)ReportError(Fault_OverHeat); //������
	else if(ErrCode==Fault_OverHeat)ClearError(); //��������ǰ����
	//PI��ʹ�ܿ���
	if(!CurrentMode->IsNeedStepDown)IsTempLIMActive=0; //��ǰ��λ����Ҫ����
	else //ʹ��ʩ���غ��������¿��Ƿ񼤻�
		{
		ThermalStatus=TempSchmittTrigger(IsTempLIMActive,QueryConstantTemp(),ReleaseTemperature); //��ȡʩ���ش������Ľ��
		if(ThermalStatus)IsTempLIMActive=1;//ʩ���غ���Ҫ�󼤻��¿أ���������
		else if(!ThermalStatus&&!TempProtBuf&&TempIntegral<0)IsTempLIMActive=0; //ʩ���غ���Ҫ��ر��¿أ��ȴ���������Ϊ0���������ر�
		}
	}	
