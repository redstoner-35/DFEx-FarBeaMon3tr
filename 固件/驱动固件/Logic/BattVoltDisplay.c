#include "ADCCfg.h"
#include "LEDMgmt.h"
#include "delay.h"
#include "ModeControl.h"
#include "SideKey.h"
#include "BattDisplay.h"
#include "SelfTest.h"
#include "LocateLED.h"

//�ڲ�flag
bit IsBatteryAlert; //��ص�ѹ���ھ���ֵ	
bit IsBatteryFault; //��ص�ѹ���ڱ���ֵ		

//�ڲ�����
static char BattShowTimer; //��ص�����ʾ��ʱ
static char OneLMShowBattStateTimer=0; //1LMģʽ����ʾ���״̬�ļ�ʱ��
static xdata AverageCalcDef BattVolt;	
static xdata signed char VshowTIM;  //��ѹ��ʾ��ʱ��
static char LowVoltStrobeTIM;
static xdata int VbattSample; //ȡ���ĵ�ص�ѹ

//�ⲿȫ�ֱ���
BattStatusDef BattState; //��ص������λ
xdata float Battery; //��Ч���ڵ�ص�ѹ
xdata BattVshowFSMDef VshowFSMState; //��ص�ѹ��ʾ����ļ�ʱ����״̬��ת��

//�ڲ�ʹ�õ��ȵ���ʾ��
static code LEDStateDef VShowIndexCode[]=
	{
	LED_Red,
	LED_Amber,
	LED_Green,  //���������Ǻ����
	LED_Amber,
	LED_Red  //�߾���ģʽ�Ƿ��������̺��
	};

//������ص�ѹ��ʾ
void TriggerVshowDisplay(void)	
	{
	if(VshowFSMState!=BattVdis_Waiting)return; //�ǵȴ���ʾ״̬��ֹ����
	VshowFSMState=BattVdis_PrepareDis;
	if(CurrentMode->ModeIdx!=Mode_OFF)
		{
		if(LEDMode!=LED_OFF)VshowTIM=8; //ָʾ�Ƶ���״̬��ѯ������Ϩ��LED��һ��
		LEDMode=LED_OFF;
		}
	}		

//���ɵ͵�����ʾ����
bit LowPowerStrobe(void)
	{
	//��������,������1LMģʽ����������ʱ
	if(CurrentMode->ModeIdx==Mode_1Lumen||BattState!=Battery_VeryLow)LowVoltStrobeTIM=0;
	//�����쳣��ʼ��ʱ
	else if(!LowVoltStrobeTIM)LowVoltStrobeTIM=1; //������ʱ��
	else if(LowVoltStrobeTIM>((LowVoltStrobeGap*8)-4))return 1; //������˸��ǵ���Ϊ0
	//�����������0
	return 0;
	}
	
//����LED�ఴ������˸ָʾ��ص�ѹ�Ĵ���
static void VshowGenerateSideStrobe(LEDStateDef Color,BattVshowFSMDef NextStep)
	{
	//���븺����ͨ������һ�α�ʾ��0
	if(VshowTIM&0x80)
		{
		MakeFastStrobe(Color);
		VshowTIM=0; 
		}
	//����ָʾ
	LEDMode=(VshowTIM%4)>1?Color:LED_OFF; //�����ɫ��˸ָʾ��Ӧλ�ĵ�ѹ
	//��ʾ����
	if(!VshowTIM) 
		{
		LEDMode=LED_OFF;
		VshowTIM=10;
		VshowFSMState=NextStep; //�ȴ�һ��
		}
	}
//��ѹ��ʾ״̬�����ݶ�Ӧ�ĵ�ѹλ���������˸��ʱ��������ֵ
static void VshowFSMGenTIMValue(int Vsample,BattVshowFSMDef NextStep)
	{
	if(!VshowTIM)	//ʱ�䵽��������
		{	
		if(!Vsample)VshowTIM=-1; //0=˲����һ��
		else VshowTIM=(4*Vsample)-1; //������ʾ��ʱ��
		VshowFSMState=NextStep; //ִ����һ����ʾ
		}
	}
	
