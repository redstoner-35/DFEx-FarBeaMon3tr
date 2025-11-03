#include "PCA9536.h"
#include "IP2366_REG.h"
#include "ADC.h"
#include "delay.h"
#include "LogSystem.h"
#include "AUXPSU.h"
#include <math.h>
#include "WatchDog.h"
#include "GUI.h"

//内部变量状态
static AUXPSUStateDef PSUState=AUXPSU_Passthrough;
static short AUXPSUTIM=80;	
bool IsCPortTriggerOK=false; //标志位，C Port触发电路是否OK
static bool IsAdvancedPMOK=false; //标志位，高级电源控制模块是否OK

//内部define
#define EnablePassthrough() PCA9536_SetIOState(PCA9536_IOPIN_2,false)
#define DisablePassthrough() PCA9536_SetIOState(PCA9536_IOPIN_2,true)	
#define EnableAUXBuck() PCA9536_SetIOState(PCA9536_IOPIN_3,true)	
#define DisableAUXBuck() PCA9536_SetIOState(PCA9536_IOPIN_3,false)	

//强制启用安全模式（进入安全模式用）
void ForceEnableAdvPM(void)
	{
	IsAdvancedPMOK=true;
	}

//给磁保持继电器发送指令让CC线切换到IP2366
bool AUXPSU_ConnectTCtoIP2366(void)
	{
	bool State;
	//硬件不受支持返回true
	if(!IsAdvancedPMOK)return true;
	//打开磁保持继电器驱动电源
	if(!PCA9536_SetIOState(PCA9536_IOPIN_7,true))return false;
	delay_ms(10);
	//令SET=0，Reset给高100mS，继电器变为复位状态
	State=PCA9536_SetIOState(PCA9536_IOPIN_5,false);
	delay_ms(10);
	State&=PCA9536_SetIOState(PCA9536_IOPIN_6,true);
	#ifndef EnableDebugMode
	WatchDog_Feed(); //喂狗
	#endif
	delay_ms(100);
	State&=PCA9536_SetIOState(PCA9536_IOPIN_6,false); //RESET生成100mS脉冲，继电器复位
	//操作完毕，关闭磁保持继电器驱动电源
	State&=PCA9536_SetIOState(PCA9536_IOPIN_7,false); 
	#ifndef EnableDebugMode
	WatchDog_Feed(); //喂狗
	#endif
	//返回操作结果
	return State;
	}
	
//给磁保持继电器发送指令让CC线切换到下拉强制取电
bool AUXPSU_ConnectTCtoIPD(void)
	{
	bool State;
	//硬件不受支持返回true
	if(!IsAdvancedPMOK)return true;
	//打开磁保持继电器驱动电源
	if(!PCA9536_SetIOState(PCA9536_IOPIN_7,true))return false;
	delay_ms(10);
	//令RESET=0，SET给高100mS，继电器变为置位状态
	State=PCA9536_SetIOState(PCA9536_IOPIN_6,false);
	delay_ms(10);
	State&=PCA9536_SetIOState(PCA9536_IOPIN_5,true);
	#ifndef EnableDebugMode
	WatchDog_Feed(); //喂狗
	#endif
	delay_ms(100);
	State&=PCA9536_SetIOState(PCA9536_IOPIN_5,false); //SET生成100mS脉冲，继电器置位
	//操作完毕，关闭磁保持继电器驱动电源
	State&=PCA9536_SetIOState(PCA9536_IOPIN_7,false); 
	#ifndef EnableDebugMode
	WatchDog_Feed(); //喂狗
	#endif
	//返回操作结果
	return State;
	}
	
//设置强制下拉C口强迫适配器输出的取电电阻的状态
bool AUXPSU_SetIPDState(bool State)
	{
	//硬件不受支持返回true
	if(!IsAdvancedPMOK)return true;
	//设置对应的IO
	return PCA9536_SetIOState(PCA9536_IOPIN_4,State?false:true);
	}

//设置强制Type-C对外输出的5.1K CC电阻诱骗电路是否激活
bool AUXPSU_SetTypeCFVoutState(bool State)
	{
	//硬件不受支持返回true
	if(!IsCPortTriggerOK)return true;
	//设置对应的IO
	return PCA9536_SetIOState(PCA9536_IOPIN_1,State);
	}

