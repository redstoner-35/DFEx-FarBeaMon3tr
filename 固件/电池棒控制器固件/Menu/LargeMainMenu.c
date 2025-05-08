#include "IP2366_REG.h"
#include "GUI.h"
#include "Config.h"
#include "lcd.h"
#include "ADC.h"
#include "Key.h"
#include "LogSystem.h"
#include <math.h>

//�ⲿ����
extern bool IsTelemOK;
extern float VTypec,ITypeC,VBat,IBat;
extern ChipStatDef CState;
extern bool Is2368Telem;
extern RecvPDODef PDO;
extern bool IsUpdateCDUI;
extern BatteryStateDef BATT;
extern IP2366VBUSStateDef VBUS;
extern bool IsResultUpdated;

//�ڲ�����
static bool IsShowTypeCState=false;

//��ʾAH��
static void LargeMenu_DisplayAh(float Ah,bool IsDis)
	{
	float Power;
	u16 Color=IsDis?CYAN:WHITE;
	LCD_Fill(99,18,150,31,BLACK); //�������ɵ�ͼ��
	if(Ah<10) //С��10Ahʹ��mAH��ʾ
		{
		LCD_ShowIntNum(99,18,iroundf(Ah*1000),4,Color,BLACK,12);
		LCD_ShowString(132,18,"mAh",Color,BLACK,12,0);
		}
	else if(Ah<100000)//ʹ�ø�����ʾ
		{
		LCD_ShowString(132,18," Ah",Color,BLACK,12,0);
		if(Ah<100)LCD_ShowFloatNum1(99,18,Ah,2,Color,BLACK,12);  //99.99��ʾ
		else if(Ah<100)LCD_ShowFloatNum1(99,18,Ah,1,Color,BLACK,12);  //999.9��ʾ
		else LCD_ShowIntNum(99,18,iroundf(Ah),5,Color,BLACK,12); //9999��ʾ
		}
	else //ʹ��KAH��ʾ
		{
		Power=Ah/1000;
    if(Power<10)LCD_ShowFloatNum1(99,18,Power,3,Color,BLACK,12);  //9.999��ʾ
		else if(Power<100)LCD_ShowFloatNum1(99,18,Power,2,Color,BLACK,12);  //99.99��ʾ
		else if(Power<1000)LCD_ShowFloatNum1(99,18,Power,1,Color,BLACK,12);  //999.9��ʾ
		else LCD_ShowIntNum(99,18,iroundf(Power),5,Color,BLACK,12); //9999��ʾ
		LCD_ShowString(132,18,"KAh",Color,BLACK,12,0);
		}	
	}
//��ʾ��ŵ�����
static void LargeMenu_DisplayWh(float Wh,bool IsDis)
	{
	float Power;
	u16 Color=IsDis?CYAN:WHITE;	
	LCD_Fill(99,34,150,46,BLACK); //�������ɵ�ͼ��
	if(Wh<10000)LCD_ShowString(132,34," Wh",Color,BLACK,12,0);
	if(Wh<10)LCD_ShowFloatNum1(99,34,Wh,3,Color,BLACK,12); //9.999��ʾ
	else if(Wh<100)LCD_ShowFloatNum1(99,34,Wh,2,Color,BLACK,12); //99.99��ʾ	
	else if(Wh<1000)LCD_ShowFloatNum1(99,34,Wh,1,Color,BLACK,12); //999.9��ʾ
	else if(Wh<10000)LCD_ShowIntNum(99,34,iroundf(Wh),5,Color,BLACK,12); //9999��ʾ
	else	
	  {
		Power=Wh;
	  if(Power<10)LCD_ShowFloatNum1(99,34,Power,2,Color,BLACK,12); //9.99��ʾ
		else if(Power<100)LCD_ShowFloatNum1(99,34,Power,1,Color,BLACK,12); //99.9��ʾ	
		else if(Power<1000)LCD_ShowIntNum(99,34,iroundf(Power),3,Color,BLACK,12); //999��ʾ
		LCD_ShowString(132,34,"KWh",Color,BLACK,12,0);	
		}
	}	