//���ݵ��״̬������LEDָʾ��ص���
static void SetPowerLEDBasedOnVbatt(void)	
	{
	switch(BattState)
		{
		 case Battery_Plenty:LEDMode=LED_Green;break; //��ص���������ɫ����
		 case Battery_Mid:LEDMode=LED_Amber;break; //��ص����еȻ�ɫ����
		 case Battery_Low:LEDMode=LED_Red;break;//��ص�������
		 case Battery_VeryLow:LEDMode=LED_RedBlink;break; //��ص������ز����ɫ����
		}
	}

//���ֵ繤��ʱ����ϵͳ״̬��ʾ���״̬
static void ShowBatteryState(void)	
	{
	bit IsShowBatteryState;
	//��1LM��λ��������ʾ
	if(CurrentMode->ModeIdx!=Mode_1Lumen)IsShowBatteryState=1;
	//1LM��λ�������ص������ع��ͣ���ʾ
	else if(BattState==Battery_VeryLow)IsShowBatteryState=1;
	//1LM��λ�»��ڼ�ʱ��������ʾ����
	else
		{
		if(!OneLMShowBattStateTimer)OneLMShowBattStateTimer=82;
		IsShowBatteryState=OneLMShowBattStateTimer>2?0:1;
		}		
	//���ݽ��ѡ���Ƿ���ú�����ʾ����	
	if(IsShowBatteryState)SetPowerLEDBasedOnVbatt();
	else LEDMode=LED_OFF;  //����ʾ״̬��Ҫ����LEDϨ��
	}

//�����ϸ��ѹ��ʾ��״̬������
static void BatVshowFSM(void)
	{
	char Index;
	//������ʾ״̬��
	switch(VshowFSMState)
		{
		case BattVdis_PrepareDis: //׼����ʾ
			if(VshowTIM)break;
	    VshowTIM=15; //�ӳ�1.75��
			VshowFSMState=BattVdis_DelayBeforeDisplay; //��ʾͷ��
		  //���е�ѹȡ��(����ΪLSB=0.01V)
			VbattSample=(int)(Data.RawBattVolt*100); 
		  break;
		//�ӳٲ���ʾ��ͷ
		case BattVdis_DelayBeforeDisplay:
			if(VshowTIM>9)
				{
				Index=((VshowTIM-8)>>1)-1;
				if(VbattSample>999)Index+=2; //�����ѹ����10V��ʹ�ó�����ʾģʽ
				LEDMode=VShowIndexCode[Index];
				}
		  else LEDMode=LED_OFF; //�������˸֮��(����Ǹ߾�����ʾģʽ��Ϊ�̺��)�ȴ�
		  //ͷ����ʾ������ʼ��ʽ��ʾ��ѹ
		  if(VshowTIM>0)break;
			//��ص�ѹ������ʾ��Χ�������޷�
		  if(VbattSample>999)VbattSample/=10;
			//���ü�ʱ����ʾ��һ���ѹ
			VshowFSMGenTIMValue(VbattSample/100,BattVdis_Show10V);
		  break;
    //��ʾʮλ
		case BattVdis_Show10V:
			VshowGenerateSideStrobe(LED_Red,BattVdis_Gap10to1V); //���ô��������ɺ�ɫ�ಿ��˸
		  break;
		//ʮλ�͸�λ֮��ļ��
		case BattVdis_Gap10to1V:
			VbattSample%=100;
			VshowFSMGenTIMValue(VbattSample/10,BattVdis_Show1V); //���ü�ʱ����ʼ��ʾ��һ��	
			break;	
		//��ʾ��λ
		case BattVdis_Show1V:
		  VshowGenerateSideStrobe(LED_Amber,BattVdis_Gap1to0_1V); //���ô��������ɻ�ɫ�ಿ��˸
		  break;
		//��λ��ʮ��λ֮��ļ��		
		case BattVdis_Gap1to0_1V:	
			VshowFSMGenTIMValue(VbattSample%10,BattVdis_Show0_1V);
			break;
		//��ʾС�����һλ(0.1V)
		case BattVdis_Show0_1V:
		  VshowGenerateSideStrobe(LED_Green,BattVdis_WaitShowChargeLvl); //���ô�����������ɫ�ಿ��˸
			break;
		//�ȴ�һ��ʱ�����ʾ��ǰ����
		case BattVdis_WaitShowChargeLvl:
			if(VshowTIM>0)break;
			if(CurrentMode->ModeIdx==Mode_1Lumen)BattShowTimer=12; //1LMģʽ�µ���ָʾ�Ʋ���פ������������Ҫ���������ʱ��LED����
		  else BattShowTimer=CurrentMode->ModeIdx!=Mode_OFF?0:12; //�������������ʾ
			VshowFSMState=BattVdis_ShowChargeLvl; //�ȴ�������ʾ״̬����
      break;
	  //�ȴ����������ʾ����
		case BattVdis_ShowChargeLvl:
		  if(BattShowTimer)SetPowerLEDBasedOnVbatt(); //��ʾ����
			else if(!getSideKeyNClickAndHoldEvent())VshowFSMState=BattVdis_Waiting; //�û���Ȼ���°������ȴ��û��ɿ�,�ɿ���ص��ȴ��׶�
      break;
		}
	}
