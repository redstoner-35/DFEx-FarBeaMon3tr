#include "IP2366_REG.h"
#include "GUI.h"
#include "Config.h"
#include "lcd.h"
#include "ADC.h"
#include "pic.h"
#include "Key.h"
#include <math.h>
#include "INA226.h"

//平均次数
#define SADCAvgCount 5
//内部变量
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
bool IsDispChargingINFO=false; //实现涓流恒流指示和充电字样交替显示
ChipStatDef CState;
static bool IsShowEff=false;

//和IP2366连接获取数据
void IP2366_Telem(void)
	{
	char i;
	float Result;
	extern bool IsEnableHPGauge;	
	INADoutSreDef VBUSData;
	//判断是否启用传输
	if(!Is2368Telem)return;
	IsTelemOK=IP2366_GetChargerState(&BATT);
	IsTelemOK|=IP2366_GetVBUSState(&VBUS);
	IsTelemOK|=IP2366_ReadChipState(&CState);
	IsTelemOK|=IP2366_GetRecvPDO(&PDO);
	//开始进行INA226的测量	
	if(IsEnableHPGauge)
		{
		//尝试读取INA226进行高精度测量
		if(!INA226_GetBusInformation(&VBUSData))
			{
			//高精度测量失效，回到传统模式
			IsEnableHPGauge=false;
			}
		//高精度测量正常完成，直接使用INA226读回来的数据	
		else
			{		
			//对数据进行替换
			VBUS.VBUSVolt=VBUSData.BusVolt;
			VBUS.VBUSCurrent=fabsf(VBUSData.BusCurrent); 
			//电池处于放电状态，电流*-1表示正在放电
			if(BATT==Batt_discharging)VBUS.VBUSCurrent*=-1;
			}
		}
	//测量成功结束开始累加
	if(IsTelemOK)
		{
		//交替显示充电状态	
		if(ChargerAltDisplayTIM>0)ChargerAltDisplayTIM--;
		else
			{
			IsDispChargingINFO=IsDispChargingINFO?false:true;
			ChargerAltDisplayTIM=16;
			}
		//电压均衡
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
      //读取TypeC结果
			Result=VBUSAvgBuf[0]/(float)SADCAvgCount;
			Result*=(float)CfgData.TypeCVoltageCal;
			Result/=(float)1000;
			if(Result>4&&Result<30)VTypec=Result;
			//读取TypeC电流结果
			Result=VBUSAvgBuf[1]/(float)SADCAvgCount;
			Result*=(float)CfgData.TypeCAmpereCal;
			Result/=(float)1000;
			if(fabsf(Result)<7.5)ITypeC=Result;
			for(i=0;i<4;i++)VBUSAvgBuf[i]=0;
			}		
		if(VBUS.IsTypeCConnected)SleepTimer=480; //禁止睡眠
		}
	Is2368Telem=false;
	}

//计算效率并输出百分比
static float CalcEfficiency(void)	
	{
	float VBUSPower,VBatPower,eff;
	//芯片数据异常，显示不可用
	if(!IsTelemOK)return -1;
	//计算输入输出功率
	VBUSPower=fabsf(VTypec*ITypeC);
	VBatPower=fabsf(VBat*IBat);	
	//计算效率	
	switch(BATT)
		{
		case Batt_discharging:eff=VBUSPower/VBatPower;break; //反向放电时Eff=VBUS/Vbat
		case Batt_PreChage:
		case Batt_CCCharge:
		case Batt_CVCharge:eff=VBatPower/VBUSPower;break; //充电时Eff=Vbat/Vbus
		default:return -1; //其余情况效率不显示
		}
	//限幅数值并返回结果
	eff*=(float)100;
  if(eff>98)eff=98;
	if(eff<5)eff=5;
	return eff;
	}

