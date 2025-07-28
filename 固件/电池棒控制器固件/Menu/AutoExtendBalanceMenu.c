#include "GUI.h"
#include "LogSystem.h"
#include "ADC.h"
#include "IP2366_REG.h"
#include <string.h>
#include <math.h>
#include "Key.h"
#include "delay.h"
#include "BalanceMgmt.h"
	
extern bool IsUpdateBalUI;
extern bool IsTimeMet;
extern BatteryStateDef BATT;
extern bool IsDispChargingINFO;
extern bool Is2366Telem;

//函数声明
bool SetSystemDischargeState(void);

//全局变量
AutoBalanFSMDef AutoBalState;
static char AutoBalTIM;
bool IsCPortBreaked=false;

void AutoBalTIMHandler(void)
	{
	if(AutoBalTIM>0)AutoBalTIM--;
	}
	
//重置充电系统到默认值
static void ResetChargerSystemToNormal(void)
	{
	AutoBalState=AutoBalance_End_Abnormal;
	AutoBalTIM=0;
	BalanceForceEnableTIM=0;
	IsCPortBreaked=false;
	SetSystemDischargeState();
	}

//利用类似typec disconnect的办法强制断开输出
static void BreakCPortConnection(void)
	{
	SetSystemDischargeState();
	delay_ms(10);
	IsCPortBreaked=true;
	//设置为DFP模式强制关闭Source
	IP2366_SetTypeCRole(TypeC_DFP);
	}
	
//在自动均衡循环结束后退回到主界面的操作
static void BalanceEndGotoMainMenuProcess(bool IsNormalExit)
	{
	BalanceForceEnableTIM=0;
	if(IsNormalExit)LogData.UnbalanceBatteryAh=0; //本次均衡已完成	
	RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data); //计算运行日志的CRC32
	WriteRuntimeLogToROM(); //保存日志
	//回到主界面
	ClearScreen(); //清屏
	SwitchingMenu(&MainMenu);
	}	
	
