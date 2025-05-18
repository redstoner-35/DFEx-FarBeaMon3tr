#include "SpecialMode.h"
#include "LEDMgmt.h"
#include "SideKey.h"
#include "LocateLED.h"
#include "BattDisplay.h"
#include "OutputChannel.h"
#include "SysConfig.h"
#include "ADCCfg.h"
#include "TempControl.h"
#include "LowVoltProt.h"
#include "SelfTest.h"
#include "SOS.h"
#include "Beacon.h"
#include "Strobe.h"

//�����ͱ�������ѡ��
//#define TurboCurrent32A  //ע�͵�����36A����������������Ϊ31.75A������FV7212D��
#define FullPowerStrobe //��������ȫ���ʱ���
#define FullPowerBeacon //ȫ�����ű�

//��λ�ṹ��
code ModeStrDef ModeSettings[ModeTotalDepth]=
	{
		//�ػ�״̬
    {
		Mode_OFF,
		0,
		0,  //����0mA
		0,  //�ػ�״̬��ֵΪ0ǿ�ƽ������
		true,
		false,
		}, 
		//������
		{
		Mode_Fault,
		0,
		0,  //����0mA
		0,
		false,
		false,
		}, 
		//�¹�
		{
		Mode_Moon,
		CalcIREFValue(20),  //ʵ����20
		0,   //��С����û�õ�������
		2500,  //2.5V�ض�
		false, //�¹⵵��ר����ڣ����������
		false,
		}, 	
		//������
		{
		Mode_ExtremelyLow,
		CalcIREFValue(200),  //200mA
		0,   //��С����û�õ�������
		2600,  //2.6V�ض�
		true, //������
		false,
		}, 	
    //����
		{
		Mode_Low,
		CalcIREFValue(1000),  //1000mA����
		0,   //��С����û�õ�������
		2800,  //2.8V�ض�
		true,
		false,
		},
    //����
		{
		Mode_Mid,
		CalcIREFValue(2000),  //2000mA����
		0,   //��С����û�õ�������
		2900,  //2.9V�ض�
		true,
		false,
		}, 	
    //�и���
		{
		Mode_MHigh,
		CalcIREFValue(4000),  //4000mA����
		0,   //��С����û�õ�������
		3000,  //3V�ض�
		true,
		true,
		}, 	
    //����
		{
		Mode_High,
		CalcIREFValue(8000),  //8000mA����
		0,   //��С����û�õ�������
		3100,  //3.1V�ض�
		true,
		true,
		}, 	
    //����
		{
		Mode_Turbo,
		#ifdef TurboCurrent32A
		CalcIREFValue(31750),  //31.75A����
		#else
    CalcIREFValue(36000),  //36A����(���7175)
	  #endif		
		0,   //��С����û�õ�������
		3400,  //3.4V�ض�
		false, //�������ܴ�����
		true,
		}, 	
    //����		
		{
		Mode_Strobe,
		#ifndef FullPowerStrobe		
		CalcIREFValue(22000),  //22A����
		#else
			//ȫ���ʱ�������
			#ifdef TurboCurrent31A
			CalcIREFValue(31750),  //31.75A����
			#else
			CalcIREFValue(36000),  //36A����(���7175)
			#endif		
    #endif			
		0,   //��С����û�õ�������
		2500,  //2.5V�ض�(ʵ����2.7�ͻ���բ���������2.5��Ϊ�˱���͵�ѹ�������������±��������쳣)
		false, //�������ܴ�����
		true,
		}, 
	  //�޼�����		
		{
		Mode_Ramp,
		CalcIREFValue(10000),  //��� 10000mA����
		CalcIREFValue(100),   //��С 100mA����
		3200,  //3.2V�ض�
		false, //���ܴ�����  
		true,
		}, 
		//�ű�ģʽ
		{
		Mode_Beacon,
		#ifdef FullPowerBeacon
			//ȫ�����ű�ģʽ����
			#ifdef TurboCurrent31A
			CalcIREFValue(31750),  //31.75A����
			#else
			CalcIREFValue(36000),  //36A����(���7175)
			#endif	
    #else	
			CalcIREFValue(22000),  //22A����
		#endif
		0,   //��С����û�õ�������
		2500,  //2.5V�ض�(ʵ����2.7�ͻ���բ��ʵ�����������2.5��Ϊ�˱���͵�ѹ��������������SOS״̬������SOS�����쳣)
		false,	//SOS���ܴ�����
		true,
		}, 
	  //SOS
		{
		Mode_SOS,
		CalcIREFValue(14000),  //14A����
		0,   //��С����û�õ�������
		2500,  //2.5V�ض�(ʵ����2.7�ͻ���բ��ʵ�����������2.5��Ϊ�˱���͵�ѹ��������������SOS״̬������SOS�����쳣)
		false,	//SOS���ܴ�����
		true,
		}, 
	};