//��ʾ��ŵ�ʱ��
static void LargeMenu_ShowTime(long TimeIN,bool IsDis)
	{
	long time;
	u16 ICONColor=IsDis?LIGHTGREEN:WHITE;
	u16 TextColor=IsDis?ORANGE:YELLOW;
	if(TimeIN>86400*30) //ʱ������һ����
		{
		time=TimeIN/(86400*30); //���������
		LCD_ShowIntNum(99,49,time,2,TextColor,BLACK,12);
		LCD_ShowChinese12x12(115,49,"��\0",ICONColor,BLACK,12,0);
		time=(TimeIN%(86400*30))/86400; //���������
		LCD_ShowIntNum(128,49,time,2,TextColor,BLACK,12);
		LCD_ShowChinese12x12(144,49,"��\0",ICONColor,BLACK,12,0);			
		}
	else if(TimeIN>86400) //ʱ������һ��
		{
		time=TimeIN/86400; //���������
		LCD_ShowIntNum(99,49,time,2,TextColor,BLACK,12);
		LCD_ShowChinese12x12(115,49,"��\0",ICONColor,BLACK,12,0);
		time=(TimeIN%86400)/3600; //�����Сʱ��
		LCD_ShowIntNum(128,49,time,2,TextColor,BLACK,12);
		LCD_ShowChinese12x12(144,49,"ʱ\0",ICONColor,BLACK,12,0);		
		}
	else if(TimeIN>3600)
		{
		time=TimeIN/3600; //�����Сʱ��
		LCD_ShowIntNum(99,49,time,2,TextColor,BLACK,12);
		LCD_ShowChinese12x12(115,49,"ʱ\0",ICONColor,BLACK,12,0);
		time=(TimeIN%3600)/60; //�����������
		LCD_ShowIntNum(128,49,time,2,TextColor,BLACK,12);
		LCD_ShowChinese12x12(144,49,"��\0",ICONColor,BLACK,12,0);			
		}
  else
		{
		time=TimeIN/60; //�����������
		LCD_ShowIntNum(99,49,time,2,TextColor,BLACK,12);
		LCD_ShowChinese12x12(115,49,"��\0",ICONColor,BLACK,12,0);
		time=TimeIN%60;
		LCD_ShowIntNum(128,49,time,2,TextColor,BLACK,12);
		LCD_ShowChinese12x12(144,49,"��\0",ICONColor,BLACK,12,0);		 
		}	
	}	
	
