#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"
#include "ADC.h"
#include "Key.h"
#include "delay.h"
#include <math.h>

typedef enum
	{
	Error_CommFault,
  Error_VbatTooLow,
  Error_SysNotInDischarge,
	Error_SysFaultAsserted
	}AdapEmuErrorDef;

//�ⲿ����
extern bool Is2368Telem;
extern IP2366VBUSStateDef VBUS;
extern float VTypec,ITypeC,VBat,IBat;
extern bool IsResultUpdated;
extern bool IsSystemOverheating;
extern ChipStatDef CState;
	
//�ڲ�����
bool IsEnableAdapterEmu=false; //�Ƿ���������ģ��
bool IsStopEmulationDueToLV=false; //�Ƿ���ֹͣģ��
bool IsStopDueToFault=false; //�Ƿ����ϵͳ����
static bool IsAdapterEnterError=false; //���������
static AdapEmuErrorDef EmuErrorName;
	
//�ڲ�������������Ƿ����
static bool CheckIfBattTooLow(void)
	{
	float VMin;
	switch(CfgData.Vlow)	
		{
		case VLow_2V5:VMin=2.5;break;
		case VLow_2V6:VMin=2.6;break;
		case VLow_2V7:VMin=2.7;break;
		case VLow_2V8:VMin=2.8;break;
		case VLow_2V9:VMin=2.9;break;
		case VLow_3V0:VMin=3.0;break;
		case VLow_3V1:VMin=3.1;break;
		case VLow_3V2:VMin=3.2;break;
		}
	VMin=(VMin*BattCellCount)+0.2; //����ó���͵�ѹ
	if(ADCO.Vbatt<VMin)return true;
  //��������
	return false;
	}	
	
//����������ģ��
void EnterAdapterEmulation(void)
	{
	BatteryStateDef BattState;
	//���ͨ�Ŵ���
	EmuErrorName=Error_CommFault;
	//��ȡоƬ��ǰ״̬
	if(CState.VSysState!=VSys_State_Normal||CState.VBusState==VBUS_OverVolt)
		{
		EmuErrorName=Error_SysFaultAsserted;
		IsAdapterEnterError=true;	
		}
	else if(!IP2366_GetChargerState(&BattState))IsAdapterEnterError=true;
	else if(BattState!=Batt_discharging)
		{
		EmuErrorName=Error_SysNotInDischarge;
		IsAdapterEnterError=true;
		}
	//����ص�ѹ�Ƿ����
	else if(CheckIfBattTooLow())
		{
	  IsAdapterEnterError=true;
	  EmuErrorName=Error_VbatTooLow;
		}
	//����Type-C
	else if(!IP2366_SetTypeCRole(TypeC_UFP))IsAdapterEnterError=true;
  else if(!IP2366_EnableDCDC(false,true))IsAdapterEnterError=true;   //����UFPģʽ��ǿ�����÷ŵ�
	else IsAdapterEnterError=false;
	//�ж��Ƿ����ɹ�
	if(!IsAdapterEnterError)IsEnableAdapterEmu=true; //�ɹ�����
	IsResultUpdated=true;
	IsStopDueToFault=false;
	IsStopEmulationDueToLV=false; //������λ
	}
	
void ExitAdapterEmulation(void)
	{
	int i;
	IP2366_EnableDCDC(1,CfgData.OutputConfig.IsEnableOutput);
	IP2366_SetTypeCRole(TypeC_Disconnect);
  for(i=0;i<5;i++)delay_ms(100);				
	IP2366_SetTypeCRole(CfgData.OutputConfig.IsEnableOutput?TypeC_DRP:TypeC_DFP);
	IsEnableAdapterEmu=false;
	}
	