//��ص���״̬��
static void BatteryStateFSM(void)
	{
	float Thres;
	//���м�����ֵ����
	if(CurrentMode->ModeIdx!=Mode_Turbo)Thres=3.7;
	else Thres=3.5;
	//״̬������	
	switch(BattState) 
		 {
		 //��ص�������
		 case Battery_Plenty: 
				if(Battery<Thres)BattState=Battery_Mid; //��ص�ѹС��3.7���ص������ϵ�״̬
			  break;
		 //��ص�����Ϊ����
		 case Battery_Mid:
			  if(Battery>(Thres+0.2))BattState=Battery_Plenty; //��ص�ѹ����3.8���ص�����״̬
				if(Battery<3.0)BattState=Battery_Low; //��ص�ѹ����3.2���л��������͵�״̬
				break;
		 //��ص�������
		 case Battery_Low:
		    if(Battery>3.2)BattState=Battery_Mid; //��ص�ѹ����3.5���л��������еȵ�״̬
			  if(Battery<2.8)BattState=Battery_VeryLow; //��ص�ѹ����2.8���������ز���
		    break;
		 //��ص������ز���
		 case Battery_VeryLow:
			  if(Battery>3.0)BattState=Battery_Low; //��ص�ѹ������3.0����ת����������׶�
		    break;
		 }
	}

//��λ��ص�ѹ��⻺��
static void ResetBattAvg(void)	
	{
	BattVolt.Min=32766;
	BattVolt.Max=-32766; //��λ�����С������
	BattVolt.Count=0;
  BattVolt.AvgBuf=0; //���ƽ���������ͻ���
	}
	
//������ʱ��ʾ��ص�ѹ
void DisplayVBattAtStart(bit IsPOR)
	{
	char i=10;
	//��ʼ��ƽ��ֵ����,��λ��־λ
	ResetBattAvg();
  //��λ��ص�ѹ״̬�͵����ʾ״̬��
  VshowFSMState=BattVdis_Waiting;		
	do
		{
		SystemTelemHandler();
		Battery=Data.BatteryVoltage; //��ȡ�����µ�ص�ѹ
		BatteryStateFSM(); //����ѭ��ִ��״̬�����µ����յĵ��״̬
		}
	while(--i);
	//������ص�����ʾ(���޴���������)
	if(!IsPOR||CurrentMode->ModeIdx!=Mode_OFF)return;
	BattShowTimer=12;
	}
