#include "LCD_Init.h"
#include "Config.h"
#include "ht32.h"
#include "GUI.h"
#include "PCA9536.h"
#include "IP2366_REG.h"
#include "LogSystem.h"
#include "BalanceMgmt.h"
#include "ADC.h"
#include <math.h>

//全局变量
int BalanceForceEnableTIM=0; //强制启用均衡系统的变量，往该变量写大于0的值启用均衡器
bool EnableExtendedBal=false;  //当未经均衡的电池充放电数目达到足够多的时候启动均衡
bool BalanceState=false;
int BalanceDisableTIM=0; //暂时关闭均衡的计时器
int ChargeFullTIM=0; //充电满充计时器

//内部全局
static bool BalTypeCConnectedState; //均衡连接状态

//配置均衡控制器
void BalanceMgmt_Init(void)
	{
	bool State;
	ShowPostInfo(52,"均衡控制器配置","30",Msg_Statu);
	State=PCA9536_SetIOState(PCA9536_IOPIN_0,false); //将对应IO设置为0
	State&=PCA9536_SetIOPolarity(PCA9536_IOPIN_0,PCA9536_IO_Normal); //正常极性
	State&=PCA9536_SetIODirection(PCA9536_IOPIN_0,PCA9536_IODIR_OUT); //输出模式
	//检查设置状态
	if(!State)
		{
		ShowPostInfo(52,"均衡控制器异常","3E",Msg_Fault);
		SelfTestErrorHandler();
		}
	}
	
//强制关闭均衡
void Balance_ForceDiasble(void)
	{
	//将对应IO设置为0
	PCA9536_SetIOState(PCA9536_IOPIN_0,false); 
	}

//检测是否启用加强均衡的模块
static void Balance_ExtendBalMgmt(void)
	{
	bool State;
	float BalValue;
	extern bool EnableManuBal;
	//检测TypeC状态
	State=IP2366_GetIfInputConnected();
	if(BalTypeCConnectedState==State)return;
	BalTypeCConnectedState=State;
	//均衡可能需要启动，检测状态
	if(State||!EnableManuBal)return;
	if(CfgData.BalanceMode==Balance_Diasbled)BalValue=11.50;	
	else BalValue=16.50; //根据均衡器模式选定需要自动均衡的时间	
	if(LogData.UnbalanceBatteryAh<BalValue)return; //循环次数还没到
	SwitchingMenu(&AutoBALMenu);	//进入自动均衡
	}	

//在检测到系统长时间处于恒压充电状态无法转绿的时候暂时关闭均衡
static void Balance_ChargeDertect(void)
	{
	BatteryStateDef SysState=Batt_StandBy;
	IP2366_GetChargerState(&SysState); //读取状态
	//检测电池状态，当电池卡在充满状态且为恒压状态时，计时器动作
	if(BalanceDisableTIM>0&&SysState==Batt_ChgDone)BalanceDisableTIM=0; //当电池成功转入充满之后立即重新激活均衡器
	if(BalanceDisableTIM>0)ChargeFullTIM=0;
	else if(fabsf(ADCO.Ibatt)<0.2&&SysState==Batt_CVCharge)ChargeFullTIM++;
	else if(ChargeFullTIM>0)ChargeFullTIM--;
	//计时器时间到3分钟，尝试关闭均衡让芯片可以正常判定充满
	if(ChargeFullTIM>=(8*60*2))
		{
		ChargeFullTIM=0;
		BalanceDisableTIM=(8*60*3); //强制关闭均衡2分钟，让芯片可以判定充满
		}
	}	
	
//运行过程中控制均衡器启用的模块
void Balance_IOMgmt(void)
	{
	bool IsBalanceEnable; 
	extern int SleepTimer;
	BatteryStateDef SysState=Batt_StandBy;
	//运行增强自动均衡以及临时禁用均衡允许芯片充满的判断
	Balance_ChargeDertect();
	Balance_ExtendBalMgmt();
	//读取电池状态
	IP2366_GetChargerState(&SysState); 
	//电池电压过低或者即将进入休眠，禁用均衡
	if(ADCO.Vbatt<10.1)IsBalanceEnable=false;
	//强制启用均衡的计时器激活，进行递减		
	else if(BalanceForceEnableTIM>0)
		{
		BalanceForceEnableTIM--;
		IsBalanceEnable=true;
		}		
	//强制关闭均衡计时器动作，禁用均衡
	else if(BalanceDisableTIM>0)
		{
		BalanceDisableTIM--;
		IsBalanceEnable=false;
		}
	//根据配置状态进行启用
	else if(SysState!=Batt_StandBy||SleepTimer>8)switch(CfgData.BalanceMode)
		{
		case Balance_Diasbled:IsBalanceEnable=false;break; //永久关闭主动均衡
		case Balance_ChgOnly: //仅充电时启用
		  switch(SysState)
				{
				case Batt_PreChage:
				case Batt_CCCharge:
				case Batt_CVCharge:
				case Batt_ChgDone:IsBalanceEnable=true;break; //处于正常充电状态时，启用均衡系统
				default:IsBalanceEnable=false; //否则关闭均衡系统
				}
			break;
		case Balance_ChgDisOnly: //仅充放电时启用
		  if(SysState==Batt_ChgWait)IsBalanceEnable=false;
		  else if(SysState!=Batt_StandBy)IsBalanceEnable=true; //处于正常充放电状态时，启用均衡系统
		  else IsBalanceEnable=false;
		  break;
		case Balance_AlwaysEnabled:IsBalanceEnable=true;break; //均衡永远开启
		}
	//系统即将进入睡眠，关闭均衡
	else IsBalanceEnable=false;
	//设置IO状态
	if(BalanceState==IsBalanceEnable)return;
	if(PCA9536_SetIOState(PCA9536_IOPIN_0,IsBalanceEnable))BalanceState=IsBalanceEnable; //设置均衡状态
	}