//自动均衡FSM处理
void AutoBalFSMHandler(void)
	{
	extern int SleepTimer;	
  //进行系统的配置		
	Is2366Telem=true; 
	SleepTimer=480; //均衡运行期间禁止系统复位，休眠时间复位为一分钟
	switch(AutoBalState)
		{
		//等待充电
		case AutoBalance_WaitBattCharge:
		  //电池处在待机，充电完毕和放电过程中等待
			if(BATT!=Batt_StandBy&&BATT!=Batt_ChgDone)AutoBalTIM=80;
		  BalanceForceEnableTIM=3600*8*5; //电池在充电过程中，禁止系统对均衡进行计时。
			//当自动均衡计时器停止计时后，关闭充放电并跳转到等待状态
		  if(!AutoBalTIM)
				{
				IsUpdateBalUI=true; //状态变化时刷新UI
				AutoBalState=AutoBalance_RunningBalance;
				BreakCPortConnection();  //强制断开输出
				break;
				}
			//用户按下ESC，或者电池进入放电状态，强制退出
			if(KeyState.KeyEvent==KeyEvent_ESC||BATT==Batt_discharging)
				{
				IsUpdateBalUI=true;
				AutoBalState=AutoBalance_End_Abnormal;
				break;
				}
		  //状态变化时刷新UI
			if(IsDispChargingINFO==IsTimeMet)break;
			IsUpdateBalUI=true; //状态变化时刷新UI
			IsTimeMet=IsDispChargingINFO;
		  break;
		//充电结束，系统关闭充放电，开始计时
		case AutoBalance_RunningBalance:
			if(!(BalanceForceEnableTIM%8)&&!IsTimeMet)
				{
				IsUpdateBalUI=true; //每秒更新一次UI
				IsTimeMet=true; //只需要更新一次就行，避免重复更新
				LogData.BalanceTime++;
				}
			else if(BalanceForceEnableTIM%8)IsTimeMet=false; //非更新时间，进行更新		
		  //状态检测
			if(KeyState.KeyEvent==KeyEvent_ESC||ADCO.Vbatt<10.1)
				{
				//电池电压小于10.1v，强制退出
				IsUpdateBalUI=true;
				BalanceForceEnableTIM=0;
				AutoBalState=AutoBalance_End_Abnormal;
				SetSystemDischargeState();  //重新开启输入输出
				break;
				}
			if(BalanceForceEnableTIM<=0)
				{
				//均衡计时结束，充电被重新打开并跳转到补电状态检测
				IsUpdateBalUI=true;
				AutoBalState=AutoBalance_ReChargingWait;
				AutoBalTIM=40;
				SetSystemDischargeState();  //重新开启输入输出
				break;
				}
			break;
		//重新开始充电流程
		case AutoBalance_ReChargingWait:
			//系统充电开始，跳转到补电阶段
			if(BATT!=Batt_StandBy&&BATT!=Batt_discharging)AutoBalState=AutoBalance_ReCharging;
		  //计时结束仍然没开始补电，跳转到结束阶段
		  if(!AutoBalTIM)
				{
				IsUpdateBalUI=true; //状态变化时刷新UI
				AutoBalState=AutoBalance_End;
				AutoBalTIM=24;
				break;
				}
		  break;
		//充电流程进行中
		case AutoBalance_ReCharging:
		  //电池处在待机，充电完毕过程中，等待
			BalanceForceEnableTIM=0;
			if(BATT!=Batt_StandBy&&BATT!=Batt_ChgDone)AutoBalTIM=80;
			//当自动均衡计时器停止计时或者变为放电状态，标记补电循环已经结束，退出
		  if(!AutoBalTIM||BATT==Batt_discharging)
				{
				IsUpdateBalUI=true; //状态变化时刷新UI
				AutoBalState=AutoBalance_End;
				AutoBalTIM=24;
				break;
				}
			//用户按下ESC，强制退出
			if(KeyState.KeyEvent==KeyEvent_ESC)
				{
				IsUpdateBalUI=true;
				AutoBalState=AutoBalance_End_Abnormal;
				SetSystemDischargeState();  //重新开启输入输出
				break;
				}
		  //状态变化时刷新UI
			if(IsDispChargingINFO==IsTimeMet)break;
			IsUpdateBalUI=true; //状态变化时刷新UI
			IsTimeMet=IsDispChargingINFO;		
      break;				
		//自动均衡循环正常结束
		case AutoBalance_End:
			if(AutoBalTIM||BATT!=Batt_StandBy)break;
			BalanceEndGotoMainMenuProcess(true);
			break;
		//自动均衡循环异常
		case AutoBalance_End_Abnormal:
		  if(KeyState.KeyEvent!=KeyEvent_ESC)break;
			BalanceEndGotoMainMenuProcess(false);
			break;		
		}
	//清除按键事件
  KeyState.KeyEvent=KeyEvent_None;
	}

