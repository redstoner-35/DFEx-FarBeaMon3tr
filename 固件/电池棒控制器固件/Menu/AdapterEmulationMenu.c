#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"
#include "ADC.h"
#include "Key.h"
#include "delay.h"
#include "AUXPSU.h"
#include <math.h>

typedef enum
	{
	Error_CommFault,
  Error_VbatTooLow,
  Error_SysNotInDischarge,
	Error_SysFaultAsserted,
	Error_EnableVOUTTimeOut
	}AdapEmuErrorDef;

typedef enum
	{
	AdapEmu_Initial,
	AdapEmu_WaitForOutput,
	AdapEmu_TryToUseFakeLoad,
	AdapEmu_StartTypeC,
	AdapEmu_InitFailed,
	AdapEmu_WaitTCOFF,
	AdapEmu_ConfigTypeCMode,
	AdapEmu_Running,
	AdapEmu_StopDueToFault,
	AdapEmu_StopDueToLowBatt
	}AdapEmuFSMDef;	

//外部变量
extern bool Is2366Telem;
extern IP2366VBUSStateDef VBUS;
extern BatteryStateDef BATT;
extern float VTypec,ITypeC,VBat,IBat;
extern bool IsResultUpdated;
extern bool IsSystemOverheating;
extern ChipStatDef CState;
	
//内部变量
static bool IsErrorShown=false; //显示错误了没
bool IsEnableAdapterEmu=false; //是否开启适配器模拟
static AdapEmuErrorDef EmuErrorName;
static AdapEmuFSMDef EmuState=AdapEmu_Initial;	
static short EmuFSMTIM=0;		
static short EmuFaultTIM=0;
static short EmuFaultCounter=0;
static short EmuAPortTimeOutTIM=0; //A口超时计时器
bool IsEnabledFakeAToCMode=false; //标志位，是否开启假的A to C输出功能	
	
//状态机计时器
void AdapEmuTIMHandler(void)
	{
	if(EmuFaultTIM>0)EmuFaultTIM--;
	if(EmuFSMTIM>0)EmuFSMTIM--;
	if(!EmuFaultTIM&&EmuAPortTimeOutTIM>0)EmuAPortTimeOutTIM--;
	}
	
//内部函数，检查电池是否过低
static bool CheckIfBattTooLow(void)
	{
	float VMin;
	//存储模式下使用判断
	if(StorageMode!=StorageMode_OFF)
		{
		VMin=3.5+((float)StorageMode*0.1f);
		VMin*=BattCellCount;
		}
	//非存储模式使用系统电压	
	else
		{
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
		}
	if(ADCO.Vbatt<VMin)return true;
  //正常进入
	return false;
	}	
	
//进入适配器模拟
void EnterAdapterEmulationPrePare(void)
	{
	BatteryStateDef BattState;
	//获取芯片当前状态	
	IsErrorShown=false;
	IsEnabledFakeAToCMode=false;
	if(CState.VSysState!=VSys_State_Normal||CState.VBusState==VBUS_OverVolt)
		{
		EmuState=AdapEmu_InitFailed;
		EmuErrorName=Error_SysFaultAsserted;
		}
	//检查电池电压是否过低
	else if(CheckIfBattTooLow())
		{
		EmuState=AdapEmu_InitFailed;
		EmuErrorName=Error_VbatTooLow;
		}
	//尝试对芯片通信
	else if(!IP2366_GetChargerState(&BattState))EmuState=AdapEmu_InitFailed;
	else if(!IP2366_EnableDCDC(false,true))EmuState=AdapEmu_InitFailed; //关闭充电系统
	else if(BattState==Batt_StandBy&&IsCPortTriggerOK)EmuState=AdapEmu_TryToUseFakeLoad; //如果C口假负载存在，则进行假负载连接的尝试
	else if(BattState!=Batt_discharging)EmuState=AdapEmu_WaitForOutput; //非放电模式跳转到等待负载连接
	//通信成功，如果系统已经是放电则直接把typec模式配置为Source进入模拟
	else EmuState=AdapEmu_ConfigTypeCMode;
	}

