#include "IP2366_REG.h"
#include "GUI.h"
#include "Config.h"
#include "lcd.h"
#include "ADC.h"
#include "Key.h"
#include "LogSystem.h"
#include <math.h>

//外部变量
extern bool IsTelemOK;
extern float VTypec,ITypeC,VBat,IBat;
extern ChipStatDef CState;
extern bool Is2368Telem;
extern RecvPDODef PDO;
extern bool IsUpdateCDUI;
extern BatteryStateDef BATT;
extern IP2366VBUSStateDef VBUS;
extern bool IsResultUpdated;

//内部变量
static bool IsShowTypeCState=false;

//显示AH数
static void LargeMenu_DisplayAh(float Ah,bool IsDis)
	{
	float Power;
	u16 Color=IsDis?CYAN:WHITE;
	LCD_Fill(99,18,150,31,BLACK); //消除掉旧的图标
	if(Ah<10) //小于10Ah使用mAH显示
		{
		LCD_ShowIntNum(99,18,iroundf(Ah*1000),4,Color,BLACK,12);
		LCD_ShowString(132,18,"mAh",Color,BLACK,12,0);
		}
	else if(Ah<100000)//使用浮点显示
		{
		LCD_ShowString(132,18," Ah",Color,BLACK,12,0);
		if(Ah<100)LCD_ShowFloatNum1(99,18,Ah,2,Color,BLACK,12);  //99.99显示
		else if(Ah<100)LCD_ShowFloatNum1(99,18,Ah,1,Color,BLACK,12);  //999.9显示
		else LCD_ShowIntNum(99,18,iroundf(Ah),5,Color,BLACK,12); //9999显示
		}
	else //使用KAH显示
		{
		Power=Ah/1000;
    if(Power<10)LCD_ShowFloatNum1(99,18,Power,3,Color,BLACK,12);  //9.999显示
		else if(Power<100)LCD_ShowFloatNum1(99,18,Power,2,Color,BLACK,12);  //99.99显示
		else if(Power<1000)LCD_ShowFloatNum1(99,18,Power,1,Color,BLACK,12);  //999.9显示
		else LCD_ShowIntNum(99,18,iroundf(Power),5,Color,BLACK,12); //9999显示
		LCD_ShowString(132,18,"KAh",Color,BLACK,12,0);
		}	
	}
//显示充放电能量
static void LargeMenu_DisplayWh(float Wh,bool IsDis)
	{
	float Power;
	u16 Color=IsDis?CYAN:WHITE;	
	LCD_Fill(99,34,150,46,BLACK); //消除掉旧的图标
	if(Wh<10000)LCD_ShowString(132,34," Wh",Color,BLACK,12,0);
	if(Wh<10)LCD_ShowFloatNum1(99,34,Wh,3,Color,BLACK,12); //9.999显示
	else if(Wh<100)LCD_ShowFloatNum1(99,34,Wh,2,Color,BLACK,12); //99.99显示	
	else if(Wh<1000)LCD_ShowFloatNum1(99,34,Wh,1,Color,BLACK,12); //999.9显示
	else if(Wh<10000)LCD_ShowIntNum(99,34,iroundf(Wh),5,Color,BLACK,12); //9999显示
	else	
	  {
		Power=Wh;
	  if(Power<10)LCD_ShowFloatNum1(99,34,Power,2,Color,BLACK,12); //9.99显示
		else if(Power<100)LCD_ShowFloatNum1(99,34,Power,1,Color,BLACK,12); //99.9显示	
		else if(Power<1000)LCD_ShowIntNum(99,34,iroundf(Power),3,Color,BLACK,12); //999显示
		LCD_ShowString(132,34,"KWh",Color,BLACK,12,0);	
		}
	}	
