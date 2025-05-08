#include "GUI.h"
#include "CapTest.h"
#include "Config.h"
#include "ADC.h"
#include "IP2366_REG.h"
#include <math.h>
#include <string.h>
#include "Key.h"

//����ϵͳ״̬����enum
typedef enum
	{
	CapTest_Initial, //��״̬
	CapTest_WaitTypeCInsert, //�ȴ�Type-C����
	CapTest_Running,
	CapTest_Finish, //�����к���������
	CapTest_ConfirmFull, //ȷ���Ƿ����
	CapTest_OverCharge, //��⵽����
	CapTest_EndERROR, //����δ�ɹ����
	//ǿ����ֹ����	
	CapTest_ConfirmForceStopTest, //���û�ȷ���Ƿ�ǿ����ֹ
	//���ݳ���
	CapTest_ErrorAlreadyCharging,
	CapTest_ErrorBattToHigh,
	CapTest_ErrorDischarging,
	CapTest_ErrorChipHang,
	}CapTestFSMDef;
	
//�ⲿ����
extern IP2366VBUSStateDef VBUS;
extern BatteryStateDef BATT;
extern bool Is2368Telem;	
extern bool IsTelemOK;
extern const unsigned char TellUserToInsertTypeC[3158];
	
//�ڲ�����
static CapTestFSMDef CFSMState;
static float VBattSumbuf;
static float IBattsumbuf;
static char AverageCounter;	
static short ConfirmTimeCounter=0;
static bool IsUpDateGUI;
static char WaitBackToContinue=0;
	
//���ò���ϵͳ
void ResetCapTestSystem(void)
	{
	IsUpDateGUI=false;
	CFSMState=CapTest_Initial;
	VBattSumbuf=0;
	IBattsumbuf=0;
	WaitBackToContinue=0;
	AverageCounter=8;
	//���õ�ǰ�Ĳ��ݽ��
	CurrentTestResult.Data.ChargeTime=0;
	CurrentTestResult.Data.IsDataValid=false;
	CurrentTestResult.Data.MaxChargeCurrent=0;
	CurrentTestResult.Data.MaxChargeRatio=0;
	CurrentTestResult.Data.MaxChargeTemp=-100;
	CurrentTestResult.Data.TotalmAH=0;
	CurrentTestResult.Data.MaxVbatt=0;
	CurrentTestResult.Data.StartVbatt=0;
	CurrentTestResult.Data.TotalWh=0;
	}
	
//����ϵͳ�ۼӴ���
void CTestAverageACC(void)
	{
	extern bool OCState;
	//�жϲ�����ϼ�ʱ���ۼ�	
	if(ConfirmTimeCounter>0)ConfirmTimeCounter--;	
	//����ϵͳû�м����ֹң��
	if(CFSMState!=CapTest_WaitTypeCInsert&&CFSMState!=CapTest_Running&&CFSMState!=CapTest_ConfirmForceStopTest)return;
	Is2368Telem=true;
	if(WaitBackToContinue>0)WaitBackToContinue--;
	//��⵱ǰ2366�Ƿ��ڳ��
	if(!IsTelemOK)return; //���ݻ�ȡʧ��
	if(BATT!=Batt_PreChage&&BATT!=Batt_CCCharge&&BATT!=Batt_CVCharge)return;
	if(OCState)return; //ϵͳ���䣬��ͣͳ��
  //��ʼͳ��
	if(AverageCounter>0)
		{
		VBattSumbuf+=ADCO.Vbatt;
		IBattsumbuf+=ADCO.Ibatt;
		AverageCounter--;
		}
	else
		{
		//�����ѹ�͵���ƽ�����
		VBattSumbuf/=(float)8;
		IBattsumbuf/=(float)8;
		//���е�ѹ����
		if(CurrentTestResult.Data.StartVbatt==0&&IBattsumbuf>0.15)CurrentTestResult.Data.StartVbatt=VBattSumbuf; //�ɹ���ʼ��磬ץȡһ�ν��
		//����ʱ���ۼ�
		CurrentTestResult.Data.ChargeTime++;
		//�ɼ���ߵ�ѹ���¶�
		if(CurrentTestResult.Data.MaxVbatt<VBattSumbuf)CurrentTestResult.Data.MaxVbatt=VBattSumbuf;
		if(CurrentTestResult.Data.MaxChargeCurrent<IBattsumbuf)CurrentTestResult.Data.MaxChargeCurrent=IBattsumbuf;
		if(!ADCO.IsNTCOK)CurrentTestResult.Data.MaxChargeTemp=-100;
		else if(CurrentTestResult.Data.MaxChargeTemp<ADCO.Systemp)CurrentTestResult.Data.MaxChargeTemp=ADCO.Systemp;
		//����mAH��Wh
		CurrentTestResult.Data.TotalmAH+=(fabsf(IBattsumbuf)*1000)/(float)3600; //��ǰ����*1000 /3600��õ�mAH
		CurrentTestResult.Data.TotalWh+=fabsf(VBattSumbuf*IBattsumbuf)/(float)3600; //��ǰ��ѹ*����/3600��õ�Wh
		//���ƽ������׼���ٴβɼ�
		VBattSumbuf=0;
		IBattsumbuf=0;
		AverageCounter=8;
		//�ɼ���ϣ�����GUI
		IsUpDateGUI=false;
		}
	}