//处理退出适配器模拟的函数
bool CalcIfDCDCOutEnabled(void);
	
void ExitAdapterEmulation(void)
	{
	int i;
	extern bool CurrentStorDisState;
	extern bool IsEnableTempChargeOnly;
	extern bool DCDCOutputBit;
	//关闭充电器并断开C口连接
  IP2366_EnableDCDC(false,false);
	IP2366_SetTypeCRole(TypeC_NoConnect);
	//等待500毫秒
  for(i=0;i<5;i++)delay_ms(100);
  //重新打开充电器		
	if(IsEnableTempChargeOnly)IP2366_EnableDCDC(true,false); //仅充电模式开启，禁用充电器
	else if(StorageMode!=StorageMode_OFF)IP2366_EnableDCDC(true,CurrentStorDisState);  //存储模式开启，根据存储模式状态配置充电器
	else IP2366_EnableDCDC(true,DCDCOutputBit);	
  //重新配置C口角色
	IP2366_SetTypeCRole(CalcIfDCDCOutEnabled()?TypeC_Bidir:TypeC_SinkOnly);
	IP2366_ClearOCFlag(); //清除OC警报
	IsEnableAdapterEmu=false;
	}
	
//处理适配器模拟进入失败的函数
static void AdapterInitFaultHandler(void)
	{
	Is2366Telem=true;
	if(IsErrorShown)return;
	RenderMenuBG();
	LCD_ShowChinese(14,22,"适配器模拟开启失败！",RED,LGRAY,0);
	switch(EmuErrorName)
		{
		case Error_VbatTooLow:LCD_ShowChinese(32,37,"电池电压过低",YELLOW,LGRAY,0);break;
		case Error_EnableVOUTTimeOut:LCD_ShowChinese(32,37,"放电输出开启失败",YELLOW,LGRAY,0);break;
		case Error_CommFault:LCD_ShowChinese(32,37,"芯片通信错误",YELLOW,LGRAY,0);break;
		case Error_SysNotInDischarge:LCD_ShowChinese(17,37,"系统处于非正常模式",YELLOW,LGRAY,0);break;
		case Error_SysFaultAsserted:LCD_ShowChinese(32,37,"系统出现故障",YELLOW,LGRAY,0);break;
		}		
	ShowPressExitToLeave();
	//显示结束，标记结果已更新
	IsErrorShown=true;
	}	
//适配器模拟进入时初始化状态机的处理	
void AdapterEmuEnter(void)
	{
	EmuFaultTIM=0;
	EmuFaultCounter=0;
	EmuErrorName=Error_CommFault;
	EmuState=AdapEmu_Initial;	
	}
	
//适配器模拟运行过程中的渲染
static void AdapterEmuRunningHandler(void)
	{
	extern bool IsDispChargingINFO;
	u16 Color;
	int Temp;
	float Power;		
	//显示标题
	if(IsSystemOverheating)LCD_ShowChinese(22,23,"系统过热，模拟暂停",ORANGE,LGRAY,0);
	else 
		{
	  if(IsCPortTriggerOK&&IsDispChargingINFO)
			{
			if(!VBUS.IsTypeCConnected&&!IsEnabledFakeAToCMode) //C口未连接且未开启模拟则显示按下Enter
				LCD_ShowHybridString(13,23,"按下Enter开启A口模拟",WHITE,LGRAY,0);
			else if(IsEnabledFakeAToCMode)
				LCD_ShowHybridString(22,23,"USB-A口模拟已开启",GREEN,LGRAY,0);
			else //其余情况照常显示适配器模拟已开启 
				LCD_ShowChinese(25,23,"适配器模拟已开启",GREEN,LGRAY,0);
			}
	  else LCD_ShowChinese(25,23,"适配器模拟已开启",GREEN,LGRAY,0);
		}
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
				else if(Temp<10)Color=BLUE;
				else if(Temp<CfgData.OverHeatLockTemp-20)Color=GREEN;
				else if(Temp<CfgData.OverHeatLockTemp-8)Color=YELLOW;
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
	//屏幕GUI已更新
	IsResultUpdated=false;	
	}		
	
