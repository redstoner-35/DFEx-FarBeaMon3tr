#include "IP2366_REG.h"
#include "GUI.h"
#include "Config.h"
#include "lcd.h"
#include "ADC.h"
#include "pic.h"
#include "Key.h"
#include <math.h>
#include "INA226.h"

//ƽ������
#define SADCAvgCount 5
//�ڲ�����
bool IsTelemOK=false;
IP2366VBUSStateDef VBUS;
BatteryStateDef BATT;
QuickChargeStateDef QCState=QuickCharge_None;
bool Is2368Telem=false;
extern short SleepTimer;
static bool ConnectState;
static float VBUSAvgBuf[4]={0};
static char VBusAvgCount=0;
float VTypec=0,ITypeC=0,VBat=0,IBat=0;
bool IsResultUpdated=true;
RecvPDODef PDO=RecvPDO_None;
static char ChargerAltDisplayTIM=0;
bool IsDispChargingINFO=false; //ʵ���������ָʾ�ͳ������������ʾ
ChipStatDef CState;
static bool IsShowEff=false;

//��IP2366���ӻ�ȡ����
void IP2366_Telem(void)
	{
	char i;
	float Result;
	extern bool IsEnableHPGauge;	
	INADoutSreDef VBUSData;
	//�ж��Ƿ����ô���
	if(!Is2368Telem)return;
	IsTelemOK=IP2366_GetChargerState(&BATT);
	IsTelemOK|=IP2366_GetVBUSState(&VBUS);
	IsTelemOK|=IP2366_ReadChipState(&CState);
	IsTelemOK|=IP2366_GetRecvPDO(&PDO);
	//��ʼ����INA226�Ĳ���	
	if(IsEnableHPGauge)
		{
		//���Զ�ȡINA226���и߾��Ȳ���
		if(!INA226_GetBusInformation(&VBUSData))
			{
			//�߾��Ȳ���ʧЧ���ص���ͳģʽ
			IsEnableHPGauge=false;
			}
		//�߾��Ȳ���������ɣ�ֱ��ʹ��INA226������������	
		else
			{		
			//�����ݽ����滻
			VBUS.VBUSVolt=VBUSData.BusVolt;
			VBUS.VBUSCurrent=fabsf(VBUSData.BusCurrent); 
			//��ش��ڷŵ�״̬������*-1��ʾ���ڷŵ�
			if(BATT==Batt_discharging)VBUS.VBUSCurrent*=-1;
			}
		}
	//�����ɹ�������ʼ�ۼ�
	if(IsTelemOK)
		{
		//������ʾ���״̬	
		if(ChargerAltDisplayTIM>0)ChargerAltDisplayTIM--;
		else
			{
			IsDispChargingINFO=IsDispChargingINFO?false:true;
			ChargerAltDisplayTIM=16;
			}
		//��ѹ����
		if(VBusAvgCount<SADCAvgCount)
			{
			VBusAvgCount++;
			VBUSAvgBuf[2]+=ADCO.Vbatt;
			VBUSAvgBuf[3]+=ADCO.Ibatt;
			VBUSAvgBuf[0]+=VBUS.VBUSVolt;
			VBUSAvgBuf[1]+=VBUS.VBUSCurrent;
			}
		else
			{
			IsResultUpdated=true;
			VBusAvgCount=0;
			VBat=VBUSAvgBuf[2]/(float)SADCAvgCount;
			IBat=VBUSAvgBuf[3]/(float)SADCAvgCount;
      //��ȡTypeC���
			Result=VBUSAvgBuf[0]/(float)SADCAvgCount;
			Result*=(float)CfgData.TypeCVoltageCal;
			Result/=(float)1000;
			if(Result>4&&Result<30)VTypec=Result;
			//��ȡTypeC�������
			Result=VBUSAvgBuf[1]/(float)SADCAvgCount;
			Result*=(float)CfgData.TypeCAmpereCal;
			Result/=(float)1000;
			if(fabsf(Result)<7.5)ITypeC=Result;
			for(i=0;i<4;i++)VBUSAvgBuf[i]=0;
			}		
		if(VBUS.IsTypeCConnected)SleepTimer=480; //��ֹ˯��
		}
	Is2368Telem=false;
	}