void AdapterEmuRender(void)
	{
	extern bool IsDispChargingINFO;
	u16 Color;
	int Temp;
	float Power;
	//��������
	Is2368Telem=true;
	if(!IsResultUpdated)return;
	RenderMenuBG();
	if(IsAdapterEnterError)
		{
		LCD_ShowChinese(14,22,"������ģ�⿪��ʧ�ܣ�",RED,LGRAY,0);
		switch(EmuErrorName)
			{
			case Error_VbatTooLow:LCD_ShowChinese(32,37,"��ص�ѹ����",YELLOW,LGRAY,0);break;
			case Error_CommFault:LCD_ShowChinese(32,37,"оƬͨ�Ŵ���",YELLOW,LGRAY,0);break;
			case Error_SysNotInDischarge:LCD_ShowChinese(17,37,"ϵͳδ���ڷŵ�ģʽ",YELLOW,LGRAY,0);break;
			case Error_SysFaultAsserted:LCD_ShowChinese(32,37,"ϵͳ���ֹ���",YELLOW,LGRAY,0);break;
			}		
		LCD_ShowChinese(32,61,"����",WHITE,LGRAY,0);
		LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		LCD_ShowChinese(86,61,"���˳�",WHITE,LGRAY,0);
		}
	else if(CState.VSysState!=VSys_State_Normal||CState.VBusState==VBUS_OverVolt||IsStopDueToFault)
		{
		LCD_ShowChinese(10,22,"ϵͳ���ֹ��ϣ�ģ��ֹͣ",RED,LGRAY,0);
		LCD_ShowChinese(32,61,"����",WHITE,LGRAY,0);
		LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		LCD_ShowChinese(86,61,"���˳�",WHITE,LGRAY,0);		
		IsStopDueToFault=true;
		}
	else if(CheckIfBattTooLow()||IsStopEmulationDueToLV)
		{		
		LCD_ShowChinese(10,22,"��ص������ͣ�ģ��ֹͣ",RED,LGRAY,0);
		LCD_ShowChinese(32,61,"����",WHITE,LGRAY,0);
		LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		LCD_ShowChinese(86,61,"���˳�",WHITE,LGRAY,0);
		//��ص������ͣ�ִ���˳�ģ�����
		if(!IsStopEmulationDueToLV)
			{
			ExitAdapterEmulation();
			IsStopEmulationDueToLV=true;
			}
		}
	else
		{
		if(IsSystemOverheating)LCD_ShowChinese(22,23,"ϵͳ���ȣ�ģ����ͣ",ORANGE,LGRAY,0);
		else LCD_ShowChinese(25,23,"������ģ���ѿ���",GREEN,LGRAY,0);
		//��ʾ���
		LCD_ShowChinese(6,44,"���",WHITE,LGRAY,0);
		LCD_ShowChar(30,44,':',WHITE,LGRAY,12,0);
		if(VBUS.IsTypeCConnected)
			{
			//��ʾ�ܹ��ʺ�Э��
			if(IsDispChargingINFO)
				{
				Power=fabsf(VTypec*ITypeC);
				if(Power<100)LCD_ShowFloatNum1(41,44,Power,2,WHITE,LGRAY,12);
				else if(Power<1000)LCD_ShowFloatNum1(41,44,Power,1,WHITE,LGRAY,12);
				else LCD_ShowIntNum(41,44,(int)Power,4,WHITE,LGRAY,12);
				LCD_ShowChar(82,44,'W',WHITE,LGRAY,12,0);
				if(VBUS.QuickChargeState==QuickCharge_PD&&VBUS.PDState!=PD_5VMode)switch(VBUS.PDState)
					{
					case PD_5VMode:break;
					case PD_7VMode:LCD_ShowString(103,44,"PD 7V",WHITE,LGRAY,12,0);break;
					case PD_9VMode:LCD_ShowString(103,44,"PD 9V",WHITE,LGRAY,12,0);break;
					case PD_12VMode:LCD_ShowString(103,44,"PD12V",CYAN,LGRAY,12,0);break;
					case PD_15VMode:LCD_ShowString(103,44,"PD15V",CYAN,LGRAY,12,0);break;
					case PD_20VMode:LCD_ShowString(103,44,"PD20V",YELLOW,LGRAY,12,0);break;
					case PD_28VMode:LCD_ShowString(103,44,"PDEPR",YELLOW,LGRAY,12,0);break;
					}
				//QC�ʹ�������
				else if(VBUS.QuickChargeState==QuickCharge_HV)LCD_ShowChinese(103,44,"��ѹ\0",WHITE,LGRAY,0);
				else if(VBUS.QuickChargeState==QuickCharge_HC)LCD_ShowChinese(103,44,"����\0",WHITE,LGRAY,0);
				else LCD_ShowChinese(103,44,"���ر�\0",WHITE,LGRAY,0);
				}
			else 
				{				
				LCD_ShowFloatNum1(41,44,VTypec,2,CYAN,LGRAY,12);
				LCD_ShowChar(82,44,'V',WHITE,LGRAY,12,0);
				LCD_ShowFloatNum1(103,44,fabsf(ITypeC),2,YELLOW,LGRAY,12);
				LCD_ShowChar(144,44,'A',WHITE,LGRAY,12,0);
				}
			}
		else 
			{
		  LCD_ShowString(41,44,"Type-C",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,44,"δ����",WHITE,LGRAY,0);
			}
		//��ʾ����
		LCD_ShowChinese(6,61,"���",WHITE,LGRAY,0);
		LCD_ShowChar(30,61,':',WHITE,LGRAY,12,0);
		//��ʾ���ʺ��¶�
		if(IsDispChargingINFO)
			{
			Power=fabsf(IBat)>MinimumCurrentFactor?IBat:0;
			Power=fabsf(Power*VBat);
			if(Power<100)LCD_ShowFloatNum1(41,61,Power,2,WHITE,LGRAY,12);
			else if(Power<1000)LCD_ShowFloatNum1(41,61,Power,1,WHITE,LGRAY,12);
			else LCD_ShowIntNum(41,61,(int)Power,4,WHITE,LGRAY,12);
			LCD_ShowChar(82,61,'W',WHITE,LGRAY,12,0);
			if(!ADCO.IsNTCOK)LCD_ShowString(103,61,"---",WHITE,LGRAY,12,0);
			else
				{
				Temp=(int)ADCO.Systemp;
				if(Temp<0)Color=DARKBLUE;	
				if(Temp<10)Color=BLUE;
				else if(Temp<40)Color=GREEN;
				else if(Temp<60)Color=YELLOW;
				else Color=RED;
				//��ʾ�¶�
				if(Temp<0)
					{
					Temp*=-1;
					LCD_ShowChar(103,61,'-',Color,LGRAY,12,0);
					LCD_ShowIntNum(103,61,Temp,2,Color,LGRAY,12);
					}
				else LCD_ShowIntNum(103,61,Temp,2,Color,LGRAY,12);
				LCD_ShowChinese12x12(136,61,"��\0",Color,LGRAY,12,0);
				}
			}
		//��ʾ��ص�ѹ����
		else
			{
			LCD_ShowFloatNum1(41,61,VBat,2,CYAN,LGRAY,12);
			LCD_ShowChar(82,61,'V',WHITE,LGRAY,12,0);
			LCD_ShowFloatNum1(103,61,fabsf(IBat)>MinimumCurrentFactor?fabsf(IBat):0,2,YELLOW,LGRAY,12);
			LCD_ShowChar(144,61,'A',WHITE,LGRAY,12,0);	
			}
		}		
	IsResultUpdated=false;	
	}	
	
void AdapterMenuKeyProc(void)
	{
	if(KeyState.KeyEvent==KeyEvent_ESC)
		{
		if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
		else SwitchingMenu(&SetMainMenu); //�����˳�״̬,����ESC��ص����˵�
		}
	KeyState.KeyEvent=KeyEvent_None;
	}	
	
const MenuConfigDef AdapterEmuMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&AdapterEmuRender, 
	&AdapterMenuKeyProc, //��������
	//�������ò˵�����Ҫ�ñ������
	"������ģ��",
	NULL,
	NULL, 
	NULL,
	//������˳����캯��
	&EnterAdapterEmulation, //����ʱ����ΪUFP
	&ExitAdapterEmulation //�˳�ģ��
	};