//执行适配器模拟的主渲染处理
void AdapterEmuRender(void)
	{
  bool InitState;
	//显示处理
  switch(EmuState)		
		{
		//初始化处理
		case AdapEmu_Initial:
			RenderMenuBG();
			LCD_ShowChinese(14,22,"适配器模拟开启中……",WHITE,LGRAY,0);
			EnterAdapterEmulationPrePare();
		  IsResultUpdated=true;
		  break;
	  case AdapEmu_TryToUseFakeLoad:
			RenderMenuBG();
			LCD_ShowChinese(14,22,"适配器模拟开启中……",WHITE,LGRAY,0);
			EmuFSMTIM=32;
		  //打开C口5.1K诱骗电路
			InitState=AUXPSU_SetIPDState(false);
		  delay_ms(10);
		  InitState&=AUXPSU_ConnectTCtoIPD();
		  InitState&=AUXPSU_SetTypeCFVoutState(true); //首先关闭C口强制取电的下拉，10mS后令磁保持继电器将2366的CC和C口断开，接着开启2366的取电下拉
		  if(!InitState)
				{
			  EmuErrorName=Error_CommFault;
			  EmuState=AdapEmu_InitFailed; //C口诱骗电路异常，无法成功答案开
				}
			//打开成功，跳转到输出等待阶段
			else EmuState=AdapEmu_WaitForOutput;
			IsResultUpdated=true;
			break;
    //等待输出建立				
	  case AdapEmu_WaitForOutput:
			Is2366Telem=true;
			if(!IsResultUpdated)break;
			RenderMenuBG();
			LCD_ShowChinese(14,22,"适配器模拟开启中……",WHITE,LGRAY,0);
		  if(!EmuFSMTIM)
				{
				if(IsCPortTriggerOK)
					{
					EmuErrorName=Error_EnableVOUTTimeOut; //开启输出电压超时，报错
					EmuState=AdapEmu_InitFailed;             
					IsResultUpdated=false;
					break;
					}
				else LCD_ShowHybridString(17,37,"请将负载连接到USB",YELLOW,LGRAY,0);
				}
		  else if(EmuFSMTIM<2)
				{
				//计时1秒后仍然没有成功切换到输出状态，说明是旧版硬件需要手动拿OTG骗一下，显示提示
				EmuFSMTIM=0;
				AUXPSU_SetTypeCFVoutState(false); //关闭C口对外诱骗电路
				delay_ms(5);
				AUXPSU_ConnectTCtoIP2366(); 
				AUXPSU_SetIPDState(true);    //延时5mS后将C口的CC线和IP2366重新连接，恢复C口
				}		
		  //屏幕已刷新，标记结束模拟
			IsResultUpdated=false;
			//等待系统进入放电模式
			if(BATT==Batt_StandBy)break;
		  else if(BATT==Batt_discharging)EmuState=AdapEmu_StartTypeC; //系统进入放电模式，连接到负载
		  else 
				{
			  EmuErrorName=Error_SysNotInDischarge;
			  EmuState=AdapEmu_InitFailed; //系统进入其他模式，失败
				}
			break;
		//开始失败
		case AdapEmu_InitFailed:
			AdapterInitFaultHandler(); //初始化失败处理
		  break;
		//配置TypeC
		case AdapEmu_StartTypeC:
			InitState=true;
			if(!IP2366_SetTypeCRole(TypeC_NoConnect))InitState=false;
			if(!IP2366_EnableDCDC(false,false))InitState=false;        //诱骗输出启动成功后，关闭C口并切换到关闭模式
		  //开启失败则报错
		  if(!InitState)
				{
				EmuErrorName=Error_CommFault;
				EmuState=AdapEmu_InitFailed;
				}
			//开启成功，进入运行模式   
		  else
				{	
				EmuState=AdapEmu_WaitTCOFF;
				if(!IsCPortTriggerOK)break;	//如果系统不支持高级C口诱骗电路，则跳过诱骗关闭处理 
				AUXPSU_SetTypeCFVoutState(false); //关闭C口对外诱骗电路
				delay_ms(5); 
				AUXPSU_ConnectTCtoIP2366(); 
				AUXPSU_SetIPDState(true);    //延时5mS后将C口的CC线和IP2366重新连接，恢复C口			
				}
			break;
		//适配器模拟等待输出关闭 
    case AdapEmu_WaitTCOFF:
			Is2366Telem=true;
			if(BATT!=Batt_StandBy)break; //等待电池放电结束
		  EmuState=AdapEmu_ConfigTypeCMode;
		  break;
		//配置type-C为SourceMode
		case AdapEmu_ConfigTypeCMode:		
			InitState=IP2366_SetTypeCRole(TypeC_SourceOnly);
		  InitState&=IP2366_EnableDCDC(false,true); 			//重新打开对外放电系统并配置为Source Only的仅放电输出
		  if(!InitState)
				{
				EmuErrorName=Error_CommFault;
				EmuState=AdapEmu_InitFailed;
				}
			else EmuState=AdapEmu_Running;	
			break;
		//适配器模拟运行中，正常显示
		case AdapEmu_Running:
			Is2366Telem=true;
			//A口模拟超时处理
		  if(IsEnabledFakeAToCMode)
				{
				if(!VBUS.IsTypeCConnected)
					{
					if(!EmuFSMTIM)EmuFSMTIM=16;   //反复尝试打开关闭
					if(EmuFSMTIM==16)AUXPSU_SetTypeCFVoutState(false);
					else if(EmuFSMTIM==10)AUXPSU_SetTypeCFVoutState(true); //反复尝试关闭重启C口诱骗器直到恢复
					}
				//在系统没有运行故障重置且输出电流大于0.1A时，进行计时
				if((fabsf(ITypeC)>0.1&&VBUS.IsTypeCConnected)||EmuFaultTIM)EmuAPortTimeOutTIM=80;
				else if(!EmuAPortTimeOutTIM)
					{
					//A to C未链接设备足够长的时间，关闭模拟
					IsEnabledFakeAToCMode=false;
					AUXPSU_SetTypeCFVoutState(false); //关闭C口对外诱骗电路
					delay_ms(5);
					AUXPSU_ConnectTCtoIP2366(); 
					AUXPSU_SetIPDState(true);    //延时5mS后将C口的CC线和IP2366重新连接，恢复C口
					}
				}
			//进行故障判断
		  if(CState.VSysState!=VSys_State_Normal||CState.VBusState==VBUS_OverVolt)
				{
				if(EmuFaultTIM>8)EmuFaultTIM=0; //在故障flag消除的时候计数器被+1，无视错误
				if(!EmuFaultTIM)
					{
					if(EmuFaultCounter<3)
					{
					IP2366_ClearOCFlag();
					EmuFaultCounter++;
					EmuFaultTIM=8;
					}
					//系统超过三次故障，强制结束模拟
					else EmuState=AdapEmu_StopDueToFault; //系统故障，跳转到模拟结束阶段
					}
				}
			//系统正常运行，消除警报
			else if(!EmuFaultTIM&&EmuFaultCounter)
				{
				EmuFaultTIM=16;
				if(EmuFaultCounter>0)EmuFaultCounter--;
				}

		  if(CheckIfBattTooLow())EmuState=AdapEmu_StopDueToLowBatt; //电池电量过低，模拟结束
		  //系统出现故障进入停止模拟状态，退出模拟
		  if(EmuState!=AdapEmu_Running)ExitAdapterEmulation();
		  //进行UI渲染
			if(!IsResultUpdated)break;
			RenderMenuBG();
			AdapterEmuRunningHandler();
			break;
		//由于系统故障，模拟停止
		case AdapEmu_StopDueToLowBatt:
		case AdapEmu_StopDueToFault:
			Is2366Telem=true;
		  if(!IsResultUpdated)break;
		  RenderMenuBG();
		  if(EmuState==AdapEmu_StopDueToLowBatt)LCD_ShowChinese(10,22,"电池电量过低，模拟停止",RED,LGRAY,0);
		  else LCD_ShowChinese(10,22,"系统出现故障，模拟停止",RED,LGRAY,0);
			ShowPressExitToLeave();
	    //模拟异常停止后，复位CC口继电器
			if(IsCPortTriggerOK&&IsEnabledFakeAToCMode)
				{
				AUXPSU_SetTypeCFVoutState(false); //关闭C口对外诱骗电路
				delay_ms(5);
				AUXPSU_ConnectTCtoIP2366(); 
				AUXPSU_SetIPDState(true);    //延时5mS后将C口的CC线和IP2366重新连接，恢复C口
				IsEnabledFakeAToCMode=false;
				}
		  //屏幕已刷新
			IsResultUpdated=false;
			break;
		}
	}