//检测C口控制的5.1K硬件是否正常
void AUXPSU_DetectIfCPortTriggerPresent(void)
	{
	bool state,SelfTestState;
	extern bool EnableDetailOutput;
	IsCPortTriggerOK=false;
	IsAdvancedPMOK=false;
	//开始检测流程
	ShowPostInfo(99,"初始化ATCM模块","45",Msg_Statu);
	SelfTestState=PCA9536_SetIODirection(PCA9536_IOPIN_1,PCA9536_IODIR_IN);
	delay_ms(10); //释放IO口为输入等待10mS
	SelfTestState&=PCA9536_ReadInputState(PCA9536_IOPIN_1,&state);
	IsCPortTriggerOK=!state?true:false;  //这里检测的原理是PCA9536对于不用的IO有弱上拉，如果外部存在C口相关电路，则弱上拉会被下拉电阻强制拉到0V,这时候就可以判断了
	SelfTestState&=PCA9536_SetIODirection(PCA9536_IOPIN_1,PCA9536_IODIR_OUT);
	SelfTestState&=AUXPSU_SetTypeCFVoutState(false);
	//检测自检结果
	if(!SelfTestState)
		{
		ShowPostInfo(99,"ATCM模块异常","FC",Msg_Fault);
		SelfTestErrorHandler();
		}
	if(EnableDetailOutput&&!IsCPortTriggerOK)
		{
		ShowPostInfo(99,"检测到旧版硬件","46",Msg_INFO);
		delay_ms(200);
		ShowPostInfo(99,"ATCM模块已禁用","46",Msg_INFO);	
		delay_ms(200);			
		}
  //开始电源管理控制检测流程
	ShowPostInfo(99,"初始化APMM模块","47",Msg_Statu);
	SelfTestState&=PCA9536_SetIODirection(PCA9536_IOPIN_2,PCA9536_IODIR_IN);
	SelfTestState&=PCA9536_SetIODirection(PCA9536_IOPIN_3,PCA9536_IODIR_IN);	
	delay_ms(10); //释放IO口为输入等待10mS
  SelfTestState&=PCA9536_ReadInputState(PCA9536_IOPIN_2,&state); //读取直通电路的IO状态
	IsAdvancedPMOK=!state?true:false;
	SelfTestState&=PCA9536_ReadInputState(PCA9536_IOPIN_3,&state); //读取辅助BUCK电路的IO状态
	if(state)IsAdvancedPMOK=false; //辅助BUCK部分电路空焊，标记为失败
	SelfTestState&=PCA9536_SetIODirection(PCA9536_IOPIN_2,PCA9536_IODIR_OUT);
	SelfTestState&=PCA9536_SetIODirection(PCA9536_IOPIN_3,PCA9536_IODIR_OUT); //调回去输出
	PSUState=AUXPSU_Passthrough;
  SelfTestState&=DisableAUXBuck();
	SelfTestState&=EnablePassthrough();  //默认系统处于直通运行状态
	//检测自检结果
	if(!SelfTestState)
		{
		ShowPostInfo(99,"APMM模块异常","FD",Msg_Statu);
		SelfTestErrorHandler();
		}	
	if(EnableDetailOutput&&!IsAdvancedPMOK)
		{
		ShowPostInfo(99,"检测到旧版硬件","48",Msg_INFO);
		delay_ms(200);
		ShowPostInfo(99,"APMM模块已禁用","48",Msg_INFO);	
		delay_ms(200);	
		}
	//支持高级电源管理且电池电压足够的时候，启动时发指令同步继电器状态
	if(!IsAdvancedPMOK||ADCO.Vbatt<(2.5*BattCellCount))return;
	if(IsBootFromVBUS)AUXPSU_ConnectTCtoIPD();
	else AUXPSU_ConnectTCtoIP2366();
	}	
	