//��������
void CTestKeyHandler(void)
	{
	switch(CFSMState)
		{
		case CapTest_Initial:break; //û���κβ���
		case CapTest_WaitTypeCInsert:
			if(KeyState.KeyEvent==KeyEvent_ESC)CFSMState=CapTest_EndERROR; //�ڵȴ�Type-C�׶�ǿ���˳�
		  break;
		//�����н���ȷ��
	  case CapTest_ConfirmFull:
		case CapTest_Running: 
		case CapTest_OverCharge:
			if(KeyState.KeyEvent!=KeyEvent_ESC)break; //�����˳�
		  WaitBackToContinue=80; //�ӳ�80����޲��������
			CFSMState=CapTest_ConfirmForceStopTest; 
		  break;
		case CapTest_EndERROR:
		case CapTest_ErrorBattToHigh:
		case CapTest_ErrorAlreadyCharging:
		case CapTest_ErrorChipHang:
		case CapTest_ErrorDischarging:
		case CapTest_Finish:
			if(KeyState.KeyEvent!=KeyEvent_ESC)break;
			if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
			else SwitchingMenu(&SetMainMenu); //�����˳�״̬,����ESC��ص����˵�
		  break;	  
		//ȷ���˳�
		case CapTest_ConfirmForceStopTest:
			if(KeyState.KeyEvent==KeyEvent_BothEnt)CFSMState=CapTest_EndERROR; //ǿ���˳�����
		  else if(!WaitBackToContinue||KeyState.KeyEvent!=KeyEvent_None)CFSMState=CapTest_Running; //��������
		}
	//��������¼�
	KeyState.KeyEvent=KeyEvent_None;
	}	