//��ص�����ʾ��ʱ�Ĵ���
void BattDisplayTIM(void)
	{
	long buf;
	//����ƽ��ģ�����
	if(BattVolt.Count<VBattAvgCount)		
		{
		buf=(long)(Data.BatteryVoltage*1000);
		BattVolt.Count++;
		BattVolt.AvgBuf+=buf;
		if(BattVolt.Min>buf)BattVolt.Min=buf;
		if(BattVolt.Max<buf)BattVolt.Max=buf; //��ֵ��ȡ
		}
	else //ƽ�������������µ�ѹ
		{
		BattVolt.AvgBuf-=(long)BattVolt.Min+(long)BattVolt.Max; //ȥ��������
		BattVolt.AvgBuf/=(long)(BattVolt.Count-2); //��ƽ��ֵ
		Battery=(float)BattVolt.AvgBuf/(float)1000; //�õ����յĵ�ص�ѹ
		ResetBattAvg(); //��λ����
		}
	//�͵�ѹ��ʾ��˸��ʱ��
	if(LowVoltStrobeTIM==LowVoltStrobeGap*8)LowVoltStrobeTIM=1;//ʱ�䵽�����ֵ���¼�ʱ
	else if(LowVoltStrobeTIM)LowVoltStrobeTIM++;
	//1LMģʽ�½�����ʾ�ļ�ʱ��
	if(OneLMShowBattStateTimer)OneLMShowBattStateTimer--;	
	//��ص�ѹ��ʾ�ļ�ʱ������	
	if(VshowTIM>0)VshowTIM--;
	//�����ʾ��ʱ��
	if(BattShowTimer)BattShowTimer--;
	}

//��ز���������ָʾ�ƿ���
void BatteryTelemHandler(void)
	{
	int AlertThr,VBatt;
	//���ݵ�ص�ѹ����flagʵ�ֵ͵�ѹ�����͹ػ�����
	if(CurrentMode->ModeIdx==Mode_Ramp)AlertThr=SysCfg.RampBattThres; //�޼�����ģʽ�£�ʹ�ýṹ���ڵĶ�̬��ֵ
	else AlertThr=CurrentMode->LowVoltThres; //�ӵ�ǰĿ�굲λ��ȡģʽֵ  
	VBatt=(int)(Battery*1000); //�õ���ص�ѹ(mV)
  if(VBatt>2650)		
		{
		IsBatteryAlert=VBatt>AlertThr?0:1; //����bit���ݸ�����λ����ֵ�����ж�
		IsBatteryFault=0; //��ص�ѹû�е���Σ��ֵ��fault=0
		}
	else
		{
		IsBatteryAlert=0; //����bit�����ǿ���������bit
		IsBatteryFault=1; //����bit=1
		}
	//��ص���ָʾ״̬��
	BatteryStateFSM();
	//LED����
	if(IsOneTimeStrobe())return; //Ϊ�˱������ֻ����һ�ε�Ƶ��ָʾ����ִ�п��� 
	if(ErrCode!=Fault_None)DisplayErrorIDHandler(); //�й��Ϸ�������ʾ����
	else if(VshowFSMState!=BattVdis_Waiting)BatVshowFSM();//��ص�ѹ��ʾ������ִ��״̬��
	else if(LocLEDState==LocateLED_Sel)LEDMode=LocateLED_ShowType(); //����LED�༭
	else if(CurrentMode->ModeIdx!=Mode_OFF||BattShowTimer)ShowBatteryState(); //�û���ѯ���������ֵ翪����ָʾ����
  else LEDMode=LED_OFF; //�ֵ紦�ڹر�״̬����û�а������µĶ�������LED����Ϊ�ر�
	}
	