void AdapterMenuKeyProc(void)
	{
	extern bool IsEnterDischargeMode;
	bool State;
	//C口诱骗硬件存在的情况下，按下Enter开启A to C输出模式
	if(IsCPortTriggerOK&&KeyState.KeyEvent==KeyEvent_Enter&&EmuState==AdapEmu_Running)
		{
		//尝试激活A to C模拟
		if(!IsEnabledFakeAToCMode&&!VBUS.IsTypeCConnected)
			{
			State=AUXPSU_SetIPDState(false);
		  delay_ms(10);
		  State&=AUXPSU_ConnectTCtoIPD();
		  State&=AUXPSU_SetTypeCFVoutState(true); //首先关闭C口强制取电的下拉，10mS后令磁保持继电器将2366的CC和C口断开，接着开启2366的取电下拉
			IsEnabledFakeAToCMode=State;
			if(State)EmuAPortTimeOutTIM=160; //成功开启A to C模拟，160秒后再进行检测
			}
		//尝试关闭A to C模拟		
		else if(IsEnabledFakeAToCMode)
			{
			State=AUXPSU_SetTypeCFVoutState(false); //关闭C口对外诱骗电路
			delay_ms(5);
			State&=AUXPSU_ConnectTCtoIP2366(); 
			State&=AUXPSU_SetIPDState(true);    //延时5mS后将C口的CC线和IP2366重新连接，恢复C口	
			if(State)IsEnabledFakeAToCMode=false;
			}
		}
  //按下esc返回菜单		
	if(KeyState.KeyEvent==KeyEvent_ESC)
		{
		//退出时如果非运行模式，或者是开启了A to C诱骗则需要强制关闭C口对外诱骗电路
		if(IsEnabledFakeAToCMode||EmuState!=AdapEmu_Running)
			{
			AUXPSU_SetTypeCFVoutState(false); //关闭C口对外诱骗电路
			delay_ms(5);
			AUXPSU_ConnectTCtoIP2366(); 
			AUXPSU_SetIPDState(true);    //延时5mS后将C口的CC线和IP2366重新连接，恢复C口
			}
		//从快捷菜单进来的，直接回主菜单
		if(IsEnterDischargeMode)
			{
			IsEnterDischargeMode=false;
			IsEnableAdvancedMode=false;
			ClearScreen(); //清屏
			SwitchingMenu(&MainMenu);
			}
		else if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
		else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
		//退出之前需要reset index
		ClearMenuIndex();
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
	&AdapterEmuEnter, //进入时配置为UFP
	&ExitAdapterEmulation //退出模拟
	};