//准备进入待机状态时，关闭buck并且切换到直通通道
void AUXPSU_SwitchToPassThrough(void)
	{
	//具备C口诱骗器的硬件，首先关闭buck和对外强制输出的5.1K电阻
	if(!IsCPortTriggerOK)PCA9536_SetIODirection(PCA9536_IOPIN_1,PCA9536_IODIR_IN);
	else AUXPSU_SetTypeCFVoutState(false);
  if(!IsAdvancedPMOK)	
		{
		//对于不支持Advanced Power Mgmt的旧版硬件，把所有没用的IO设置为input tri-state
		PCA9536_SetIODirection(PCA9536_IOPIN_2,PCA9536_IODIR_IN);
		PCA9536_SetIODirection(PCA9536_IOPIN_3,PCA9536_IODIR_IN);  
		}		
	else
	 { 
	 //支持高级电源管理，关闭buck并打开直通模块，延迟20mS后在打开直通模块否则buck会炸掉
	 DisableAUXBuck();
	 delay_ms(20);
	 EnablePassthrough();
	 }
	}
	
//系统正常工作阶段切换
void AUXPSU_Mgmt(void)
	{
	float VBUS;
	BatteryStateDef State;
	//系统运行在旧硬件上，不支持新的电源管理控制，跳过该函数	
	if(!IsAdvancedPMOK)return;
	//获取VBUS电压
	VBUS=0;
	State=Batt_StandBy;
	IP2366_GetVBUSVoltage(&VBUS);
	IP2366_GetChargerState(&State);
	//状态机处理
	switch(PSUState)
		{
		//BUCK降压模式（此时屏幕由USB C口输入的电压通过buck降压下来供电，减少发热）
		case AUXPSU_Buck:
			 //由直通模式进入BUCK模式，延时一段时间确保BUCK端已经完全放电再打开buck
			 if(AUXPSUTIM>0)
					{
					AUXPSUTIM--;
					if(!AUXPSUTIM)EnableAUXBuck();
					}
			 //进行状态转移
			if(VBUS<4.0||State==Batt_discharging||State==Batt_StandBy)		
					{
					//输入电压小于4V或者是放电/待机模式，关闭直通和buck由电池为系统供电
					DisableAUXBuck();
					DisablePassthrough();
					PSUState=AUXPSU_ALLOFF;
					}
			 else if(VBUS<13.40)
					{
					//C口输入电压小于13.40V，此时buck无法维持工作，立即切换到直通模式
					DisableAUXBuck();
					delay_ms(5);
					EnablePassthrough();
					AUXPSUTIM=16;                  //开启pass through之后有两秒钟冷却
					PSUState=AUXPSU_Passthrough;
					}
			 break;	
		//直通模式（屏幕的供电由TYPE-C输入的电源直接进入LDO进行降压，发热极大）
		case AUXPSU_Passthrough:
			 //电池电量异常或者系统仍然处于安全模式，禁止状态转移
			 if(ADCO.Vbatt<2.5*BattCellCount||IsBootFromVBUS)AUXPSUTIM=80;
		   else if(AUXPSUTIM>0)AUXPSUTIM--;
		   //电池存在电压之后状态转移允许启动
		   if(AUXPSUTIM>0)break;
		   if(VBUS<(ADCO.Vbatt+0.1)||State==Batt_discharging||State==Batt_StandBy)
				 {
				 //输入电压小于4V或者是放电/待机模式，关闭直通和buck由电池为系统供电
			   DisableAUXBuck();
				 DisablePassthrough();
				 PSUState=AUXPSU_ALLOFF;
				 }
		   else if(VBUS>13.90)
				{
				//如果系统处于充电模式，且输入电压大于13V，则切换到buck模式
				AUXPSUTIM=4;
				DisablePassthrough();
				PSUState=AUXPSU_Buck;
				}
		   break;
	  //辅助电源关闭模式，此时屏幕完全由系统供电
		case AUXPSU_ALLOFF:
			 //系统处于放电或者待机模式，保持关闭状态
			 if(State==Batt_discharging||State==Batt_StandBy)break;
			 //VBUS电压大于13.9，立即启用BUCK模块由BUCK给屏幕供电
		   if(VBUS>13.90)
				{	
				EnableAUXBuck();
				PSUState=AUXPSU_Buck;
				}
			 //VBUS电压在电池电压+0.1到13.9之间，启用直通模块
		   else if(VBUS>(ADCO.Vbatt+0.1))
				{
				EnablePassthrough();
				AUXPSUTIM=16;                  //开启pass through之后有两秒钟冷却
				PSUState=AUXPSU_Passthrough;
				}
		   break;
		
		}
	}