//显示充放电时间
static void LargeMenu_ShowTime(long TimeIN,bool IsDis)
	{
	long time;
	u16 ICONColor=IsDis?LIGHTGREEN:WHITE;
	u16 TextColor=IsDis?ORANGE:YELLOW;
	if(TimeIN>86400*30) //时长超过一个月
		{
		time=TimeIN/(86400*30); //计算出月数
		LCD_ShowIntNum(99,49,time,2,TextColor,BLACK,12);
		LCD_ShowChinese12x12(115,49,"月\0",ICONColor,BLACK,12,0);
		time=(TimeIN%(86400*30))/86400; //计算出天数
		LCD_ShowIntNum(128,49,time,2,TextColor,BLACK,12);
		LCD_ShowChinese12x12(144,49,"天\0",ICONColor,BLACK,12,0);			
		}
	else if(TimeIN>86400) //时长超过一天
		{
		time=TimeIN/86400; //计算出天数
		LCD_ShowIntNum(99,49,time,2,TextColor,BLACK,12);
		LCD_ShowChinese12x12(115,49,"天\0",ICONColor,BLACK,12,0);
		time=(TimeIN%86400)/3600; //计算出小时数
		LCD_ShowIntNum(128,49,time,2,TextColor,BLACK,12);
		LCD_ShowChinese12x12(144,49,"时\0",ICONColor,BLACK,12,0);		
		}
	else if(TimeIN>3600)
		{
		time=TimeIN/3600; //计算出小时数
		LCD_ShowIntNum(99,49,time,2,TextColor,BLACK,12);
		LCD_ShowChinese12x12(115,49,"时\0",ICONColor,BLACK,12,0);
		time=(TimeIN%3600)/60; //计算出分钟数
		LCD_ShowIntNum(128,49,time,2,TextColor,BLACK,12);
		LCD_ShowChinese12x12(144,49,"分\0",ICONColor,BLACK,12,0);			
		}
  else
		{
		time=TimeIN/60; //计算出分钟数
		LCD_ShowIntNum(99,49,time,2,TextColor,BLACK,12);
		LCD_ShowChinese12x12(115,49,"分\0",ICONColor,BLACK,12,0);
		time=TimeIN%60;
		LCD_ShowIntNum(128,49,time,2,TextColor,BLACK,12);
		LCD_ShowChinese12x12(144,49,"秒\0",ICONColor,BLACK,12,0);		 
		}	
	}	
	
