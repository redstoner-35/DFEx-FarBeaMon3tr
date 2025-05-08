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

//外部变量
extern bool Is2368Telem;
extern IP2366VBUSStateDef VBUS;
extern float VTypec,ITypeC,VBat,IBat;
extern bool IsResultUpdated;
extern bool IsSystemOverheating;
extern ChipStatDef CState;
	
//内部变量
bool IsEnableAdapterEmu=false; //是否开启适配器模拟
bool IsStopEmulationDueToLV=false; //是否已停止模拟
bool IsStopDueToFault=false; //是否出现系统故障
static bool IsAdapterEnterError=false; //进入错误监测
static AdapEmuErrorDef EmuErrorName;
	
//内部函数，检查电池是否过低
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
	VMin=(VMin*BattCellCount)+0.2; //计算得出最低电压
	if(ADCO.Vbatt<VMin)return true;
  //正常进入
	return false;
	}	
	
//进入适配器模拟
void EnterAdapterEmulation(void)
	{
	BatteryStateDef BattState;
	//标记通信错误
	EmuErrorName=Error_CommFault;
	//获取芯片当前状态
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
	//检查电池电压是否过低
	else if(CheckIfBattTooLow())
		{
	  IsAdapterEnterError=true;
	  EmuErrorName=Error_VbatTooLow;
		}
	//配置Type-C
	else if(!IP2366_SetTypeCRole(TypeC_UFP))IsAdapterEnterError=true;
  else if(!IP2366_EnableDCDC(false,true))IsAdapterEnterError=true;   //开启UFP模式，强制启用放电
	else IsAdapterEnterError=false;
	//判断是否进入成功
	if(!IsAdapterEnterError)IsEnableAdapterEmu=true; //成功进入
	IsResultUpdated=true;
	IsStopDueToFault=false;
	IsStopEmulationDueToLV=false; //清除标记位
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
	//启动传输
	Is2368Telem=true;
	if(!IsResultUpdated)return;
	RenderMenuBG();
	if(IsAdapterEnterError)
		{
		LCD_ShowChinese(14,22,"适配器模拟开启失败！",RED,LGRAY,0);
		switch(EmuErrorName)
			{
			case Error_VbatTooLow:LCD_ShowChinese(32,37,"电池电压过低",YELLOW,LGRAY,0);break;
			case Error_CommFault:LCD_ShowChinese(32,37,"芯片通信错误",YELLOW,LGRAY,0);break;
			case Error_SysNotInDischarge:LCD_ShowChinese(17,37,"系统未处于放电模式",YELLOW,LGRAY,0);break;
			case Error_SysFaultAsserted:LCD_ShowChinese(32,37,"系统出现故障",YELLOW,LGRAY,0);break;
			}		
		LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
		LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
		}
	else if(CState.VSysState!=VSys_State_Normal||CState.VBusState==VBUS_OverVolt||IsStopDueToFault)
		{
		LCD_ShowChinese(10,22,"系统出现故障，模拟停止",RED,LGRAY,0);
		LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
		LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);		
		IsStopDueToFault=true;
		}
	else if(CheckIfBattTooLow()||IsStopEmulationDueToLV)
		{		
		LCD_ShowChinese(10,22,"电池电量过低，模拟停止",RED,LGRAY,0);
		LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
		LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
		//电池电量过低，执行退出模拟操作
		if(!IsStopEmulationDueToLV)
			{
			ExitAdapterEmulation();
			IsStopEmulationDueToLV=true;
			}
		}
	else
		{
		if(IsSystemOverheating)LCD_ShowChinese(22,23,"系统过热，模拟暂停",ORANGE,LGRAY,0);
		else LCD_ShowChinese(25,23,"适配器模拟已开启",GREEN,LGRAY,0);
		//显示输出
		LCD_ShowChinese(6,44,"输出",WHITE,LGRAY,0);
		LCD_ShowChar(30,44,':',WHITE,LGRAY,12,0);
		if(VBUS.IsTypeCConnected)
			{
			//显示总功率和协议
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
				//QC和大电流快充
				else if(VBUS.QuickChargeState==QuickCharge_HV)LCD_ShowChinese(103,44,"高压\0",WHITE,LGRAY,0);
				else if(VBUS.QuickChargeState==QuickCharge_HC)LCD_ShowChinese(103,44,"高流\0",WHITE,LGRAY,0);
				else LCD_ShowChinese(103,44,"快充关闭\0",WHITE,LGRAY,0);
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
		  LCD_ShowChinese(86,44,"未连接",WHITE,LGRAY,0);
			}
		//显示输入
		LCD_ShowChinese(6,61,"电池",WHITE,LGRAY,0);
		LCD_ShowChar(30,61,':',WHITE,LGRAY,12,0);
		//显示功率和温度
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
				//显示温度
				if(Temp<0)
					{
					Temp*=-1;
					LCD_ShowChar(103,61,'-',Color,LGRAY,12,0);
					LCD_ShowIntNum(103,61,Temp,2,Color,LGRAY,12);
					}
				else LCD_ShowIntNum(103,61,Temp,2,Color,LGRAY,12);
				LCD_ShowChinese12x12(136,61,"℃\0",Color,LGRAY,12,0);
				}
			}
		//显示电池电压电流
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
		else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
		}
	KeyState.KeyEvent=KeyEvent_None;
	}	
	
const MenuConfigDef AdapterEmuMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&AdapterEmuRender, 
	&AdapterMenuKeyProc, //按键处理
	//不是设置菜单不需要用别的事情
	"适配器模拟",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&EnterAdapterEmulation, //进入时配置为UFP
	&ExitAdapterEmulation //退出模拟
	};