//GUI����
void CTestGUIHandler(void)
	{
	unsigned long time;
	float Power;
	u16 Color;
	int Temp;
	extern bool IsDispChargingINFO;
	//��ִ����Ⱦ
	if(IsUpDateGUI&&KeyState.KeyEvent==KeyEvent_None)return;
	RenderMenuBG(); //��ʾ����
	switch(CFSMState)
		{
		//��ʼ��
		case CapTest_Initial:
				LCD_ShowChinese(27,33,"����ϵͳ��������",CYAN,LGRAY,0);
		    LCD_ShowChinese(46,47,"���Ժ󡭡�",CYAN,LGRAY,0);
		    break;
	  //ָʾ���ӳ����
		case CapTest_WaitTypeCInsert:
			  LCD_ShowPicture(40,21,75,21,TellUserToInsertTypeC);
			  LCD_ShowChinese(27,47,"�뽫��ؼ����ӵ�",WHITE,LGRAY,0);
		    LCD_ShowString(36,61,"Type-C",WHITE,LGRAY,12,0);
		    LCD_ShowChinese(81,61,"�����",WHITE,LGRAY,0);
		    break;
	  //ϵͳ���ڵȴ������ж� 
		case CapTest_ConfirmFull:
		    LCD_ShowChinese(20,19,"����ȷ�ϳ���״̬��",WHITE,LGRAY,0);
				LCD_ShowChinese(46,47,"���Ժ󡭡�",WHITE,LGRAY,0);
		    break;
		case CapTest_OverCharge:
		    LCD_ShowChinese(20,19,"ϵͳ��⵽�����¼�",YELLOW,LGRAY,0);
				LCD_ShowChinese(20,47,"��ȴ��¼��������",WHITE,LGRAY,0);
		    break;			 
		//���ݽ�����
		case CapTest_Running:
			  LCD_ShowChinese(20,19,"�����ݽ�����",CYAN,LGRAY,0);
		    //Ӫ���ʡ�Ժŷ����仯��Ч��ָʾ��������
	      time=CurrentTestResult.Data.ChargeTime&0x03;
        switch(time)
					{
					case 0:LCD_ShowChinese12x12(111,19,"��\0",CYAN,LGRAY,12,0);;break;
					case 1:LCD_ShowChinese(111,19,"����\0",CYAN,LGRAY,0);break;
					default : break;
					}
		    //��ʾ���ʱ��
		    LCD_ShowChinese(3,35,"���ʱ��",WHITE,LGRAY,0);
		    if(CurrentTestResult.Data.ChargeTime>86400) //���ʱ������һ��
					{
					time=CurrentTestResult.Data.ChargeTime/86400; //���������
					LCD_ShowIntNum(60,35,time,2,YELLOW,LGRAY,12);
					LCD_ShowChinese12x12(79,35,"��\0",WHITE,LGRAY,12,0);
					time=(CurrentTestResult.Data.ChargeTime%86400)/3600; //�����Сʱ��
					LCD_ShowIntNum(93,35,time,2,YELLOW,LGRAY,12);
					LCD_ShowChinese12x12(111,35,"ʱ\0",WHITE,LGRAY,12,0);
					time=(CurrentTestResult.Data.ChargeTime%3600)/60; //�����������
					LCD_ShowIntNum(124,35,time,2,YELLOW,LGRAY,12);
					LCD_ShowChinese12x12(143,35,"��\0",WHITE,LGRAY,12,0);		  
					}
				else //ʹ��ʱ����
					{
					time=CurrentTestResult.Data.ChargeTime/3600; //�����Сʱ��
					LCD_ShowIntNum(60,35,time,2,YELLOW,LGRAY,12);
					LCD_ShowChinese12x12(79,35,"ʱ\0",WHITE,LGRAY,12,0);
					time=(CurrentTestResult.Data.ChargeTime%3600)/60; //�����������
					LCD_ShowIntNum(93,35,time,2,YELLOW,LGRAY,12);
					LCD_ShowChinese12x12(111,35,"��\0",WHITE,LGRAY,12,0);
					time=CurrentTestResult.Data.ChargeTime%60;
					LCD_ShowIntNum(124,35,time,2,YELLOW,LGRAY,12);
					LCD_ShowChinese12x12(143,35,"��\0",WHITE,LGRAY,12,0);		  
					}
		    //��ʾ��繦�ʺ�Wh��
		    LCD_ShowChinese(3,50,"����",WHITE,LGRAY,0);
		    if(CurrentTestResult.Data.TotalWh<10)LCD_ShowFloatNum1(33,50,CurrentTestResult.Data.TotalWh,1,CYAN,LGRAY,12);
				else LCD_ShowIntNum(33,50,iroundf(CurrentTestResult.Data.TotalWh),3,CYAN,LGRAY,12);
		    LCD_ShowString(60,50,"Wh",CYAN,LGRAY,12,0);
				if(!IsDispChargingINFO)
					{
					LCD_ShowChinese(87,50,"����",WHITE,LGRAY,0);
					Power=fabsf(ADCO.Vbatt*ADCO.Ibatt);
					if(Power<10)LCD_ShowFloatNum1(115,50,Power,1,GREEN,LGRAY,12);
					else LCD_ShowIntNum(115,50,iroundf(Power),3,GREEN,LGRAY,12);
					LCD_ShowChar(147,50,'W',GREEN,LGRAY,12,0);
					}
				else
					{
					LCD_ShowChinese(87,50,"��ѹ",WHITE,LGRAY,0);
					LCD_ShowFloatNum1(115,50,ADCO.Vbatt,1,GREEN,LGRAY,12);
					LCD_ShowChar(147,50,'V',GREEN,LGRAY,12,0);
					}
				//��ʾAh��
				LCD_ShowChinese(3,64,"����",WHITE,LGRAY,0);
				if(CurrentTestResult.Data.TotalmAH<100)
					{
					LCD_ShowIntNum(33,64,iroundf(CurrentTestResult.Data.TotalmAH),2,RED,LGRAY,12);
					LCD_ShowString(51,64,"mAh",RED,LGRAY,12,0);
					}
				else if(CurrentTestResult.Data.TotalmAH<10000) //ʹ��0.1Ah��ʾ
					{
					LCD_ShowFloatNum1(33,64,CurrentTestResult.Data.TotalmAH/(float)1000,1,RED,LGRAY,12);
					LCD_ShowString(60,64,"Ah",RED,LGRAY,12,0);
					}					
				else //��������10Ah��ʹ������Ah��ʾ
					{
					LCD_ShowIntNum(33,64,iroundf(CurrentTestResult.Data.TotalmAH/1000),3,RED,LGRAY,12);
					LCD_ShowString(60,64,"Ah",RED,LGRAY,12,0);
					}
			 //�¶���ʾ
			LCD_ShowChinese(87,64,"�¶�",WHITE,LGRAY,0);
			if(!ADCO.IsNTCOK)LCD_ShowString(115,64,"---",WHITE,LGRAY,12,0);
			else
				{
				Temp=iroundf(ADCO.Systemp);
				if(Temp<0)Color=DARKBLUE;	
				else if(Temp<10)Color=BLUE;
				else if(Temp<CfgData.OverHeatLockTemp-20)Color=GREEN;
				else if(Temp<CfgData.OverHeatLockTemp-8)Color=YELLOW;
				else Color=RED;
				//�����¶ȣ���ʾΪ����ʶ��
				if(Temp<0)
					{
					Temp*=-1;
					LCD_ShowChar(117,61,'-',Color,LGRAY,12,0);
					LCD_ShowIntNum(124,64,Temp,2,Color,LGRAY,12);
					}		
				//��λ���¶ȣ�ʹ�ø�����ʾ
				else if(Temp<10)LCD_ShowFloatNum1(115,64,ADCO.Systemp,1,Color,LGRAY,12);		
				//�����¶ȣ�������ʾ
				else LCD_ShowIntNum(115,64,Temp,2,Color,LGRAY,12);
				}
			//��ʾ�����
			LCD_ShowChinese12x12(143,64,"��\0",Color,LGRAY,12,0);	
		  break;
		//���н���
		case CapTest_Finish:
			LCD_ShowChinese(33,22,"�������������",GREEN,LGRAY,0);
		  LCD_ShowChinese(24,41,"������",GREEN,LGRAY,0);
		  Power=CurrentTestResult.Data.TotalmAH/1000;
		  if(Power<10)LCD_ShowFloatNum1(67,41,Power,3,WHITE,LGRAY,12);
		  else if(Power<100)LCD_ShowFloatNum1(67,41,Power,2,WHITE,LGRAY,12);
		  else if(Power<1000)LCD_ShowFloatNum1(67,41,Power,1,WHITE,LGRAY,12);
		  else LCD_ShowIntNum(67,41,iroundf(Power),4,WHITE,LGRAY,12);
		  LCD_ShowString(112,41,"Ah",WHITE,LGRAY,12,0);
		  LCD_ShowChinese(32,61,"����",WHITE,LGRAY,0);
		  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,61,"���˳�",WHITE,LGRAY,0);
		  break;
	  //ȷ���˳�
		case CapTest_ConfirmForceStopTest:
			LCD_ShowChinese(4,22,"ǿ�ƽ������β��ݲ��˳�",RED,LGRAY,0);
		  LCD_ShowChar(147,22,'?',RED,LGRAY,12,0);
			LCD_ShowChinese(7,46,"ͬʱ����",WHITE,LGRAY,0);
		  LCD_ShowString(59,46,"ENT",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese12x12(86,46,"��",WHITE,LGRAY,12,0);
		  LCD_ShowString(99,46,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(127,46,"�˳�",WHITE,LGRAY,0);
		  LCD_ShowChinese(7,62,"������������Լ���",WHITE,LGRAY,0);
		  break;
		//�쳣�˳�
		case CapTest_EndERROR:
			LCD_ShowChinese(28,22,"���������쳣����",RED,LGRAY,0);
			LCD_ShowChinese(32,61,"����",WHITE,LGRAY,0);
		  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,61,"���˳�",WHITE,LGRAY,0);
		  break;
	  //���ڳ�ŵ�
	  case CapTest_ErrorDischarging:
		case CapTest_ErrorAlreadyCharging:
			LCD_ShowChinese(28,22,"���������޷�����",RED,LGRAY,0);
		  LCD_ShowChinese(10,41,"ϵͳ",RED,LGRAY,0);
		  if(CFSMState==CapTest_ErrorAlreadyCharging)LCD_ShowChinese12x12(36,41,"��",RED,LGRAY,12,0);
			else LCD_ShowChinese12x12(36,41,"��",RED,LGRAY,12,0);
		  LCD_ShowChinese(50,41,"����",RED,LGRAY,0);
 		  LCD_ShowChar(74,41,',',RED,LGRAY,12,0);  
			LCD_ShowChinese(83,41,"���Ƴ��߲�",RED,LGRAY,0);
			LCD_ShowChinese(32,61,"����",WHITE,LGRAY,0);
		  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,61,"���˳�",WHITE,LGRAY,0);
		  break;
		case CapTest_ErrorChipHang:
			LCD_ShowChinese(28,22,"���������޷�����",RED,LGRAY,0);
		  LCD_ShowChinese(21,41,"��ŵ����оƬ�쳣",RED,LGRAY,0);
			LCD_ShowChinese(32,61,"����",WHITE,LGRAY,0);
		  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,61,"���˳�",WHITE,LGRAY,0);
		  break;
		case CapTest_ErrorBattToHigh:
			LCD_ShowChinese(28,22,"���������޷�����",RED,LGRAY,0);
		  LCD_ShowChinese(4,41,"�뽫��طŵ���",RED,LGRAY,0);
		  LCD_ShowString(94,41,"12.3V",RED,LGRAY,12,0);
		  LCD_ShowChinese(133,41,"����",RED,LGRAY,0);
			LCD_ShowChinese(32,61,"����",WHITE,LGRAY,0);
		  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,61,"���˳�",WHITE,LGRAY,0);
		  break;
		}
	//��Ⱦ���
	IsUpDateGUI=true;
	}