//����Ч�ʲ�����ٷֱ�
static float CalcEfficiency(void)	
	{
	float VBUSPower,VBatPower,eff;
	//оƬ�����쳣����ʾ������
	if(!IsTelemOK)return -1;
	//���������������
	VBUSPower=fabsf(VTypec*ITypeC);
	VBatPower=fabsf(VBat*IBat);	
	//����Ч��	
	switch(BATT)
		{
		case Batt_discharging:eff=VBUSPower/VBatPower;break; //����ŵ�ʱEff=VBUS/Vbat
		case Batt_PreChage:
		case Batt_CCCharge:
		case Batt_CVCharge:eff=VBatPower/VBUSPower;break; //���ʱEff=Vbat/Vbus
		default:return -1; //�������Ч�ʲ���ʾ
		}
	//�޷���ֵ�����ؽ��
	eff*=(float)100;
  if(eff>98)eff=98;
	if(eff<5)eff=5;
	return eff;
	}

//��Ⱦ����
void MainMenuRenderProcess(void)
	{
	float Power;
	u16 Color;
	int Temp;
	extern bool IsSystemOverheating;
	extern bool OCState;
	extern bool IsEnableHPGauge;
	//�ж��Ƿ���������ģʽ	
	if(CfgData.EnableLargeMenu)
		{
		SwitchingMenu(&LargeMainMenu);
		return;
		}
	//��������
	Is2368Telem=true;
	if(!IsTelemOK||!IsResultUpdated)return;
	//��ʾTypeC��ѹ����
	LCD_ShowPicture(19,0,39,15,USBLogo);
	LCD_DrawLine(4,7,15,7,WHITE);
	LCD_DrawLine(61,7,75,7,WHITE);
	//ÿ�θ���Type-C״̬��ʱ��ѱ����ٸɾ�
	if(ConnectState!=VBUS.IsTypeCConnected)
		{
		LCD_Fill(5,14,74,75,BLACK);
		LCD_Fill(86,18,146,57,BLACK);
		ConnectState=VBUS.IsTypeCConnected;
		}
  if(VBUS.IsTypeCConnected)		
		{
		LCD_Fill(9,18,74,75,BLACK);
		//��ѹ
		Power=(VTypec>4&&VTypec<30)?VTypec:0;
		LCD_ShowFloatNum1(9,18,Power,2,IsEnableHPGauge?ORANGE:LIGHTGREEN,BLACK,12);
		LCD_ShowChar(62,18,'V',IsEnableHPGauge?ORANGE:LIGHTGREEN,BLACK,12,0);
		//����
		Power=fabsf(ITypeC);
		if(Power>5.5||Power<0.05)Power=0;
		else Power=ITypeC; //����ֵ����
		if(Power<0) //����Ϊ�������ϸ���
			{
			Power*=-1;
			LCD_ShowChar(9,32,'-',YELLOW,BLACK,12,0);	
			LCD_ShowFloatNum1(18,32,Power,Power<10?2:1,YELLOW,BLACK,12);
			}
		else LCD_ShowFloatNum1(9,32,Power,2,YELLOW,BLACK,12);
		LCD_ShowChar(62,32,'A',YELLOW,BLACK,12,0);	
		//����
	  Power=fabsf(((VTypec>4&&VTypec<30)?VTypec:0)*Power);
		if(Power<10)LCD_ShowFloatNum1(9,46,Power,3,CYAN,BLACK,12);
    else if(Power<100)LCD_ShowFloatNum1(9,46,Power,2,CYAN,BLACK,12);
		else LCD_ShowFloatNum1(9,46,Power,1,CYAN,BLACK,12);
		LCD_ShowChar(62,46,'W',CYAN,BLACK,12,0);	
		//Ч��ָʾ��������ʾϵͳЧ��
		if(IsShowEff)	
			{
			LCD_Fill(5,60,74,75,BLACK);
			LCD_ShowChinese(9,61,"Ч��\0",WHITE,BLACK,0);
			Power=CalcEfficiency();
			if(Power!=-1)
				{
				//����Ч�������������ʾ
				LCD_ShowFloatNum1(34,61,Power,1,WHITE,BLACK,12);
				LCD_ShowChar(65,61,'%',WHITE,BLACK,12,0);
				}
			//Ч����ʾ�쳣����ʾN/A
			else LCD_ShowString(48,61,"N/A",WHITE,BLACK,12,0);
			}
		//Ч��ָʾ�رգ���ʾ���ָʾ
		else
			{
			if(QCState!=VBUS.QuickChargeState)
				{
				LCD_Fill(5,60,74,75,BLACK);
				QCState=VBUS.QuickChargeState;
				}
			if(VBUS.QuickChargeState!=QuickCharge_None)
				{
				LCD_ShowPicture(61,60,9,14,QuickCHarge);
				//PD���
				if(VBUS.QuickChargeState==QuickCharge_PD&&VBUS.PDState!=PD_5VMode)switch(VBUS.PDState)
					{
					case PD_5VMode:break;
					case PD_7VMode:LCD_ShowString(9,61,"PD 7V",MAGENTA,BLACK,12,0);break;
					case PD_9VMode:LCD_ShowString(9,61,"PD 9V",MAGENTA,BLACK,12,0);break;
					case PD_12VMode:LCD_ShowString(9,61,"PD12V",MAGENTA,BLACK,12,0);break;
					case PD_15VMode:LCD_ShowString(9,61,"PD15V",MAGENTA,BLACK,12,0);break;
					case PD_20VMode:LCD_ShowString(9,61,"PD20V",YELLOW,BLACK,12,0);break;
					case PD_28VMode:LCD_ShowString(9,61,"PDEPR",CYAN,BLACK,12,0);break;
					}
				//QC�ʹ�������
				else if(VBUS.QuickChargeState==QuickCharge_HV)LCD_ShowChinese(9,61,"��ѹ\0",YELLOW,BLACK,0);
				else if(VBUS.QuickChargeState==QuickCharge_HC)LCD_ShowChinese(9,61,"����\0",YELLOW,BLACK,0);
				}
			}			
		}
	else //��ʾδ����
		{
		VTypec=0;
		ITypeC=0;
		LCD_ShowString(19,28,"TYPE-C",CYAN,BLACK,12,0);
		LCD_ShowChinese(20,43,"δ����\0",YELLOW,BLACK,0);
		}
	//�ѿ���
	LCD_DrawLine(4,7,4,76,WHITE);
	LCD_DrawLine(75,7,75,76,WHITE);
	LCD_DrawLine(4,76,75,76,WHITE);
	//��ʾ��ص�ѹ
	LCD_ShowPicture(105,1,24,15,BattICON);
	LCD_DrawLine(81,7,102,7,WHITE);
	LCD_DrawLine(131,7,152,7,WHITE);
	//��ѹ
	LCD_ShowFloatNum1(86,18,VBat,2,LIGHTGREEN,BLACK,12);
	LCD_ShowChar(139,18,'V',LIGHTGREEN,BLACK,12,0);
	//����
	LCD_Fill(86,32,135,57,BLACK);
	Power=fabsf(IBat)>MinimumCurrentFactor?IBat:0;
	if(Power<0) //����Ϊ�������ϸ���
			{
			LCD_ShowChar(86,32,'-',YELLOW,BLACK,12,0);	
		  if(Power>-10)LCD_ShowFloatNum1(95,32,Power,2,YELLOW,BLACK,12);
			else LCD_ShowFloatNum1(95,32,Power,1,YELLOW,BLACK,12);
			}
	else LCD_ShowFloatNum1(86,32,Power,2,YELLOW,BLACK,12);
	LCD_ShowChar(139,32,'A',YELLOW,BLACK,12,0);	
	//����
	Power=fabsf(VBat*Power);
	if(Power<10)LCD_ShowFloatNum1(86,46,Power,3,CYAN,BLACK,12);
	else if(Power<100)LCD_ShowFloatNum1(86,46,Power,2,CYAN,BLACK,12);
  else LCD_ShowFloatNum1(86,46,Power,1,CYAN,BLACK,12);
  LCD_ShowChar(139,46,'W',CYAN,BLACK,12,0);	
	//��ʾϵͳ״̬
	if(IsSystemOverheating)
		LCD_ShowChinese(86,61,"����\0",RED,BLACK,0);
	else if(CState.VSysState!=VSys_State_Normal||CState.VBusState==VBUS_OverVolt) //�����·���������ѹ
		LCD_ShowChinese(86,61,"����\0",RED,BLACK,0);
  else if(OCState)
		LCD_ShowChinese(86,61,IsDispChargingINFO?"����":"����",YELLOW,BLACK,0);
  else switch(BATT)			//����ö��״̬��ʾ
		{
		case Batt_StandBy:LCD_ShowChinese(86,61,"����\0",WHITE,BLACK,0);break;
		case Batt_PreChage:
			LCD_ShowChinese(86,61,IsDispChargingINFO?"���":"���\0",MAGENTA,BLACK,0);
			break;
		case Batt_CCCharge:
			LCD_ShowChinese(86,61,IsDispChargingINFO?"���":"����\0",YELLOW,BLACK,0);
			break;
		case Batt_CVCharge:
			LCD_ShowChinese(86,61,IsDispChargingINFO?"���":"��ѹ\0",GBLUE,BLACK,0);
			break;
		case Batt_ChgWait:
			LCD_ShowChinese(86,61,IsDispChargingINFO?"���":"��ͣ\0",YELLOW,BLACK,0);
			break;
		case Batt_ChgDone:LCD_ShowChinese(86,61,"����\0",LIGHTGREEN,BLACK,0);break;
		case Batt_ChgError:LCD_ShowChinese(86,61,IsDispChargingINFO?"���":"��ʱ\0",ORANGE,BLACK,0);break;
		case Batt_discharging:LCD_ShowChinese(86,61,"�ŵ�\0",WHITE,BLACK,0);break;
		}
	//�¶���ʾ
	if(!ADCO.IsNTCOK||ADCO.Systemp<-9)LCD_ShowString(117,61,"--",WHITE,BLACK,12,0);
	else
		{
		Temp=iroundf(ADCO.Systemp);
		if(Temp<0)Color=DARKBLUE;	
		else if(Temp<10)Color=BLUE;
		else if(Temp<CfgData.OverHeatLockTemp-20)Color=GREEN;
		else if(Temp<CfgData.OverHeatLockTemp-8)Color=YELLOW;
		else Color=RED;
	  //��ʾ�¶�
		if(Temp<0)
			{
			Temp*=-1;
			LCD_ShowChar(117,61,'-',Color,BLACK,12,0);
			LCD_ShowChar(127,61,0x30+Temp,Color,BLACK,12,0);
			}
		else LCD_ShowIntNum(117,61,Temp,2,Color,BLACK,12);
		LCD_ShowChinese12x12(135,61,"��\0",Color,BLACK,12,0);
		//����ʣ�µ���
		LCD_DrawLine(81,7,81,76,WHITE);
		LCD_DrawLine(152,7,152,76,WHITE);
		LCD_DrawLine(81,76,152,76,WHITE);
		}
  //������ʾ��ϣ��ȴ����ݸ��º���ˢ��
  IsTelemOK=false;		
	IsResultUpdated=false;
	}

//��������
void EnterAdvModeProc(void);	
	
void MainMenuKeyProcess(void)
	{
	//ͬʱ��ס���¼���������
	if(KeyState.KeyEvent==KeyEvent_BothEnt)
		{
		if(!CfgData.EnableAdvAccess)SwitchingMenu(&EasySetMainMenu); //�������ģʽ
		else EnterAdvModeProc(); //����߼�ģʽ
		}
	if(KeyState.KeyEvent==KeyEvent_Up)IsShowEff=true;
	if(KeyState.KeyEvent==KeyEvent_Down)
		{
		IsShowEff=false;
		QCState=QuickCharge_PlaceHolder; //ͨ���ѽ�������ΪPlaceholder���и�λ
		}
	KeyState.KeyEvent=KeyEvent_None;
	}

const MenuConfigDef MainMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&MainMenuRenderProcess, //��Ⱦ����
	&MainMenuKeyProcess, //��������
	//�������ò˵�����Ҫ�ñ������
	NULL,
	NULL,
	NULL, 
	NULL,
	//������˳����캯��û������Ҫ��
	NULL,
	NULL
	};