//渲染函数
void MainMenuRenderProcess(void)
	{
	float Power;
	u16 Color;
	int Temp;
	extern bool IsSystemOverheating;
	extern bool OCState;
	extern bool IsEnableHPGauge;
	//判断是否启用老人模式	
	if(CfgData.EnableLargeMenu)
		{
		SwitchingMenu(&LargeMainMenu);
		return;
		}
	//启动传输
	Is2368Telem=true;
	if(!IsTelemOK||!IsResultUpdated)return;
	//显示TypeC电压电流
	LCD_ShowPicture(19,0,39,15,USBLogo);
	LCD_DrawLine(4,7,15,7,WHITE);
	LCD_DrawLine(61,7,75,7,WHITE);
	//每次更新Type-C状态的时候把背景抠干净
	if(ConnectState!=VBUS.IsTypeCConnected)
		{
		LCD_Fill(5,14,74,75,BLACK);
		LCD_Fill(86,18,146,57,BLACK);
		ConnectState=VBUS.IsTypeCConnected;
		}
  if(VBUS.IsTypeCConnected)		
		{
		LCD_Fill(9,18,74,75,BLACK);
		//电压
		Power=(VTypec>4&&VTypec<30)?VTypec:0;
		LCD_ShowFloatNum1(9,18,Power,2,IsEnableHPGauge?ORANGE:LIGHTGREEN,BLACK,12);
		LCD_ShowChar(62,18,'V',IsEnableHPGauge?ORANGE:LIGHTGREEN,BLACK,12,0);
		//电流
		Power=fabsf(ITypeC);
		if(Power>5.5||Power<0.05)Power=0;
		else Power=ITypeC; //电流值限制
		if(Power<0) //电流为负数加上负号
			{
			Power*=-1;
			LCD_ShowChar(9,32,'-',YELLOW,BLACK,12,0);	
			LCD_ShowFloatNum1(18,32,Power,Power<10?2:1,YELLOW,BLACK,12);
			}
		else LCD_ShowFloatNum1(9,32,Power,2,YELLOW,BLACK,12);
		LCD_ShowChar(62,32,'A',YELLOW,BLACK,12,0);	
		//功率
	  Power=fabsf(((VTypec>4&&VTypec<30)?VTypec:0)*Power);
		if(Power<10)LCD_ShowFloatNum1(9,46,Power,3,CYAN,BLACK,12);
    else if(Power<100)LCD_ShowFloatNum1(9,46,Power,2,CYAN,BLACK,12);
		else LCD_ShowFloatNum1(9,46,Power,1,CYAN,BLACK,12);
		LCD_ShowChar(62,46,'W',CYAN,BLACK,12,0);	
		//效率指示开启，显示系统效率
		if(IsShowEff)	
			{
			LCD_Fill(5,60,74,75,BLACK);
			LCD_ShowChinese(9,61,"效率\0",WHITE,BLACK,0);
			Power=CalcEfficiency();
			if(Power!=-1)
				{
				//正常效率输出，正常显示
				LCD_ShowFloatNum1(34,61,Power,1,WHITE,BLACK,12);
				LCD_ShowChar(65,61,'%',WHITE,BLACK,12,0);
				}
			//效率显示异常，提示N/A
			else LCD_ShowString(48,61,"N/A",WHITE,BLACK,12,0);
			}
		//效率指示关闭，显示快充指示
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
				//PD快充
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
				//QC和大电流快充
				else if(VBUS.QuickChargeState==QuickCharge_HV)LCD_ShowChinese(9,61,"高压\0",YELLOW,BLACK,0);
				else if(VBUS.QuickChargeState==QuickCharge_HC)LCD_ShowChinese(9,61,"高流\0",YELLOW,BLACK,0);
				}
			}			
		}
	else //显示未连接
		{
		VTypec=0;
		ITypeC=0;
		LCD_ShowString(19,28,"TYPE-C",CYAN,BLACK,12,0);
		LCD_ShowChinese(20,43,"未连接\0",YELLOW,BLACK,0);
		}
	//把框画完
	LCD_DrawLine(4,7,4,76,WHITE);
	LCD_DrawLine(75,7,75,76,WHITE);
	LCD_DrawLine(4,76,75,76,WHITE);
	//显示电池电压
	LCD_ShowPicture(105,1,24,15,BattICON);
	LCD_DrawLine(81,7,102,7,WHITE);
	LCD_DrawLine(131,7,152,7,WHITE);
	//电压
	LCD_ShowFloatNum1(86,18,VBat,2,LIGHTGREEN,BLACK,12);
	LCD_ShowChar(139,18,'V',LIGHTGREEN,BLACK,12,0);
	//电流
	LCD_Fill(86,32,135,57,BLACK);
	Power=fabsf(IBat)>MinimumCurrentFactor?IBat:0;
	if(Power<0) //电流为负数加上负号
			{
			LCD_ShowChar(86,32,'-',YELLOW,BLACK,12,0);	
		  if(Power>-10)LCD_ShowFloatNum1(95,32,Power,2,YELLOW,BLACK,12);
			else LCD_ShowFloatNum1(95,32,Power,1,YELLOW,BLACK,12);
			}
	else LCD_ShowFloatNum1(86,32,Power,2,YELLOW,BLACK,12);
	LCD_ShowChar(139,32,'A',YELLOW,BLACK,12,0);	
	//功率
	Power=fabsf(VBat*Power);
	if(Power<10)LCD_ShowFloatNum1(86,46,Power,3,CYAN,BLACK,12);
	else if(Power<100)LCD_ShowFloatNum1(86,46,Power,2,CYAN,BLACK,12);
  else LCD_ShowFloatNum1(86,46,Power,1,CYAN,BLACK,12);
  LCD_ShowChar(139,46,'W',CYAN,BLACK,12,0);	
	//显示系统状态
	if(IsSystemOverheating)
		LCD_ShowChinese(86,61,"过热\0",RED,BLACK,0);
	else if(CState.VSysState!=VSys_State_Normal||CState.VBusState==VBUS_OverVolt) //输出短路或者输入过压
		LCD_ShowChinese(86,61,"故障\0",RED,BLACK,0);
  else if(OCState)
		LCD_ShowChinese(86,61,IsDispChargingINFO?"过充":"保护",YELLOW,BLACK,0);
  else switch(BATT)			//根据枚举状态显示
		{
		case Batt_StandBy:LCD_ShowChinese(86,61,"待机\0",WHITE,BLACK,0);break;
		case Batt_PreChage:
			LCD_ShowChinese(86,61,IsDispChargingINFO?"充电":"涓流\0",MAGENTA,BLACK,0);
			break;
		case Batt_CCCharge:
			LCD_ShowChinese(86,61,IsDispChargingINFO?"充电":"恒流\0",YELLOW,BLACK,0);
			break;
		case Batt_CVCharge:
			LCD_ShowChinese(86,61,IsDispChargingINFO?"充电":"恒压\0",GBLUE,BLACK,0);
			break;
		case Batt_ChgWait:
			LCD_ShowChinese(86,61,IsDispChargingINFO?"充电":"暂停\0",YELLOW,BLACK,0);
			break;
		case Batt_ChgDone:LCD_ShowChinese(86,61,"充满\0",LIGHTGREEN,BLACK,0);break;
		case Batt_ChgError:LCD_ShowChinese(86,61,IsDispChargingINFO?"充电":"超时\0",ORANGE,BLACK,0);break;
		case Batt_discharging:LCD_ShowChinese(86,61,"放电\0",WHITE,BLACK,0);break;
		}
	//温度显示
	if(!ADCO.IsNTCOK||ADCO.Systemp<-9)LCD_ShowString(117,61,"--",WHITE,BLACK,12,0);
	else
		{
		Temp=iroundf(ADCO.Systemp);
		if(Temp<0)Color=DARKBLUE;	
		else if(Temp<10)Color=BLUE;
		else if(Temp<CfgData.OverHeatLockTemp-20)Color=GREEN;
		else if(Temp<CfgData.OverHeatLockTemp-8)Color=YELLOW;
		else Color=RED;
	  //显示温度
		if(Temp<0)
			{
			Temp*=-1;
			LCD_ShowChar(117,61,'-',Color,BLACK,12,0);
			LCD_ShowChar(127,61,0x30+Temp,Color,BLACK,12,0);
			}
		else LCD_ShowIntNum(117,61,Temp,2,Color,BLACK,12);
		LCD_ShowChinese12x12(135,61,"℃\0",Color,BLACK,12,0);
		//画完剩下的线
		LCD_DrawLine(81,7,81,76,WHITE);
		LCD_DrawLine(152,7,152,76,WHITE);
		LCD_DrawLine(81,76,152,76,WHITE);
		}
  //本次显示完毕，等待数据更新后再刷新
  IsTelemOK=false;		
	IsResultUpdated=false;
	}

//按键处理
void EnterAdvModeProc(void);	
	
void MainMenuKeyProcess(void)
	{
	//同时按住上下键进入设置
	if(KeyState.KeyEvent==KeyEvent_BothEnt)
		{
		if(!CfgData.EnableAdvAccess)SwitchingMenu(&EasySetMainMenu); //进入简易模式
		else EnterAdvModeProc(); //进入高级模式
		}
	if(KeyState.KeyEvent==KeyEvent_Up)IsShowEff=true;
	if(KeyState.KeyEvent==KeyEvent_Down)
		{
		IsShowEff=false;
		QCState=QuickCharge_PlaceHolder; //通过把界面设置为Placeholder进行复位
		}
	KeyState.KeyEvent=KeyEvent_None;
	}

const MenuConfigDef MainMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&MainMenuRenderProcess, //渲染函数
	&MainMenuKeyProcess, //按键处理
	//不是设置菜单不需要用别的事情
	NULL,
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};
