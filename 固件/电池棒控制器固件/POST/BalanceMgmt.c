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

//内部参数定义
#define BalanceEnableVolt 9.95  //设置均衡模块开始正常运行的电池电压
#define CVBalanceOFFTimeSec 10     //配置均衡系统在CV模式下关闭主动均衡的时间(单位秒)

//全局变量
static unsigned char BalanceOFFWaitTIM;  //检测均衡电流低于一定值，停止均衡运行的计时器
int BalanceForceEnableTIM=0; //强制启用均衡系统的变量，往该变量写大于0的值启用均衡器
bool EnableExtendedBal=false;  //当未经均衡的电池充放电数目达到足够多的时候启动均衡
bool BalanceState=false;
int ChargeFullTIM=0; //充电满充计时器

//内部全局变量
static int WaitChargingBeginTIM;
static bool BalTypeCConnectedState=false; //均衡连接状态
	
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
	//用户禁止自动均衡的状态下，停止该函数响应禁止自动均衡
  if(!CfgData.EnableExtendedBalance)return;
	//系统开始充电，倒计时
	IP2366_GetChargerState(&SysState);	
  if(SysState==Batt_StandBy||SysState==Batt_discharging)WaitChargingBeginTIM=80;
	else if(WaitChargingBeginTIM>0)WaitChargingBeginTIM--;		
	
	//电池电压还没到或者是倒计时未结束，不允许执行检测流程
	if(ADCO.Vbatt<BalanceEnableVolt||WaitChargingBeginTIM)return; 	
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

//内部函数，获取均衡配置（用于在自动补电均衡时强制开启均衡系统）
static BalanceModeDef QueryBalanceMode(void)
	{
	extern AutoBalanFSMDef AutoBalState;
	//当前系统处于自动均衡的补电阶段，根据充电阶段打开均衡
	if(AutoBalState==AutoBalance_ReCharging)return Balance_ChgOnly;
	//其余情况，返回系统配置的结果
	return CfgData.BalanceMode;
	}
	
//恒压充电模式下控制均衡的控制模块
static bool CVModeBalMgmt(void)
	{
	//均衡计时器数值计算
	#define CVBalanceOFFTime CVBalanceOFFTimeSec*8
	#if (CVBalanceOFFTime > 0xFF)
		#error "Invalid Balance Module Off Time Configuration!"
	#endif
	//均衡处于激活状态时执行关闭均衡的判断
	if(BalanceState)
		 {
		 //当前关闭计时器已经达到限制，返回false禁用均衡模块
		 if(!BalanceOFFWaitTIM)return false;
		 //电流小于0.6,开始倒计时
		 else if(fabsf(ADCO.Ibatt)<0.6)BalanceOFFWaitTIM--;
		 //电流大于=0.6，阻止倒计时并反向增加计时时间
		 else if(BalanceOFFWaitTIM<CVBalanceOFFTime)BalanceOFFWaitTIM++; 
		 }
	//均衡器处于关闭状态，执行开启均衡的判断	 
  else
		{
		//均衡器处于关闭状态且当前计时已经达到重新开启均衡的参数值，立即开启
		if(BalanceOFFWaitTIM>=CVBalanceOFFTime)return true;
		//电流大于1.5，开始正向计时
		else if(ADCO.Ibatt>1.5)BalanceOFFWaitTIM++;
		//电流不够1.5A，反向倒计时
		else if(BalanceOFFWaitTIM>0)BalanceOFFWaitTIM--;
		}

	//其余情况则衡保持当前状态，避免反复搁楞
	return BalanceState;
	}
	
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
	//初始化本地计时变量
	BalanceOFFWaitTIM=CVBalanceOFFTime;
	WaitChargingBeginTIM=80; //开始充电等待10秒才进入
	//应用校准数据
	ShowPostInfo(53,"应用ADC校准数据","13",Msg_Statu);
	InternalADC_LoadCalibration(CfgData.BatteryVoltageCalFactor,CfgData.BatteryCurrentCalFactor,CfgData.SystemTempCalFactor);
	//禁止掉关闭时间的声明
	#undef CVBalanceOFFTime
	}	
	
//运行过程中控制均衡器启用的模块
void Balance_IOMgmt(void)
	{
	bool IsBalanceEnable; 
	extern short SleepTimer;
	
	BatteryStateDef SysState=Batt_StandBy;
	//运行增强自动均衡的判断
	Balance_ExtendBalMgmt();
	//读取电池状态
	IP2366_GetChargerState(&SysState); 
	//电池电压过低,此时均衡开了也没用，因为栅极驱动IC处于UVLO状态所以保持关闭
	if(ADCO.Vbatt<BalanceEnableVolt)IsBalanceEnable=false;
	//强制启用均衡的计时器激活，进行递减		
	else if(BalanceForceEnableTIM>0)
		{
		BalanceForceEnableTIM--;
		IsBalanceEnable=true;
		}		
	//系统在待机状态且即将进入睡眠，关闭均衡	
	else if(SysState==Batt_StandBy&&SleepTimer<8)IsBalanceEnable=false;
	//根据配置状态进行启用
	else switch(QueryBalanceMode())
		{
		case Balance_Diasbled:IsBalanceEnable=false;break; //永久关闭主动均衡
		case Balance_AlwaysEnabled:   //始终启用（除特定情况）
		case Balance_ChgOnly: 				//仅充电时启用
		  switch(SysState)
				{
				case Batt_PreChage:
				case Batt_CCCharge:IsBalanceEnable=true;break; //处于正常充电状态时，启用均衡系统
				case Batt_CVCharge:IsBalanceEnable=CVModeBalMgmt();break;	//恒压模式下，根据系统电流执行判断		   
				case Batt_ChgDone:IsBalanceEnable=false;break; //电池充满，禁止均衡运行
				default:
					  if(QueryBalanceMode()==Balance_AlwaysEnabled)
							//当前系统被设置为始终使能均衡，在其余状态下均衡始终开启
							IsBalanceEnable=true;
						else
							//当前系统被设置为仅充电时开启均衡，其余状态下关闭均衡系统
							IsBalanceEnable=false; 								 
				}
			break;
		case Balance_ChgDisOnly: //仅充放电时启用
			 
  		//系统处于充电完毕或者充电等待时，禁止均衡运行
		  if(SysState==Batt_ChgWait||SysState==Batt_ChgDone)IsBalanceEnable=false; 
		  //系统当前处于恒压充电阶段，根据电流参数控制均衡
		  else if(SysState==Batt_CVCharge)IsBalanceEnable=CVModeBalMgmt();
		  //其余的正常充放电状态，系统启动均衡器
		  else if(SysState!=Batt_StandBy)IsBalanceEnable=true; 
		  //否则均衡器保持关闭
		  else IsBalanceEnable=false;
		  break;
		}
	//设置IO状态
	if(BalanceState==IsBalanceEnable)return;
	if(PCA9536_SetIOState(PCA9536_IOPIN_0,IsBalanceEnable))BalanceState=IsBalanceEnable; //设置均衡状态
	}