//显示电池状态
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
	//检测UI是否结束渲染
	Is2368Telem=true;
	time=(BATT==Batt_discharging)?LogData.DischargeTime:LogData.ChargeTime; //获取充放电时间
	if(!IsTelemOK)return;
	if(BATT!=Batt_StandBy&&fabsf(ADCO.Ibatt)>MinimumCurrentFactor)	//系统处于开机状态，且日志正在累加，按照结果渲染数据
		{
		if(time>86400&&!IsResultUpdated)return; //时间超过1个小时可以使用较快的刷新频率
		else if(time<86400&&!IsUpdateCDUI)return;	 //时间一个小时内使用较慢的刷新频率让秒数跳字同步	
		}
	else if(!IsResultUpdated)return;  //静态阶段使用快速更新系统
	//正常渲染
	LCD_DrawRectangle(0,0,159,79,WHITE);	
	//电压
	LCD_ShowFloatNum1(3,3,VBat,2,LIGHTGREEN,BLACK,24);
	LCD_ShowChar(73,3,'V',LIGHTGREEN,BLACK,24,0);
	//电流
	LCD_Fill(3,28,84,76,BLACK);
	Power=fabsf(IBat)>MinimumCurrentFactor?IBat:0;
	if(Power<0) //电流为负数加上负号
			{
			LCD_ShowChar(3,28,'-',YELLOW,BLACK,24,0);	
			if(Power>-10)LCD_ShowFloatNum1(16,28,Power,2,YELLOW,BLACK,24);
			else LCD_ShowFloatNum1(16,28,Power,1,YELLOW,BLACK,24);
			}
	else LCD_ShowFloatNum1(3,28,Power,2,YELLOW,BLACK,24);
	LCD_ShowChar(73,28,'A',YELLOW,BLACK,24,0);	
	//功率
	Power=fabsf(VBat*Power);
	if(Power<10)LCD_ShowFloatNum1(3,53,Power,3,CYAN,BLACK,24);
	else if(Power<100)LCD_ShowFloatNum1(3,53,Power,2,CYAN,BLACK,24);
  else LCD_ShowFloatNum1(3,53,Power,1,CYAN,BLACK,24);
  LCD_ShowChar(73,53,'W',CYAN,BLACK,24,0);	
	//温度
	if(!ADCO.IsNTCOK)LCD_ShowString(99,3,"---.-",WHITE,BLACK,12,0);
	else
		{
		Temp=iroundf(ADCO.Systemp);
		if(Temp<0)Color=DARKBLUE;	
		if(Temp<10)Color=BLUE;
		else if(Temp<CfgData.OverHeatLockTemp-20)Color=GREEN;
		else if(Temp<CfgData.OverHeatLockTemp-8)Color=YELLOW;
		else Color=RED;	
		//0度
		if(ADCO.Systemp<0)
			{
			LCD_ShowChar(99,3,'-',WHITE,BLACK,12,0);
			if(ADCO.Systemp>-10)LCD_ShowFloatNum1(107,3,ADCO.Systemp*-1,2,Color,BLACK,12);
			else LCD_ShowFloatNum1(107,3,ADCO.Systemp*-1,1,Color,BLACK,12);
			}
		//其他温度
		else if(ADCO.Systemp<99)LCD_ShowFloatNum1(99,3,ADCO.Systemp*-1,2,Color,BLACK,12);
		else LCD_ShowFloatNum1(99,3,ADCO.Systemp*-1,1,Color,BLACK,12);
		LCD_ShowChinese12x12(144,3,"℃\0",Color,BLACK,12,0);
		}
	//容量显示
	if(BATT==Batt_discharging)LargeMenu_DisplayAh(LogData.TotalDischargeAh,true);
	else LargeMenu_DisplayAh(LogData.TotalChargeAh,false);
	//能量显示
  if(BATT==Batt_discharging)LargeMenu_DisplayWh(LogData.TotalDischargeWh,true);
	else LargeMenu_DisplayWh(LogData.TotalChargeWh,false);
	//充电时间显示
	LargeMenu_ShowTime(time,BATT==Batt_discharging?true:false);
  //显示电池状态
	if(CState.VSysState!=VSys_State_Normal||CState.VBusState==VBUS_OverVolt)IsShowBatt=false;
	else if(IsSystemOverheating)IsShowBatt=false;
  else if(OCState)IsShowBatt=true;		
	else if(BATT!=Batt_StandBy)IsShowBatt=true;
	else IsShowBatt=false;	
	//显示实际的内容	
	LCD_ShowHybridString(99,64,IsShowBatt?"电池:":"系统:",WHITE,BLACK,0);
	if(IsSystemOverheating)
		LCD_ShowChinese(132,64,"过热\0",RED,BLACK,0);
	else if(CState.VSysState!=VSys_State_Normal||CState.VBusState==VBUS_OverVolt) //输出短路或者输入过压
		LCD_ShowChinese(132,64,"故障\0",RED,BLACK,0);
  else if(OCState)
		LCD_ShowChinese(132,64,IsDispChargingINFO?"过充":"保护",YELLOW,BLACK,0);
  else switch(BATT)			//根据枚举状态显示
		{
		case Batt_StandBy:LCD_ShowChinese(132,64,"待机\0",WHITE,BLACK,0);break;
		case Batt_PreChage:
			LCD_ShowChinese(132,64,IsDispChargingINFO?"充电":"涓流\0",MAGENTA,BLACK,0);
			break;
		case Batt_CCCharge:
			LCD_ShowChinese(132,64,IsDispChargingINFO?"充电":"恒流\0",YELLOW,BLACK,0);
			break;
		case Batt_CVCharge:
			LCD_ShowChinese(132,64,IsDispChargingINFO?"充电":"恒压\0",GBLUE,BLACK,0);
			break;
		case Batt_ChgWait:
			LCD_ShowChinese(132,64,IsDispChargingINFO?"充电":"暂停\0",YELLOW,BLACK,0);
			break;
		case Batt_ChgDone:LCD_ShowChinese(132,64,"充满\0",LIGHTGREEN,BLACK,0);break;
		case Batt_ChgError:LCD_ShowChinese(132,64,IsDispChargingINFO?"充电":"超时\0",ORANGE,BLACK,0);break;
		case Batt_discharging:LCD_ShowChinese(132,64,"放电\0",CYAN,BLACK,0);break;
		}
	//本次显示完毕，等待数据更新后再刷新
  IsTelemOK=false;	
  IsResultUpdated=false;		
	IsUpdateCDUI=false;
	}
	