//ȫ�ֱ���(��λ)
ModeStrDef *CurrentMode; //��λ�ṹ��ָ��
xdata ModeIdxDef LastMode; //��λ����洢
SysConfigDef SysCfg; //ϵͳ����	

//ȫ�ֱ���(״̬λ)
bit IsRampEnabled; //�Ƿ����޼�����
static bit IsNotifyMaxRampLimitReached=0; //����޼�����ﵽ������	
	
//�����ʱ����
xdata char HoldChangeGearTIM; //��λģʽ�³�������
xdata char DisplayLockedTIM; //������ս��ģʽ�����˳���ʾ	
static xdata char RampDIVCNT; //��Ƶ��ʱ��	
	
//��ȡ��������
static int QueryTurboCurrent(void)	
	{
	//����MPPT�����������Ӷ�̬������������ȡ��������
	if(TurboILIM<QueryCurrentGearILED())return TurboILIM;
	//����������ӵ�ǰ��λ�õ���	
	return QueryCurrentGearILED();  
	}

//��ʼ��ģʽ״̬��
void ModeFSMInit(void)
{
	char i;
	//��ʼ���޼�����
	SysCfg.RampLimitReachDisplayTIM=0;
  ReadSysConfig(); //��EEPROM�ڶ�ȡ�޼���������
  for(i=0;i<ModeTotalDepth;i++)if(ModeSettings[i].ModeIdx==Mode_Ramp) //������λ���ýṹ��Ѱ���޼�����ĵ�λ����ȡ����
		{
		SysCfg.RampBattThres=ModeSettings[i].LowVoltThres; //��ѹ������޻ָ�
		SysCfg.RampCurrentLimit=ModeSettings[i].Current; //�ҵ���λ�������޼�����ĵ�λ���������޻ָ�
		//��ȡ���ݽ����󣬼�����������Ƿ�Ϸ������Ϸ���ֱ������
		if(SysCfg.RampCurrent<ModeSettings[i].MinCurrent)SysCfg.RampCurrent=ModeSettings[i].MinCurrent;
		if(SysCfg.RampCurrent>SysCfg.RampCurrentLimit)SysCfg.RampCurrent=SysCfg.RampCurrentLimit;
		//��ȡ����������ѭ��
		break;
		}
	//��λ����
	RampDIVCNT=3; 	
	//��λģʽ����
	ResetSOSModule(); //��λSOSģ��
	LastMode=Mode_Low;
	ErrCode=Fault_None; //û�й���
	CurrentMode=&ModeSettings[0]; //��������Ϊ��һ����
}	

//��λ״̬������������ʱ������
void ModeFSMTIMHandler(void)
{
	//�޼�������صĶ�ʱ��
	if(SysCfg.CfgSavedTIM<32)SysCfg.CfgSavedTIM++;
	if(SysCfg.RampLimitReachDisplayTIM>0)
		{
		SysCfg.RampLimitReachDisplayTIM--;
		if(!SysCfg.RampLimitReachDisplayTIM)IsNotifyMaxRampLimitReached=0; //���޼�������ʾ��ʱ����Ϊ0֮�󣬸�λ��־λ
		}
	//����������ʾ��ʱ��
  if(DisplayLockedTIM>0)DisplayLockedTIM--;
}