//��ʾ���״̬
static void RenderBattState(void)
	{
	float Power;
	u16 Color;
	int Temp;
	long time;
	bool IsShowBatt;
	extern bool IsSystemOverheating;
	extern bool OCState;
	extern bool IsDispChargingINFO;
	//���UI�Ƿ������Ⱦ
	Is2368Telem=true;
	time=(BATT==Batt_discharging)?LogData.DischargeTime:LogData.ChargeTime; //��ȡ��ŵ�ʱ��
	if(!IsTelemOK)return;
	if(BATT!=Batt_StandBy&&fabsf(ADCO.Ibatt)>MinimumCurrentFactor)	//ϵͳ���ڿ���״̬������־�����ۼӣ����ս����Ⱦ����
		{
		if(time>86400&&!IsResultUpdated)return; //ʱ�䳬��1��Сʱ����ʹ�ýϿ��ˢ��Ƶ��
		else if(time<86400&&!IsUpdateCDUI)return;	 //ʱ��һ��Сʱ��ʹ�ý�����ˢ��Ƶ������������ͬ��	
		}
	else if(!IsResultUpdated)return;  //��̬�׶�ʹ�ÿ��ٸ���ϵͳ
	//������Ⱦ
	LCD_DrawRectangle(0,0,159,79,WHITE);	
	//��ѹ
	LCD_ShowFloatNum1(3,3,VBat,2,LIGHTGREEN,BLACK,24);
	LCD_ShowChar(73,3,'V',LIGHTGREEN,BLACK,24,0);
	//����
	LCD_Fill(3,28,84,76,BLACK);
	Power=fabsf(IBat)>MinimumCurrentFactor?IBat:0;
	if(Power<0) //����Ϊ�������ϸ���
			{
			LCD_ShowChar(3,28,'-',YELLOW,BLACK,24,0);	
			if(Power>-10)LCD_ShowFloatNum1(16,28,Power,2,YELLOW,BLACK,24);
			else LCD_ShowFloatNum1(16,28,Power,1,YELLOW,BLACK,24);
			}
	else LCD_ShowFloatNum1(3,28,Power,2,YELLOW,BLACK,24);
	LCD_ShowChar(73,28,'A',YELLOW,BLACK,24,0);	
	//����
	Power=fabsf(VBat*Power);
	if(Power<10)LCD_ShowFloatNum1(3,53,Power,3,CYAN,BLACK,24);
	else if(Power<100)LCD_ShowFloatNum1(3,53,Power,2,CYAN,BLACK,24);
  else LCD_ShowFloatNum1(3,53,Power,1,CYAN,BLACK,24);
  LCD_ShowChar(73,53,'W',CYAN,BLACK,24,0);	
	//�¶�
	if(!ADCO.IsNTCOK)LCD_ShowString(99,3,"---.-",WHITE,BLACK,12,0);
	else
		{
		Temp=iroundf(ADCO.Systemp);
		if(Temp<0)Color=DARKBLUE;	
		if(Temp<10)Color=BLUE;
		else if(Temp<CfgData.OverHeatLockTemp-20)Color=GREEN;
		else if(Temp<CfgData.OverHeatLockTemp-8)Color=YELLOW;
		else Color=RED;	
		//0��
		if(ADCO.Systemp<0)
			{
			LCD_ShowChar(99,3,'-',WHITE,BLACK,12,0);
			if(ADCO.Systemp>-10)LCD_ShowFloatNum1(107,3,ADCO.Systemp*-1,2,Color,BLACK,12);
			else LCD_ShowFloatNum1(107,3,ADCO.Systemp*-1,1,Color,BLACK,12);
			}
		//�����¶�
		else if(ADCO.Systemp<99)LCD_ShowFloatNum1(99,3,ADCO.Systemp*-1,2,Color,BLACK,12);
		else LCD_ShowFloatNum1(99,3,ADCO.Systemp*-1,1,Color,BLACK,12);
		LCD_ShowChinese12x12(144,3,"��\0",Color,BLACK,12,0);
		}
	//������ʾ
	if(BATT==Batt_discharging)LargeMenu_DisplayAh(LogData.TotalDischargeAh,true);
	else LargeMenu_DisplayAh(LogData.TotalChargeAh,false);
	//������ʾ
  if(BATT==Batt_discharging)LargeMenu_DisplayWh(LogData.TotalDischargeWh,true);
	else LargeMenu_DisplayWh(LogData.TotalChargeWh,false);
	//���ʱ����ʾ
	LargeMenu_ShowTime(time,BATT==Batt_discharging?true:false);
  //��ʾ���״̬
	if(CState.VSysState!=VSys_State_Normal||CState.VBusState==VBUS_OverVolt)IsShowBatt=false;
	else if(IsSystemOverheating)IsShowBatt=false;
  else if(OCState)IsShowBatt=true;		
	else if(BATT!=Batt_StandBy)IsShowBatt=true;
	else IsShowBatt=false;	
	//��ʾʵ�ʵ�����	
	LCD_ShowHybridString(99,64,IsShowBatt?"���:":"ϵͳ:",WHITE,BLACK,0);
	if(IsSystemOverheating)
		LCD_ShowChinese(132,64,"����\0",RED,BLACK,0);
	else if(CState.VSysState!=VSys_State_Normal||CState.VBusState==VBUS_OverVolt) //�����·���������ѹ
		LCD_ShowChinese(132,64,"����\0",RED,BLACK,0);
  else if(OCState)
		LCD_ShowChinese(132,64,IsDispChargingINFO?"����":"����",YELLOW,BLACK,0);
  else switch(BATT)			//����ö��״̬��ʾ
		{
		case Batt_StandBy:LCD_ShowChinese(132,64,"����\0",WHITE,BLACK,0);break;
		case Batt_PreChage:
			LCD_ShowChinese(132,64,IsDispChargingINFO?"���":"���\0",MAGENTA,BLACK,0);
			break;
		case Batt_CCCharge:
			LCD_ShowChinese(132,64,IsDispChargingINFO?"���":"����\0",YELLOW,BLACK,0);
			break;
		case Batt_CVCharge:
			LCD_ShowChinese(132,64,IsDispChargingINFO?"���":"��ѹ\0",GBLUE,BLACK,0);
			break;
		case Batt_ChgWait:
			LCD_ShowChinese(132,64,IsDispChargingINFO?"���":"��ͣ\0",YELLOW,BLACK,0);
			break;
		case Batt_ChgDone:LCD_ShowChinese(132,64,"����\0",LIGHTGREEN,BLACK,0);break;
		case Batt_ChgError:LCD_ShowChinese(132,64,IsDispChargingINFO?"���":"��ʱ\0",ORANGE,BLACK,0);break;
		case Batt_discharging:LCD_ShowChinese(132,64,"�ŵ�\0",CYAN,BLACK,0);break;
		}
	//������ʾ��ϣ��ȴ����ݸ��º���ˢ��
  IsTelemOK=false;	
  IsResultUpdated=false;		
	IsUpdateCDUI=false;
	}
	