//����ϵͳ״̬������
void CTestFSMHandler(void)
	{
	extern bool OCState;
	//״̬��	
	switch(CFSMState)
		{
		//��״̬
		case CapTest_Initial:
      //˲ʱ��������			
			if(CfgData.InstantCTest==InstantCTest_EnteredOK)
				{
				if(BATT==Batt_discharging)CFSMState=CapTest_ErrorDischarging;
				else if(BATT==Batt_ChgError)CFSMState=CapTest_ErrorChipHang;
  			else //ֱ�ӽ��뵽����		
					 {	
					 CFSMState=CapTest_WaitTypeCInsert; //�ȴ�Type-C����
					 CfgData.InstantCTest=InstantCTest_NotTriggered; //���������Ѵ���
					 WriteConfiguration(&CfgUnion,false); //д�뵽not triggered
					 }
				break;
				}
	    if(BATT==Batt_discharging)CFSMState=CapTest_ErrorDischarging;
	    else if(BATT==Batt_ChgError)CFSMState=CapTest_ErrorChipHang;
		  else if(BATT!=Batt_StandBy||VBUS.IsTypeCConnected)CFSMState=CapTest_ErrorAlreadyCharging;
		  else if(ADCO.Vbatt>12.3)CFSMState=CapTest_ErrorBattToHigh;
		  else CFSMState=CapTest_WaitTypeCInsert; //�ȴ�Type-C����
			IsUpDateGUI=false; //����ָ���ػ�GUI
		  break;
		//�ȴ�Type-C����
		case CapTest_WaitTypeCInsert:
			if(!VBUS.IsTypeCConnected)break; //TypeCδ����
		  switch(BATT)
				{
				case Batt_discharging:CFSMState=CapTest_ErrorDischarging;break; //����ŵ���ʾ��������ʧ��
				case Batt_PreChage:
				case Batt_CCCharge:
				case Batt_CVCharge:
				case Batt_ChgDone: //��ʼ����
					 CurrentTestResult.Data.StartVbatt=ADCO.Vbatt;
					 VBattSumbuf=0;
					 IBattsumbuf=0;
					 AverageCounter=8; //��λ���
				   CFSMState=CapTest_Running;
					 break;
				case Batt_ChgError:CFSMState=CapTest_ErrorChipHang;break; //оƬ�쳣
				default:break;
				}
			if(CFSMState==CapTest_Running)IsUpDateGUI=false; //����ָ���ػ�GUI
		  break;
		//�����¼�
		case CapTest_OverCharge:		
       //������ת			
			 if(!VBUS.IsTypeCConnected)CFSMState=CapTest_EndERROR;
			 else if(BATT==Batt_discharging)CFSMState=CapTest_ErrorDischarging;
			 else if(BATT==Batt_ChgError)CFSMState=CapTest_ErrorChipHang;
			 else if(!OCState)CFSMState=CapTest_Running; //�����¼�������������
		 //�ػ�GUI���
			 if(CFSMState!=CapTest_OverCharge)IsUpDateGUI=false; //����ָ���ػ�GUI
		   break;
		//���ݵȴ���
		case CapTest_ConfirmFull:
			 //������ת
			 if(!VBUS.IsTypeCConnected)CFSMState=CapTest_EndERROR;
			 else if(BATT==Batt_discharging)CFSMState=CapTest_ErrorDischarging;
			 else if(BATT==Batt_ChgError)CFSMState=CapTest_ErrorChipHang;
		   else if(OCState)CFSMState=CapTest_OverCharge; //�����¼�bit���𣬱�ǹ��䷢��
		   else if(BATT!=Batt_ChgDone)CFSMState=CapTest_Running; //оƬ�ص��ǳ���״̬
	     //�ػ�GUI���
			 if(CFSMState!=CapTest_ConfirmFull)IsUpDateGUI=false; //����ָ���ػ�GUI
		   //�ɹ����
			 if(ConfirmTimeCounter>0)break;
		   IsUpDateGUI=false; //����ָ���ػ�GUI
			 CFSMState=CapTest_Finish;
			 CurrentTestResult.Data.IsDataValid=true;
			 CurrentTestResult.Data.MaxChargeRatio=(CurrentTestResult.Data.MaxChargeCurrent*1000)/CurrentTestResult.Data.TotalmAH; //���C������������/������
			 memcpy(CTestData.ROMImage.Data.ByteBuf,CurrentTestResult.ByteBuf,sizeof(ChargeTestUnionDef)); //���µ�ǰ��������
			 WriteCapData(&CurrentTestResult,false);
		   break;
		//����������
		case CapTest_Running:	
			if(!VBUS.IsTypeCConnected)CFSMState=CapTest_EndERROR;
		  else if(BATT==Batt_discharging)CFSMState=CapTest_ErrorDischarging;
	    else if(BATT==Batt_ChgError)CFSMState=CapTest_ErrorChipHang;
		  else if(OCState)CFSMState=CapTest_OverCharge; //�����¼�bit���𣬱�ǹ��䷢��
			else if(BATT==Batt_ChgDone) //�ɹ���ɲ��ݣ���������
				{			
        ConfirmTimeCounter=960;
				CFSMState=CapTest_ConfirmFull; //�ȴ�����
				}
			if(CFSMState!=CapTest_Running)IsUpDateGUI=false; //����ָ���ػ�GUI
			break;
	  //�����������Ӧ
		default: break;
		}
	}
	
	
const MenuConfigDef CapTestMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
  &CTestGUIHandler,
	&CTestKeyHandler,	
	//�������ò˵�����Ҫ�ñ������
	"һ��������\0",
	NULL,
	NULL, 
	NULL,
	//������˳����캯��
	&ResetCapTestSystem,
	&ResetCapTestSystem
	};