//��λ��ת
void SwitchToGear(ModeIdxDef TargetMode)
	{
	char i;
  int LastICC;
	bool IsLastModeNeedStepDown;
	//��¼����ǰ�Ľ��
	ModeIdxDef BeforeMode=CurrentMode->ModeIdx; 			
	IsLastModeNeedStepDown=CurrentMode->IsNeedStepDown; //�����Ƿ���Ҫ����
	if(CurrentMode->ModeIdx==Mode_Turbo)LastICC=QueryTurboCurrent(); //����Ǽ�����λ����Ҫȡ��ǰ�ĵ���������Ϊ���յ���
	else LastICC=CurrentMode->Current; //�洢����֮ǰ�ĵ�λ�͵���ֵ
	//��ʼѰ��
	for(i=0;i<ModeTotalDepth;i++)if(ModeSettings[i].ModeIdx==TargetMode)
		{
		//��λ���⹦�ܵ�λ����ʼ״̬
    ResetSOSModule();		//��λ����SOSģ��
		BeaconFSM_Reset(); //��λ�����ű�ģ��
		ResetStrobeModule(); //��λ��������
		
		//�ҵ�ƥ��index������Ӧ�Ľṹ�����ַ��ֵ��ָ��
		CurrentMode=&ModeSettings[i]; 
		//���л���֮����¿ؽ��Ӻ����¼��㼫����������
		if(BeforeMode!=Mode_Turbo&&TargetMode==Mode_Turbo)CalcTurboILIM(); //�ӷǼ�����λ���뼫�����¼������ֵ������MPPTϵͳ
		if(IsLastModeNeedStepDown)RecalcPILoop(LastICC); //��������PI�������������
		//���ҵ�Ŀ�굲λ���˳�ѭ��
		break;
		}
	}
	
//�����ػ�����	
void ReturnToOFFState(void)
	{
	if(CurrentMode->ModeIdx==Mode_OFF)return; //�ػ�״̬��ִ��		
	if(CurrentMode->IsModeHasMemory)LastMode=CurrentMode->ModeIdx; //�洢�ػ�ǰ�ĵ�λ
	SwitchToGear(Mode_OFF); //ǿ�����ص��ػ���λ
	}	

//���������ļ����������
void HoldSwitchGearCmdHandler(void)
	{
	char buf;
	if(SysMode||CurrentMode->ModeIdx==Mode_Ramp)HoldChangeGearTIM=0; //ս��ģʽ���߽����������Լ�λ���޼�����ģʽ�£���ֹ����ϵͳ����
	else if(!getSideKeyHoldEvent()&&!getSideKey1HEvent())HoldChangeGearTIM=0; //�����ɿ�����ʱ����λ
	else //ִ�л�������
		{
		buf=HoldChangeGearTIM&0x1F; //ȡ��TIMֵ
		if(buf==0&&!(HoldChangeGearTIM&0x40))HoldChangeGearTIM|=getSideKey1HEvent()?0x20:0x80; //�������λ1ָʾ�������Լ���
		HoldChangeGearTIM&=0xE0; //ȥ����ԭʼ��TIMֵ
		if(buf<HoldSwitchDelay&&!(HoldChangeGearTIM&0x40))buf++;
		else buf=0;  //ʱ�䵽��������
		HoldChangeGearTIM|=buf; //����ֵд��ȥ
		}
	}	

//�ఴ������������ִ��
static void SideKeySwitchGearHandler(ModeIdxDef TargetMode)	
	{
	if(!(HoldChangeGearTIM&0x80))return;
	HoldChangeGearTIM&=0x7F; //������λ��Ǳ��λ������
  SwitchToGear(TargetMode); //����Ŀ�굲λ
	}
	
//�ఴ����+�����������˲���ִ��
static void SideKey1HRevGearHandler(ModeIdxDef TargetMode)
	{
	if(!(HoldChangeGearTIM&0x20))return;
	HoldChangeGearTIM&=0xDF; //������λ��Ǳ��λ������
	SwitchToGear(TargetMode); //����Ŀ�굲λ
	}	
	