//自动均衡渲染
void AutoBalMenuRenderHandler(void)
	{
	int H,M,S,Temp;
	u16 Color;
	extern float VBat,IBat;
	float Power;
	if(!IsUpdateBalUI&&KeyState.KeyEvent==KeyEvent_None)return;
	//实际的渲染流程
	RenderMenuBG(); //显示背景	
	switch(AutoBalState)
		{
		//正在充电中
		case AutoBalance_ReCharging:
		case AutoBalance_WaitBattCharge:
			switch(BATT)
				{
				case Batt_ChgDone:LCD_ShowChinese(33,22,"确认充满状态…",GREEN,LGRAY,0);break;
				case Batt_ChgError:LCD_ShowChinese(33,22,"电池包充电异常",RED,LGRAY,0);break;
				default:
					if(AutoBalState==AutoBalance_ReCharging)LCD_ShowChinese(33,22,"电池二次补电中",ORANGE,LGRAY,0);
					else LCD_ShowChinese(33,22,"电池包充电中…",ORANGE,LGRAY,0);
					break;
				}
			LCD_ShowChinese(6,40,"电池",WHITE,LGRAY,0);
			LCD_ShowChar(30,40,':',WHITE,LGRAY,12,0);
			//显示功率和温度
			if(IsDispChargingINFO)
					{
					Power=fabs(IBat)>MinimumCurrentFactor?IBat:0;
					Power=fabs(Power*VBat);
					if(Power<100)LCD_ShowFloatNum1(41,40,Power,2,WHITE,LGRAY,12);
					else if(Power<1000)LCD_ShowFloatNum1(41,40,Power,1,WHITE,LGRAY,12);
					else LCD_ShowIntNum(41,40,(int)Power,4,WHITE,LGRAY,12);
					LCD_ShowChar(82,40,'W',WHITE,LGRAY,12,0);
					if(!ADCO.IsNTCOK)LCD_ShowString(103,40,"---",WHITE,LGRAY,12,0);
					else
						{
						Temp=(int)ADCO.Systemp;
						if(Temp<0)Color=DARKBLUE;	
						else if(Temp<10)Color=BLUE;
						else if(Temp<CfgData.OverHeatLockTemp-20)Color=GREEN;
						else if(Temp<CfgData.OverHeatLockTemp-8)Color=YELLOW;
						else Color=RED;
						//显示温度
						if(Temp<0)
							{
							Temp*=-1;
							LCD_ShowChar(103,40,'-',Color,LGRAY,12,0);
							LCD_ShowIntNum(103,40,Temp,2,Color,LGRAY,12);
							}
						else LCD_ShowIntNum(103,40,Temp,2,Color,LGRAY,12);
						LCD_ShowChinese12x12(136,40,"℃\0",Color,LGRAY,12,0);
						}
				}
			//显示电池电压电流
			else
				{
				LCD_ShowFloatNum1(41,40,VBat,2,CYAN,LGRAY,12);
				LCD_ShowChar(82,40,'V',WHITE,LGRAY,12,0);
				LCD_ShowFloatNum1(103,40,fabsf(IBat)>MinimumCurrentFactor?fabsf(IBat):0,2,YELLOW,LGRAY,12);
				LCD_ShowChar(144,40,'A',WHITE,LGRAY,12,0);				
				}
			break;
		//均衡运行中
		case AutoBalance_RunningBalance:
			LCD_ShowChinese(33,22,"自动均衡运行中",WHITE,LGRAY,0);
			//显示剩余时间
			if(!IsDispChargingINFO)
				{
				H=BalanceForceEnableTIM/8;
				M=(H%3600)/60;
				S=H%60;
				H/=3600;
				LCD_ShowHybridString(14,40,"剩余:",WHITE,LGRAY,0);
				LCD_ShowIntNum(51,40,H,1,GREEN,LGRAY,12);
				LCD_ShowChinese(62,40,"时",WHITE,LGRAY,12);
				LCD_ShowIntNum(78,40,M,2,GREEN,LGRAY,12);
				LCD_ShowChinese(98,40,"分",WHITE,LGRAY,12);
				LCD_ShowIntNum(114,40,S,2,GREEN,LGRAY,12);
				LCD_ShowChinese(134,40,"秒",WHITE,LGRAY,12);
				}
			else
				{
				LCD_ShowHybridString(14+11,40,"电池电压:",WHITE,LGRAY,0);
				LCD_ShowFloatNum1(68+11,40,VBat,2,CYAN,LGRAY,12);
				LCD_ShowChar(109+11,40,'V',WHITE,LGRAY,12,0);	
				}
		  break;
	  //检查电池包是否可以补电
		case AutoBalance_ReChargingWait:
			LCD_ShowChinese(10,22,"确认是否可以二次补电",WHITE,LGRAY,0);
		  LCD_ShowChinese(46,40,"请稍等……",WHITE,LGRAY,0);
		  break;
	  //正常结束
		case AutoBalance_End:
			LCD_ShowChinese(33,34,"自动均衡已结束",GREEN,LGRAY,0);
		  break;
		//异常结束
		case AutoBalance_End_Abnormal:
			LCD_ShowChinese(27,34,"自动均衡异常结束",RED,LGRAY,0);
		  break;		
		}
	LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
	LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
	LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
	//渲染完毕，复位
	IsUpdateBalUI=false;
	}	
	
//启动均衡
void EnableAutoBal(void)	
	{
	IsCPortBreaked=false;
	IsUpdateBalUI=true;
	AutoBalState=AutoBalance_WaitBattCharge; //等待充电
	BalanceForceEnableTIM=3600*8*5; //5个小时
	}

//菜单配置
const MenuConfigDef AutoBALMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
  &AutoBalMenuRenderHandler,
	&AutoBalFSMHandler,	
	//不是设置菜单不需要用别的事情
	"自动均衡修正\0",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&EnableAutoBal,
	&ResetChargerSystemToNormal
	};