//��ʾType-C״̬
void RenderTypeCState(void)
	{
	float Power;
	extern bool IsDispChargingINFO;
	extern bool IsEnableHPGauge;	
	//��������
	Is2368Telem=true;
	if(!VBUS.IsTypeCConnected)
		{
		//Type-C���Ƴ��������˳����˵�
		ClearScreen();
		IsShowTypeCState=false;
		return;
		}
	if(!IsTelemOK||!IsResultUpdated)return;
	//������Ⱦ
	LCD_DrawRectangle(0,0,159,79,WHITE);	
	//��ѹ
	LCD_ShowFloatNum1(3,3,VTypec,2,IsEnableHPGauge?ORANGE:LIGHTGREEN,BLACK,24);
	LCD_ShowChar(73,3,'V',IsEnableHPGauge?ORANGE:LIGHTGREEN,BLACK,24,0);
	//����
	LCD_Fill(3,28,84,76,BLACK);
	Power=fabsf(ITypeC)>MinimumCurrentFactor?ITypeC:0;
	if(Power<0) //����Ϊ�������ϸ���
			{
			LCD_ShowChar(3,28,'-',YELLOW,BLACK,24,0);	
			if(Power>-10)LCD_ShowFloatNum1(16,28,Power,2,YELLOW,BLACK,24);
			else LCD_ShowFloatNum1(16,28,Power,1,YELLOW,BLACK,24);
			}
	else LCD_ShowFloatNum1(3,28,Power,2,YELLOW,BLACK,24);
	LCD_ShowChar(73,28,'A',YELLOW,BLACK,24,0);	
	//����
	Power=fabsf(VTypec*Power);
	if(Power<10)LCD_ShowFloatNum1(3,53,Power,3,CYAN,BLACK,24);
	else if(Power<100)LCD_ShowFloatNum1(3,53,Power,2,CYAN,BLACK,24);
  else LCD_ShowFloatNum1(3,53,Power,1,CYAN,BLACK,24);
  LCD_ShowChar(73,53,'W',CYAN,BLACK,24,0);	
	//PDO�����VBUS״̬��ʾ
	LCD_Fill(102,9,152,36,BLACK);
	LCD_DrawRectangle(102,9,152,36,WHITE);
	LCD_Fill(104,4,150,15,BLACK);
	if(!IsDispChargingINFO)	
		{		
		LCD_ShowHybridString(104,4,"PDO�㲥",WHITE,BLACK,0);
		switch(PDO)
			{
			case RecvPDO_None:LCD_ShowString(116,19,"N/A",WHITE,BLACK,12,0);break;
			case RecvPDO_5V:LCD_ShowString(116,19," 5V",CYAN,BLACK,12,0);break;
			case RecvPDO_9V:LCD_ShowString(116,19," 9V",BLUE,BLACK,12,0);break;
			case RecvPDO_12V:LCD_ShowString(116,19,"12V",GREEN,BLACK,12,0);break;
			case RecvPDO_15V:LCD_ShowString(116,19,"15V",CYAN,BLACK,12,0);break;
			case RecvPDO_20V:LCD_ShowString(116,19,"20V",CYAN,BLACK,12,0);break;
			}
		}
	else //��ʾ����״̬
		{
		LCD_ShowHybridString(105,4,"Type-C",WHITE,BLACK,0);
		if(CState.VBusState==VBUS_NoPower)LCD_ShowHybridString(115,19,"�Ͽ�",LGRAY,BLACK,0);	
		else if(CState.VBusState==VBUS_OverVolt)LCD_ShowHybridString(115,19,"��ѹ",RED,BLACK,0);
		else switch(CState.VSysState)
			{
			case VSys_State_Normal:LCD_ShowHybridString(115,19,"����",GREEN,BLACK,0);break;
			case VSys_State_OCP:LCD_ShowHybridString(115,19,"����",YELLOW,BLACK,0);break;
			case VSys_State_Short:LCD_ShowHybridString(115,19,"��·",RED,BLACK,0);break;
			}
		}		
	//Э����ʾ
	LCD_Fill(102,46,152,73,BLACK);
	LCD_DrawRectangle(102,46,152,73,WHITE);
	LCD_Fill(112,41,142,52,BLACK);
	LCD_ShowHybridString(115,41,"Э��",WHITE,BLACK,0);
	//PD���
	if(VBUS.QuickChargeState==QuickCharge_PD&&VBUS.PDState!=PD_5VMode)switch(VBUS.PDState)
		{
		case PD_5VMode:LCD_ShowString(115,56,"N/A",WHITE,BLACK,12,0);break;
		case PD_7VMode:LCD_ShowString(108,56,"PD 7V",MAGENTA,BLACK,12,0);break;
		case PD_9VMode:LCD_ShowString(108,56,"PD 9V",MAGENTA,BLACK,12,0);break;
		case PD_12VMode:LCD_ShowString(108,56,"PD12V",MAGENTA,BLACK,12,0);break;
		case PD_15VMode:LCD_ShowString(108,56,"PD15V",MAGENTA,BLACK,12,0);break;
		case PD_20VMode:LCD_ShowString(108,56,"PD20V",YELLOW,BLACK,12,0);break;
		case PD_28VMode:LCD_ShowString(108,56,"PDEPR",CYAN,BLACK,12,0);break;
		}
	//QC�ʹ�������
	else if(VBUS.QuickChargeState==QuickCharge_HV)LCD_ShowChinese(115,56,"��ѹ\0",YELLOW,BLACK,0);
	else if(VBUS.QuickChargeState==QuickCharge_HC)LCD_ShowChinese(115,56,"����\0",YELLOW,BLACK,0);
	//�޿��
	else LCD_ShowString(115,56,"N/A",WHITE,BLACK,12,0);
	//������ʾ��ϣ��ȴ����ݸ��º���ˢ��
  IsTelemOK=false;		
	IsResultUpdated=false;
	}	