//�޼����⴦��
static void RampAdjHandler(void)
	{
	static bit IsKeyPressed=0;	
  int Limit;
	bit IsPress;
  //������޼���������
	IsPress=(getSideKey1HEvent()||getSideKeyHoldEvent())?1:0;
	Limit=SysCfg.RampCurrentLimit<CurrentMode->Current?SysCfg.RampCurrentLimit:CurrentMode->Current;
	if(Limit<CurrentMode->Current&&IsPress&&SysCfg.RampCurrent>Limit)SysCfg.RampCurrent=Limit; //�ڵ��������Ƶ�������û����°������Ե��������������޷�
	//�������ȵ���
	if(getSideKeyHoldEvent()&&!IsKeyPressed) //�������ӵ���
			{	
			if(RampDIVCNT>0)RampDIVCNT--;
			else 
				{
				//ʱ�䵽����ʼ���ӵ���
				if(SysCfg.RampCurrent<Limit)SysCfg.RampCurrent++;
				else
					{
					IsNotifyMaxRampLimitReached=1; //����Ѵﵽ����
					SysCfg.RampLimitReachDisplayTIM=4; //Ϩ��0.5��ָʾ�Ѿ�������
					SysCfg.RampCurrent=Limit; //���Ƶ������ֵ	
					IsKeyPressed=1;
					}
				//��ʱʱ�䵽����λ����
				RampDIVCNT=3;
				}
			}	
	else if(getSideKey1HEvent()&&!IsKeyPressed) //����+�������ٵ���
		 {
			if(RampDIVCNT>0)RampDIVCNT--;
			else
				{
				if(SysCfg.RampCurrent>CurrentMode->MinCurrent)SysCfg.RampCurrent--; //���ٵ���	
				else
					{
					IsNotifyMaxRampLimitReached=0;
					SysCfg.RampLimitReachDisplayTIM=4; //Ϩ��0.5��ָʾ�Ѿ�������
					SysCfg.RampCurrent=CurrentMode->MinCurrent; //���Ƶ�����Сֵ
					IsKeyPressed=1;
					}
				//��ʱʱ�䵽����λ����
				RampDIVCNT=3;
				}
		 }
  else if(!IsPress&&IsKeyPressed)
		{
	  IsKeyPressed=0; //�û��ſ��������������		
		RampDIVCNT=3; //��λ��Ƶ��ʱ��
		}
	//�������ݱ�����ж�
	if(IsPress)SysCfg.CfgSavedTIM=0; //��������˵�����ڵ�������λ��ʱ��
	else if(SysCfg.CfgSavedTIM==32)
			{
			SysCfg.CfgSavedTIM++;
			SaveSysConfig(0);  //һ��ʱ����û����˵���Ѿ�������ϣ���������
			}
	}

//����Ƿ���Ҫ�ػ�
static void DetectIfNeedsOFF(int ClickCount)
	{
	if(getSideKeyNClickAndHoldEvent()==2)TriggerVshowDisplay();
	if(!SysMode&&ClickCount!=1)return;
	if(SysMode&&getSideKeyHoldEvent())return;
	ReturnToOFFState();//�ఴ����������ս��ģʽ���ɿ���ťʱ�ػ�
	}	

