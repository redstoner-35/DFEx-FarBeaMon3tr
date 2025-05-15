#include "BattDisplay.h"
#include "ModeControl.h"
#include "LowVoltProt.h"
#include "OutputChannel.h"
#include "SideKey.h"

//�ڲ�����
static xdata char BattAlertTimer=0; //��ص͵�ѹ�澯����
static xdata char RampCurrentRiseAttmTIM=0; //�޼�����ָ������ļ�ʱ��	
static xdata char TryTurboILIMTimer=0; //�����µ���������ȴ��ʱ

//ȫ�ֲο�
xdata int TurboILIM; //������������


//�͵�����������
static void StartBattAlertTimer(void)
	{
	//������ʱ��
	if(BattAlertTimer)return;
	BattAlertTimer=1;
	}	

//��ص͵�������������
void BattAlertTIMHandler(void)
	{
	//�����µ�������ʱ	
	if(TryTurboILIMTimer>0)TryTurboILIMTimer--;
	//�޼����⾯����ʱ
	if(RampCurrentRiseAttmTIM>0&&RampCurrentRiseAttmTIM<9)RampCurrentRiseAttmTIM++;
	//��������
	if(BattAlertTimer>0&&BattAlertTimer<(BatteryAlertDelay+1))BattAlertTimer++;
	}	
	
//���㼫����λ����������ֵ
void CalcTurboILIM(void)
	{
	if(Battery>3.6)TurboILIM=QueryCurrentGearILED(); //��ص�ѹ����3.6ʱ����Ŀ�����ȥȡ
	else TurboILIM=CalcIREFValue(25000); //��ص�ѹ�ͣ�������25A���
	}	
	
//������λʱ��̬���Լ�������ֵ�Ĺ���
void TurboLVILIMProcess(void)	
	{
	//��ص�ѹ���ͣ������˳�����
	if(IsBatteryFault)
		{
		StartBattAlertTimer();
		if(BattAlertTimer<BatteryFaultDelay)return;
		//ʱ�䵽����������
		BattAlertTimer=0;	
		SwitchToGear(IsRampEnabled?Mode_Ramp:Mode_High);
		}
	//��ص�ѹ�ͻ��ߴ��������������µ�����
	else if(IsBatteryAlert)
		{
		//�ڵ���RampUp�Ĺ������������������������������ǰ����ֵ����Ϊ��������
		if(IsCurrentRampUp&&CurrentBuf<QueryCurrentGearILED())
			{
		  CurrentBuf=TurboILIM;
			IsCurrentRampUp=1; //ǿ��set���λȷ����������ִֻ��һ��
			return;
			}
		//�ֵ��Ѿ����뼫��������ִ����������
		if(TryTurboILIMTimer)return;
		TurboILIM-=25;
		TryTurboILIMTimer=TurboILIMTryCDTime; //Ӧ�ö�ʱ�����͵�����ȴ�һ�����ж�
		//�жϵ����Ƿ����ڼ���������
		if(TurboILIM>CalcIREFValue(13000))return;
		//���Ե�13A��Ȼ�޷����㼫�����˳�����
		TurboILIM=CalcIREFValue(13000);
		SwitchToGear(IsRampEnabled?Mode_Ramp:Mode_High);
		}
	}

//��ص͵�����������
void BatteryLowAlertProcess(bool IsNeedToShutOff,ModeIdxDef ModeJump)
	{
	char Thr;
	bit IsChangingGear;
	//��ȡ�ֵ簴����״̬
	if(getSideKey1HEvent())IsChangingGear=1;
	else IsChangingGear=getSideKeyHoldEvent();
	//���Ƽ�ʱ����ͣ
	if(!IsBatteryFault) //���û�з�����ѹ����
		{
		Thr=BatteryAlertDelay; //û�й��Ͽ�����һ�㽵��
		//��ǰ�ڻ����׶λ���û�и澯��ֹͣ��ʱ��,��������
		if(!IsBatteryAlert||IsChangingGear)BattAlertTimer=0;
		else StartBattAlertTimer();
		}
  else //������ѹ�澯����������ʱ��
		{
	  Thr=BatteryFaultDelay;
		StartBattAlertTimer(); 
		}
	//��ǰģʽ��Ҫ�ػ�
	if(IsNeedToShutOff||IsChangingGear)
		 {
		 //��ص�ѹ���ڹػ���ֵ�㹻ʱ�䣬�����ر�
		 if(IsBatteryFault&&BattAlertTimer>Thr)ReturnToOFFState(); 
		 }
	//����Ҫ�ػ���������������
	else if(BattAlertTimer>Thr)
		 {
	   BattAlertTimer=0;//���ö�ʱ������ʼֵ
	   SwitchToGear(ModeJump); //��λ��ָ����λ
		 }
	}		

//�޼����⿪��ʱ�ָ���ѹ���������Ĵ���	
void RampRestoreLVProtToMax(void)
	{
	if(IsBatteryAlert||IsBatteryFault)return;
	if(BattState==Battery_Plenty)SysCfg.RampCurrentLimit=CurrentMode->Current; //��ص�������������״̬����λ��������
	}
	
//�޼�����ĵ͵�ѹ����
void RampLowVoltHandler(void)
	{
	if(!IsBatteryAlert&&!IsBatteryFault)//û�и澯
		{
		BattAlertTimer=0;
		if(BattState==Battery_Plenty) //��ص�������������״̬���������ӵ�������
			{
	    if(SysCfg.RampCurrentLimit<CurrentMode->Current)
				 {
			   if(!RampCurrentRiseAttmTIM)RampCurrentRiseAttmTIM=1; //������ʱ����ʼ��ʱ
				 else if(RampCurrentRiseAttmTIM<9)return; //ʱ��δ��
         RampCurrentRiseAttmTIM=1;
				 if(SysCfg.RampBattThres>CurrentMode->LowVoltThres)SysCfg.RampBattThres=CurrentMode->LowVoltThres; //��ѹ���ﵽ���ޣ���ֹ��������
				 else SysCfg.RampBattThres+=50; //��ѹ����ϵ�50mV
         if(SysCfg.RampCurrentLimit>CurrentMode->Current)SysCfg.RampCurrentLimit=CurrentMode->Current;//���ӵ���֮�������ֵ�Ƿ񳬳�����ֵ
				 else SysCfg.RampCurrentLimit+=250;	//�����ϵ�250mA		 
				 }
			else RampCurrentRiseAttmTIM=0; //�Ѵﵽ�������޽�ֹ��������
			}
		return;
		}
	else RampCurrentRiseAttmTIM=0; //������������λ�������ӵ����Ķ�ʱ��
	//��ѹ�澯������������ʱ��
	StartBattAlertTimer(); //��������������ʱ��
	if(IsBatteryFault&&BattAlertTimer>4)ReturnToOFFState(); //��ص�ѹ���ڹػ���ֵ����0.5�룬�����ر�
	else if(BattAlertTimer>BatteryAlertDelay) //��ص�λ����
		{
		if(SysCfg.RampCurrentLimit>CalcIREFValue(500))SysCfg.RampCurrentLimit-=250; //�����µ�250mA
		if(SysCfg.RampBattThres>2750)SysCfg.RampBattThres-=25; //����25mV
    BattAlertTimer=1;//���ö�ʱ��
		}
	}