//显示Type-C状态
void RenderTypeCState(void)
	{
	float Power;
	extern bool IsDispChargingINFO;
	extern bool IsEnableHPGauge;	
	//启动传输
	Is2368Telem=true;
	if(!VBUS.IsTypeCConnected)
		{
		//Type-C被移除，立即退出本菜单
		ClearScreen();
		IsShowTypeCState=false;
		return;
		}
	if(!IsTelemOK||!IsResultUpdated)return;
	//正常渲染
	LCD_DrawRectangle(0,0,159,79,WHITE);	
	//电压
	LCD_ShowFloatNum1(3,3,VTypec,2,IsEnableHPGauge?ORANGE:LIGHTGREEN,BLACK,24);
	LCD_ShowChar(73,3,'V',IsEnableHPGauge?ORANGE:LIGHTGREEN,BLACK,24,0);
	//电流
	LCD_Fill(3,28,84,76,BLACK);
	Power=fabsf(ITypeC)>MinimumCurrentFactor?ITypeC:0;
	if(Power<0) //电流为负数加上负号
			{
			LCD_ShowChar(3,28,'-',YELLOW,BLACK,24,0);	
			if(Power>-10)LCD_ShowFloatNum1(16,28,Power,2,YELLOW,BLACK,24);
			else LCD_ShowFloatNum1(16,28,Power,1,YELLOW,BLACK,24);
			}
	else LCD_ShowFloatNum1(3,28,Power,2,YELLOW,BLACK,24);
	LCD_ShowChar(73,28,'A',YELLOW,BLACK,24,0);	
	//功率
	Power=fabsf(VTypec*Power);
	if(Power<10)LCD_ShowFloatNum1(3,53,Power,3,CYAN,BLACK,24);
	else if(Power<100)LCD_ShowFloatNum1(3,53,Power,2,CYAN,BLACK,24);
  else LCD_ShowFloatNum1(3,53,Power,1,CYAN,BLACK,24);
  LCD_ShowChar(73,53,'W',CYAN,BLACK,24,0);	
	//PDO输入和VBUS状态显示
	LCD_Fill(102,9,152,36,BLACK);
	LCD_DrawRectangle(102,9,152,36,WHITE);
	LCD_Fill(104,4,150,15,BLACK);
	if(!IsDispChargingINFO)	
		{		
		LCD_ShowHybridString(104,4,"PDO广播",WHITE,BLACK,0);
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
	else //显示输入状态
		{
		LCD_ShowHybridString(105,4,"Type-C",WHITE,BLACK,0);
		if(CState.VBusState==VBUS_NoPower)LCD_ShowHybridString(115,19,"断开",LGRAY,BLACK,0);	
		else if(CState.VBusState==VBUS_OverVolt)LCD_ShowHybridString(115,19,"过压",RED,BLACK,0);
		else switch(CState.VSysState)
			{
			case VSys_State_Normal:LCD_ShowHybridString(115,19,"正常",GREEN,BLACK,0);break;
			case VSys_State_OCP:LCD_ShowHybridString(115,19,"过流",YELLOW,BLACK,0);break;
			case VSys_State_Short:LCD_ShowHybridString(115,19,"短路",RED,BLACK,0);break;
			}
		}		
	//协议显示
	LCD_Fill(102,46,152,73,BLACK);
	LCD_DrawRectangle(102,46,152,73,WHITE);
	LCD_Fill(112,41,142,52,BLACK);
	LCD_ShowHybridString(115,41,"协议",WHITE,BLACK,0);
	//PD快充
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
	//QC和大电流快充
	else if(VBUS.QuickChargeState==QuickCharge_HV)LCD_ShowChinese(115,56,"高压\0",YELLOW,BLACK,0);
	else if(VBUS.QuickChargeState==QuickCharge_HC)LCD_ShowChinese(115,56,"高流\0",YELLOW,BLACK,0);
	//无快充
	else LCD_ShowString(115,56,"N/A",WHITE,BLACK,12,0);
	//本次显示完毕，等待数据更新后再刷新
  IsTelemOK=false;		
	IsResultUpdated=false;
	}	

//按键处理
void EnterAdvModeProc(void);	
	
void LargeMainMenuKeyProcess(void)
	{
	//同时按住上下键进入设置
	if(KeyState.KeyEvent==KeyEvent_BothEnt)
		{
		if(!CfgData.EnableAdvAccess)SwitchingMenu(&EasySetMainMenu); //进入简易模式
		else EnterAdvModeProc(); //进入高级模式
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
	
//渲染处理
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
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&LargeMenuRenderProc, //渲染函数
	&LargeMainMenuKeyProcess, //按键处理
	//不是设置菜单不需要用别的事情
	NULL,
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};
	