//��λ״̬��
void ModeSwitchFSM(void)
	{
	char ClickCount;
	//��ȡ����״̬
	if(GetIfSystemInPOFFSeq())return; //ϵͳ���ڹػ������У���ִ�а�������
	ClickCount=getSideKeyShortPressCount(0);	//��ȡ�����������������Ĳ���
	//��λ�����������EEPROM����
	if(LastMode==Mode_OFF||LastMode>=ModeTotalDepth)LastMode=Mode_Low;
	//״̬��
	IsHalfBrightness=0; //������Ĭ��ȫ��
	switch(CurrentMode->ModeIdx)	
		{
		//���ִ���	
		case Mode_Fault:
      SysMode=Operation_Normal; //���Ϻ��Զ��ص���ͨģʽ			
			if(!getSideKeyLongPressEvent()||IsErrorFatal())break; //�û�û�а��°�ť�����������Ĵ���״̬����������
			ClearError(); //��������ǰ����
		  break;
		//�ػ�״̬
		case Mode_OFF:		  
			//�������⹦��
		  if(LocLEDState==LocateLED_NotEdit)
				{
				SpecialModeOperation(ClickCount);  //ֻ�����˳��˶�λLED�༭ģʽ֮�����ִ��
				if(SysMode)break;
				}
		  //����λLED���
			if(LocateLED_Edit(ClickCount))break;
		  //������ģʽ�����������ػ�������
			if(ClickCount==1)PowerToNormalMode(LastMode); //�ఴ������������ѭ��	
			//���뼫���ͱ���
			else EnterTurboStrobe(ClickCount);		
      if(getSideKeyLongPressEvent())SwitchToGear(Mode_Moon); //��������ֱ�ӽ��¹�					
			if(ClickCount==4) //�Ļ��л���λģʽ���޼�����
					{	
					IsRampEnabled=~IsRampEnabled; //ת���޼�����״̬	
					LEDMode=IsRampEnabled?LED_GreenBlinkThird:LED_RedBlinkThird; //��ʾ�Ƿ���
					SaveSysConfig(0); //�������õ�ROM��
					}
		  //��ѯ��ѹ
			if(getSideKeyNClickAndHoldEvent())TriggerVshowDisplay();
  		break;
		//�¹�״̬
		 case Mode_Moon:
			 IsHalfBrightness=1; //�¹�ģʽ���������ȼ���
			 BatteryLowAlertProcess(true,Mode_Moon);
		   DetectIfNeedsOFF(ClickCount); //ִ�йػ��������	
			 //��ص�ѹ���㣬�������������λ
		   if(getSideKeyLongPressEvent())  
					{
					PowerToNormalMode(Mode_ExtremelyLow); //������������ģʽ
					if(CurrentMode->ModeIdx==Mode_Moon)break;//����֮���޷��ɹ��뿪�¹�ģʽ������������ĸ�λ����
					if(IsRampEnabled)RestoreToMinimumSysCurrent(); //������޼�������ָ�����͵���
					HoldChangeGearTIM|=0x40; //��ֹ����ϵͳ����
					}		    
		    break;			
    //�޼�����״̬				
    case Mode_Ramp:
			  DetectIfNeedsOFF(ClickCount); //����Ƿ���Ҫ�ػ�
				EnterTurboStrobe(ClickCount); //���뼫�����߱����ļ��
		    //�޼����⴦��
		    RampLowVoltHandler(); //�͵�ѹ����
        RampAdjHandler();			
		    break;
		//������
    case Mode_ExtremelyLow:					
				BatteryLowAlertProcess(true,Mode_ExtremelyLow);
				DetectIfNeedsOFF(ClickCount); //ִ�йػ��������
		    EnterTurboStrobe(ClickCount); //���뼫�����߱����ļ��
				SideKeySwitchGearHandler(Mode_Low); //�����͵�
		    break;	    		
    //����״̬		
    case Mode_Low:
			  BatteryLowAlertProcess(false,Mode_ExtremelyLow);
		    DetectIfNeedsOFF(ClickCount); //ִ�йػ��������
				EnterTurboStrobe(ClickCount); //���뼫�����߱����ļ��
		    //������������
				SideKey1HRevGearHandler(Mode_ExtremelyLow); //����+�������˵�λ�����͵�
		    SideKeySwitchGearHandler(Mode_Mid); //�����е�
		    break;	    		
    //����״̬		
    case Mode_Mid:
			  BatteryLowAlertProcess(false,Mode_Low);
		    DetectIfNeedsOFF(ClickCount); //ִ�йػ��������
				EnterTurboStrobe(ClickCount); //���뼫�����߱����ļ��
		    //������������
		    SideKeySwitchGearHandler(Mode_MHigh); //�����иߵ�
		    SideKey1HRevGearHandler(Mode_Low); //����+�������˵�λ���͵�
		    break;	
	  //�и���״̬
    case Mode_MHigh:
			  BatteryLowAlertProcess(false,Mode_Mid);
		    DetectIfNeedsOFF(ClickCount); //ִ�йػ��������
				EnterTurboStrobe(ClickCount); //���뼫�����߱����ļ��
		    //������������
		    SideKeySwitchGearHandler(Mode_High); //�����ߵ�
		    SideKey1HRevGearHandler(Mode_Mid); //����+�������˵�λ���е�
		    break;	
	  //����״̬
    case Mode_High:
			  BatteryLowAlertProcess(false,Mode_MHigh);
		    DetectIfNeedsOFF(ClickCount); //ִ�йػ��������
				EnterTurboStrobe(ClickCount); //���뼫�����߱����ļ��
		    //������������
		    SideKeySwitchGearHandler(Mode_ExtremelyLow); //�������͵�λ����ѭ��  
		    SideKey1HRevGearHandler(Mode_MHigh); //����+�������˵�λ���иߵ�
		    break;
		//����״̬
    case Mode_Turbo:
				TurboLVILIMProcess(); //ִ�м����͵������
		    DetectIfNeedsOFF(ClickCount); //ִ�йػ��������
		    SideKeySwitchGearHandler(Mode_High); //�����˻ظߵ� 
			  if(ClickCount==2||IsForceLeaveTurbo)SwitchToGear(IsRampEnabled?Mode_Ramp:Mode_Low); //˫�������¶ȴﵽ����ֵ��ǿ�Ʒ��ص�����
				if(ClickCount==3)SwitchToGear(Mode_Strobe); //�ఴ3�����뱬��
		    break;	
		//����״̬
    case Mode_Strobe:
			  BatteryLowAlertProcess(true,Mode_Strobe);
		    DetectIfNeedsOFF(ClickCount); //ִ�йػ��������
		    LeaveSpecialMode(ClickCount); //�˳�����ģʽ�ص������ط������
		    //������������
		    SideKeySwitchGearHandler(Mode_SOS); //�����л���SOS
		    break;	
    //SOS��ȵ�λ		
		case Mode_SOS:
			  BatteryLowAlertProcess(true,Mode_SOS);
		    DetectIfNeedsOFF(ClickCount); //ִ�йػ��������
			  LeaveSpecialMode(ClickCount); //�˳�����ģʽ�ص������ط������
		    //������������
		    SideKeySwitchGearHandler(Mode_Beacon); //�����л����ű�
		    break;	
		//�ű굲λ
		case Mode_Beacon:
			  BatteryLowAlertProcess(true,Mode_Beacon);
		    DetectIfNeedsOFF(ClickCount); //ִ�йػ��������
			  LeaveSpecialMode(ClickCount); //�˳�����ģʽ�ص������ط������
		    //������������
		    SideKeySwitchGearHandler(Mode_Strobe); //�����л�������
		    break;				
		}
  //Ӧ���������
	if(DisplayLockedTIM||IsDisplayLocked)Current=CalcIREFValue(50); //�û���������˳���������50mA���ݵ�����ʾһ��
	else switch(CurrentMode->ModeIdx)	
		{
		case Mode_Turbo:Current=QueryTurboCurrent();break; //����ģʽ
		case Mode_Beacon: //�ű�ģʽ			
		case Mode_SOS: 
		case Mode_Strobe://����ģʽ��SOSģʽ	     
	     switch(BattState)//ȡ����λ����
				 {
				 case Battery_Plenty:Current=QueryCurrentGearILED();break;
			   case Battery_Mid:Current=CalcIREFValue(10000);break;
         case Battery_Low:Current=CalcIREFValue(8000);break;
				 case Battery_VeryLow:Current=CalcIREFValue(2000);break;
				 }
			 //����״̬���Ƶ���
			 if(CurrentMode->ModeIdx==Mode_Strobe&&!StrobeOutputHandler())Current=-1; 
			 if(CurrentMode->ModeIdx==Mode_SOS&&!SOSFSM())Current=-1;
			 if(CurrentMode->ModeIdx==Mode_Beacon)switch(BeaconFSM())
				 {
				 case 0:Current=-1;break; //0��ʾ�õ����ر�
				 case 2:Current=CalcIREFValue(200);break; //��200mA������ʾ��֪�û��ѽ����ű�ģʽ
				 case 1:break; //����1�������κδ���
				 }
		   break; 
		//����ģʽ������ȡ����ֵ
		default:
		  if(LowPowerStrobe())Current=-1; //������ѹ��������˸
			else if(CurrentMode->ModeIdx==Mode_Ramp)Current=SysCfg.RampCurrentLimit<SysCfg.RampCurrent?SysCfg.RampCurrentLimit:SysCfg.RampCurrent; //�޼�����ģʽȡ�ṹ��������
		  else Current=QueryCurrentGearILED();//������λʹ������ֵ��ΪĿ�����
		}
  //�޼�����ģʽָʾ(�޼�����ģʽ�ڵִ������޺����Ϩ����ߵ���33%)
	if(SysCfg.RampLimitReachDisplayTIM)Current=IsNotifyMaxRampLimitReached?Current/3:-1;
	//�����������
	getSideKeyShortPressCount(1); 
	}