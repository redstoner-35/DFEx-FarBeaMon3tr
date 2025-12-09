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

//内部全局变量
static int WaitChargingBeginTIM;
static bool BalTypeCConnectedState=false; //均衡连接状态

//配置均衡控制器
void BalanceMgmt_Init(void)
	{
	bool State;
	char i,buf;
	ShowPostInfo(52,"均衡控制器配置","12",Msg_Statu);
	buf=0x01;
	for(i=0;i<8;i++)
		{
		//按顺序循环配置所有的GPIO
		State=PCA9536_SetIOState((SMIOPinDef)buf,false); //将对应IO设置为0
		State&=PCA9536_SetIOPolarity((SMIOPinDef)buf,PCA9536_IO_Normal); //正常极性
		State&=PCA9536_SetIODirection((SMIOPinDef)buf,PCA9536_IODIR_OUT); //输出模式
		if((SMIOPinDef)buf==PCA9536_IOPIN_3&&GetIfPCAIsOldOne())break;    //非新的IO则跳过额外IO的初始化
		buf<<=1;
		}		
	//检查设置状态
	if(!State)
		{
		ShowPostInfo(52,"均衡控制器异常","ED",Msg_Fault);
		SelfTestErrorHandler();
		}
	//应用校准数据
	WaitChargingBeginTIM=80; //开始充电等待10秒才进入
	ShowPostInfo(53,"应用ADC校准数据","13",Msg_Statu);
	InternalADC_LoadCalibration(CfgData.BatteryVoltageCalFactor,CfgData.BatteryCurrentCalFactor,CfgData.SystemTempCalFactor);
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
	BatteryStateDef SysState;
	extern bool EnableManuBal;
	extern bool IsUserForceDisableAutoCharge;
	//在均衡未完全关闭且用户禁止自动均衡的状态下，停止该函数响应禁止自动均衡
  if(CfgData.BalanceMode!=Balance_Diasbled&&!CfgData.EnableExtendedBalance)
		{
		//关闭自动均衡时自动清除未均衡容量避免溢出
		if(LogData.UnbalanceBatteryAh>20.0)LogData.UnbalanceBatteryAh=0;
		return;			
		}			
	//系统开始充电，倒计时
	IP2366_GetChargerState(&SysState);	
  if(SysState==Batt_StandBy||SysState==Batt_discharging)WaitChargingBeginTIM=80;
	else if(WaitChargingBeginTIM>0)WaitChargingBeginTIM--;		
	
	//电池电压还没到或者是倒计时未结束，不允许执行检测流程
	if(ADCO.Vbatt<=10.1||WaitChargingBeginTIM)return; 	
	//检测TypeC状态
	State=IP2366_GetIfInputConnected();
	if(BalTypeCConnectedState==State)return;
	BalTypeCConnectedState=State;
	//均衡可能需要启动，检测状态
	if(!BalTypeCConnectedState||BalanceForceEnableTIM>0)return;                //检测输入状态
	if(CfgData.BalanceMode==Balance_Diasbled)BalValue=11.50;	
	else BalValue=16.50; //根据均衡器模式选定需要自动均衡的时间	
	
	if(IsUserForceDisableAutoCharge||LogData.UnbalanceBatteryAh<BalValue)return; //循环次数还没到或者是用户在本次开机强制退出均衡
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
	extern short SleepTimer;
	extern AutoBalanFSMDef AutoBalState;
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
	//当前系统处于自动均衡的补电阶段，需要强制打开均衡	
	else if(AutoBalState==AutoBalance_ReCharging)IsBalanceEnable=true;
	//系统在待机状态且即将进入睡眠，关闭均衡	
	else if(SysState==Batt_StandBy&&SleepTimer<8)IsBalanceEnable=false;
	//根据配置状态进行启用
	else switch(CfgData.BalanceMode)
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
	//设置IO状态
	if(BalanceState==IsBalanceEnable)return;
	if(PCA9536_SetIOState(PCA9536_IOPIN_0,IsBalanceEnable))BalanceState=IsBalanceEnable; //设置均衡状态
	}