//��������
void EnterAdvModeProc(void);	
	
void LargeMainMenuKeyProcess(void)
	{
	//ͬʱ��ס���¼���������
	if(KeyState.KeyEvent==KeyEvent_BothEnt)
		{
		if(!CfgData.EnableAdvAccess)SwitchingMenu(&EasySetMainMenu); //�������ģʽ
		else EnterAdvModeProc(); //����߼�ģʽ
		}
	if(KeyState.KeyEvent==KeyEvent_Up&&VBUS.IsTypeCConnected)
		{
		if(!IsShowTypeCState)ClearScreen();
		IsShowTypeCState=true;
		}
	if(KeyState.KeyEvent==KeyEvent_Down)
		{
		if(IsShowTypeCState)ClearScreen();
		IsShowTypeCState=false;
		}
	KeyState.KeyEvent=KeyEvent_None;
	}
	
//��Ⱦ����
void LargeMenuRenderProc(void)
	{
	if(!CfgData.EnableLargeMenu)
		{	
		SwitchingMenu(&MainMenu);
		return;
		}
	if(IsShowTypeCState)RenderTypeCState();
	else RenderBattState(); 
	}
	
const MenuConfigDef LargeMainMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&LargeMenuRenderProc, //��Ⱦ����
	&LargeMainMenuKeyProcess, //��������
	//�������ò˵�����Ҫ�ñ������
	NULL,
	NULL,
	NULL, 
	NULL,
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};
	
