#include "IP2366_REG.h"
#include "lcd.h"
#include "GUI.h"
#include "Config.h"
#include "delay.h"
#include "ADC.h"
#include "BalanceMgmt.h"

//全局变量
static const char NonStdFwID[]={"YCNNH"};
static const char HSCPFwID[]={"YCEDS"};
bool IsBootFromVBUS;	
bool IsEnable17AMode=true;
bool IsEnableHSCPMode=false;
static char WaitAfterTypeCRemoved;	

void IP2366_PreInit(void)
	{
	char i=5;
	IP2366InputDef ICFG;
	IP2366OutConfigDef OCFG;
	ShowPostInfo(25,"充电IC初配置","09",Msg_Statu);
	//配置结构体
	ICFG.ChargeCurrent=9700;
	ICFG.ChargePower=Power_100W;
	ICFG.FullVoltage=4200;
	ICFG.PreChargeCurrent=200;
	ICFG.IsEnableCharger=false;
		
	OCFG.IsEnableDPDMOut=false;
  OCFG.IsEnableOutput=false;
  OCFG.IsEnablePDOut=false;
  OCFG.IsEnableSCPOut=false;		
	//监测是否存在
	do	
		{
		if(IP2366_DetectIfPresent())break;
		delay_ms(200);
		ShowPostInfo(25,"尝试连接至IC","0A",Msg_Warning);
		i--;
		}
	while(i>0);	
	//尝试了五次仍然失败，提示错误发送
	if(i==0)	
		{
		ShowPostInfo(25,"充电IC通信失败","E3",Msg_Fault);
		SelfTestErrorHandler();
		}
	IP2366_SetInputState(&ICFG);
	//监测Type-C状态
  if(!IP2366_GetIfInputConnected())IP2366_SetOutputState(&OCFG); //禁用输出		
	}
	
//系统过热保护处理	
bool IsSystemOverheating=false;	
static bool IP2366DCDCState=false;
	
void SysOverHeatProt(void)
	{
	bool ChgEN,DisEN,IsStepDown;
	extern bool IsEnableAdapterEmu;
	TypeCRoleDef Role;
	//NTC异常
	if(!ADCO.IsNTCOK)return;
	//触发过热
	if(ADCO.Systemp>(float)CfgData.OverHeatLockTemp)IsSystemOverheating=true;
	else if(ADCO.Systemp<(float)CfgData.OverHeatLockTemp-10)IsSystemOverheating=false;
	//同步DCDC状态
  if(IP2366DCDCState==IsSystemOverheating)return;
	//计算DCDC使能状态
	if(BalanceForceEnableTIM>0)	
		{
		//手动均衡运行中，关闭充放电
		ChgEN=false;
		DisEN=false;
		Role=TypeC_Disconnect;
		}
	else if(IsSystemOverheating)
		{
		//系统过热，关闭充放电
		ChgEN=false;
		DisEN=false;
		Role=TypeC_Disconnect;
		//判断过热保护是否满足条件
		if(!CfgData.EnableThermalStepdown)IsStepDown=false;
		else if(CfgData.InputConfig.ChargePower==Power_140W)IsStepDown=true;
		else if(CfgData.InputConfig.ChargePower==Power_100W)IsStepDown=true;	
		else IsStepDown=false;
		//触发过热保护强行把功率调回去
		if(IsStepDown)
			{
			//进行掉档
			if(CfgData.InputConfig.ChargePower==Power_140W)CfgData.InputConfig.ChargePower=Power_100W;
			else CfgData.InputConfig.ChargePower=Power_65W;
			WriteConfiguration(&CfgUnion,false);
			if(!IP2366_UpdataChargePower(CfgData.InputConfig.ChargePower))return;
			}
		}
	else if(IsEnableAdapterEmu)
		{
		//开启适配器模拟，开启放电关闭充电
		ChgEN=false;
		DisEN=true;
		Role=TypeC_UFP;
		}
	else
		{
		ChgEN=true;
		DisEN=CfgData.OutputConfig.IsEnableOutput;
		Role=CfgData.OutputConfig.IsEnableOutput?TypeC_DRP:TypeC_DFP;
		}		
	if(!IP2366_EnableDCDC(ChgEN,DisEN))return;
	if(!IP2366_SetTypeCRole(Role))return;
	IP2366DCDCState=IsSystemOverheating;
	}	
	
//根据系统配置重新初始化2366
void IP2366_ReInitBasedOnConfig(void)
	{
	IP2366InputDef ICFG;
	IP2366OutConfigDef OCFG;
	extern bool IsEnableAdapterEmu;
	extern bool OCState;
	//设置低电保护、PDO参数和输出参数
	OCFG.IsEnableDPDMOut=CfgData.OutputConfig.IsEnableDPDMOut;
	OCFG.IsEnablePDOut=CfgData.OutputConfig.IsEnablePDOut;
	OCFG.IsEnableSCPOut=CfgData.OutputConfig.IsEnableSCPOut;
	if(IsSystemOverheating)OCFG.IsEnableOutput=false;
	else if(IsEnableAdapterEmu)OCFG.IsEnableOutput=true;
	else OCFG.IsEnableOutput=CfgData.OutputConfig.IsEnableOutput;
			
	IP2366_SetVLowVolt(CfgData.Vlow);
	if(!IsBootFromVBUS&&!OCState)IP2366_SetOutputState(&OCFG);	//如果是在过充阶段或者是单独插着Type-C，那就不能设置输出，不然单片机直接断电了
	IP2366_SetPDOBroadCast(&CfgData.PDOCFG);
	//设置输入参数
	ICFG.ChargePower=CfgData.InputConfig.ChargePower;	
	ICFG.FullVoltage=CfgData.InputConfig.FullVoltage;
	ICFG.PreChargeCurrent=CfgData.InputConfig.PreChargeCurrent; //其他参数照常填写
	if(IsEnableAdapterEmu)ICFG.IsEnableCharger=false; //关闭充电器
	else ICFG.IsEnableCharger=IsSystemOverheating?false:true; //充电器如果过热触发则关闭
	ICFG.ChargeCurrent=CfgData.OutputConfig.IsEnableOutput?9700:CfgData.InputConfig.ChargeCurrent; //初始化时如果开启输出则填写9.7A					
	IP2366_SetInputState(&ICFG);
	//设置Type-C模式
	if(!IsBootFromVBUS&&!OCState) //如果是在过充阶段或者是单独插着Type-C，那就不能发送TCRST命令不然单片机直接断电了
		{
		if(!IsEnableAdapterEmu)
			{
			IP2366_SetTypeCRole(TypeC_Disconnect);
			delay_ms(200);
			IP2366_SetTypeCRole(CfgData.OutputConfig.IsEnableOutput?TypeC_DRP:TypeC_DFP);
			}
		else IP2366_SetTypeCRole(TypeC_UFP); //开启适配器模拟启用UFP模式
		delay_ms(100);
		}
	IP2366_SetOTPSign();
	IP2366_ClearOCFlag(); //移除OC Flag
	}	

//根据配置动态计算和设置2366的限流
void IP2366_SetIBatLIMBaseOnSysCfg(void)
	{
	int Current;
	//计算动态限流
	if(IP2366_GetIfInputConnected())Current=CfgData.InputConfig.ChargeCurrent; //充电模式已连接，设置为充电限流
	else Current=CfgData.InputConfig.ChargeCurrent>9700?CfgData.InputConfig.ChargeCurrent:9700; //为了确保放电正常运行，当充电未接入时设置为9.7A限流(如果电池限流比这个大那就设置为9700)
	//设置ICCMAX寄存器	
	IP2366_SetICCMax(Current); 
	}
	
//尝试恢复IP2366
void IP2366StallRestore(void);
extern short IPStallTime;	
	
void DetectIfIP2366Reset(void)
	{
	bool Reset;
	
	//获取当前芯片通信状态
	if(!IP2366_DetectIfChipReset(&Reset))IP2366StallRestore(); //开始计时
	//芯片发生复位，重置寄存器
	else if(Reset)IP2366_ReInitBasedOnConfig(); //重新初始化
	else IPStallTime=0; //未发生复位
	//对输出电流进行应用
	if(!CfgData.OutputConfig.IsEnableOutput)return; //输出被关闭不需要动态设置限流
  IP2366_SetIBatLIMBaseOnSysCfg(); //动态设置
	}

//如果存在电池，则在拔出TypeC之后的一段时间重新初始化
void IP2366_ReConfigOutWhenTypeCOFF(void)	
	{
	if(!IsBootFromVBUS)return;
	if(WaitAfterTypeCRemoved>0)WaitAfterTypeCRemoved--;
	else
		{
		IsBootFromVBUS=false;
		IP2366_SetOutputState(&CfgData.OutputConfig);
		IP2366_SetTypeCRole(CfgData.OutputConfig.IsEnableOutput?TypeC_DRP:TypeC_DFP);
		IP2366_SetOTPSign();
		}
	}

//后初始化
void IP2366_PostInit(void)
	{
	bool IsCmdSendOK,IsPoweredByVBUS;		
	extern bool EnableDetailOutput;
	IP2366InputDef ICFG;	
	int retry=5,i;
	char VendorString[5];
	ShowPostInfo(78,"充电IC后配置\0","10",Msg_Statu);
	//检查系统是否由VBUS供电
	for(i=0;i<15;i++)
		{
		ADC_GetResult();
		delay_ms(2);
		if(ADCO.Vbatt>(BattCellCount*2.6))break;
		}
	if(i<15)IsPoweredByVBUS=false;
	else  //监测到电池异常，触发保护	
    {		
		IsPoweredByVBUS=true;
		ShowPostInfo(78,"电池电压异常\0","EB",Msg_Warning);
		delay_Second(1);
		ShowPostInfo(78,"电池是否正确安装?","EB",Msg_Warning);
		delay_Second(1);
		}
	//检测固件版本
	ShowPostInfo(79,"读取芯片版本\0","11",Msg_Statu);
  if(!IP2366_GetFirmwareTimeStamp(VendorString))		
		{
		ShowPostInfo(79,"读取芯片版本失败\0","DE",Msg_Fault);
		SelfTestErrorHandler();
		}
	IsEnable17AMode=true;
	for(i=0;i<5;i++)if(NonStdFwID[i]!=VendorString[i])
		{
		//时间戳不匹配，禁止野兽模式
		IsEnable17AMode=false;	
		break;
		}
	if(IsEnable17AMode)
		{
		ShowPostInfo(79,"超充模式已启用\0","6F",Msg_Statu);
		if(CfgData.IStop==IStop_100mA||CfgData.IStop==IStop_150mA)
			{
			ShowPostInfo(78,"停充电流非法\0","D3",Msg_Warning);
			delay_Second(1);
			ShowPostInfo(78,"已自动修正\0","D3",Msg_Warning);
			delay_Second(1);
			CfgData.IStop=IStop_200mA;
			if(!WriteConfiguration(&CfgUnion,true))
				{
				ShowPostInfo(30,"存储器写入异常\0","E6",Msg_Fault);
				SelfTestErrorHandler();
				}
			}
		if(EnableDetailOutput)delay_ms(300);
		}	
	if((CfgData.MaxVPD==PDMaxIN_28V||CfgData.InputConfig.ChargeCurrent>9700)&&!IsEnable17AMode)
		{
		ShowPostInfo(78,"芯片为公版固件\0","D0",Msg_Warning);
		delay_Second(1);
		ShowPostInfo(78,"超充模式已禁用\0","D0",Msg_Warning);
		delay_Second(1);
		//将峰值限流调回去
		CfgData.MaxVPD=PDMaxIN_20V;
		CfgData.InputConfig.ChargeCurrent=9700;
		if(!WriteConfiguration(&CfgUnion,true))
			{
			ShowPostInfo(80,"存储器写入异常\0","E6",Msg_Fault);
			SelfTestErrorHandler();
			}		
		}
	//检查高压SCP支持
	IsEnableHSCPMode=true;
	for(i=0;i<5;i++)if(VendorString[i]!=HSCPFwID[i])
		{
		//时间戳不匹配，关闭高功率SCP支持
		IsEnableHSCPMode=false;
		break;
		}		
	if(!IsEnableHSCPMode&&CfgData.OutputConfig.IsEnableHSCPOut)
		{
		ShowPostInfo(78,"芯片为公版固件\0","D2",Msg_Warning);
		delay_Second(1);
		ShowPostInfo(78,"高功率SCP已禁用\0","D2",Msg_Warning);
		delay_Second(1);
		//关闭高功率SCP功能
		CfgData.OutputConfig.IsEnableHSCPOut=false;
		if(!WriteConfiguration(&CfgUnion,true))
			{
			ShowPostInfo(80,"存储器写入异常\0","E6",Msg_Fault);
			SelfTestErrorHandler();
			}		
		}
	//设置再充电参数
	ShowPostInfo(80,"设置再充电参数\0","12",Msg_Statu);	
	if(!IP2366_SetReChargeParam(CfgData.VRecharge,CfgData.IStop))
		{
		ShowPostInfo(80,"再充参数设置失败\0","EC",Msg_Fault);
		SelfTestErrorHandler();
		}
	//设置低压保护参数
	ShowPostInfo(82,"设置低压保护电压\0","13",Msg_Statu);	
	if(!IP2366_SetVLowVolt(CfgData.Vlow))	
		{
		ShowPostInfo(82,"低压保护设置失败\0","E8",Msg_Fault);
		SelfTestErrorHandler();
		}
	//开始写寄存器
	if(!IsPoweredByVBUS)
		{
		ShowPostInfo(85,"设置放电系统\0","14",Msg_Statu);	
		if(!IP2366_SetOutputState(&CfgData.OutputConfig))
			{
			ShowPostInfo(85,"放电系统设置失败\0","E6",Msg_Fault);
			SelfTestErrorHandler();
			}
		}
	//设置输入
	ShowPostInfo(88,"设置充电系统\0","15",Msg_Statu);
	ICFG.ChargePower=CfgData.InputConfig.ChargePower;	
	ICFG.FullVoltage=CfgData.InputConfig.FullVoltage;
	ICFG.PreChargeCurrent=CfgData.InputConfig.PreChargeCurrent; //其他参数照常填写
	ICFG.IsEnableCharger=true; //充电器始终开启
	ICFG.ChargeCurrent=CfgData.OutputConfig.IsEnableOutput?9700:CfgData.InputConfig.ChargeCurrent; //初始化时如果开启输出则填写9.7A	
  //设置PDO输入
	if(!IP2366_SetInputState(&ICFG))
		{
		ShowPostInfo(88,"充电系统设置失败\0","E7",Msg_Fault);
		SelfTestErrorHandler();
		}	
	//设置PDO参数
	ShowPostInfo(92,"设置PDO广播\0","16",Msg_Statu);
	if(!IP2366_SetPDOBroadCast(&CfgData.PDOCFG))
		{
		ShowPostInfo(92,"PDO广播设置失败\0","EA",Msg_Fault);
		SelfTestErrorHandler();		
		}
	//发送重新握手的指令
	ShowPostInfo(94,"发送TypeC重连命令\0","17",Msg_Statu);
  if(!IsPoweredByVBUS)do
		{
		IsCmdSendOK=IP2366_SetTypeCRole(TypeC_Disconnect);
		delay_ms(200);
		IsCmdSendOK&=IP2366_SetTypeCRole(CfgData.OutputConfig.IsEnableOutput?TypeC_DRP:TypeC_DFP);
		delay_ms(200);
		IsCmdSendOK&=IP2366_SetOTPSign();
		if(IsCmdSendOK)break;
		retry--;
		ShowPostInfo(94,"重连命令重试中\0","16",Msg_Warning);
		}
	while(retry>0);
	//否则只执行设置OTP
	else do
		{
		IsCmdSendOK=IP2366_SetOTPSign();
		if(IsCmdSendOK)break;
		retry--;
		delay_ms(10);
		ShowPostInfo(94,"重连命令重试中\0","18",Msg_Warning);
		}
	while(retry>0);
	//尝试失败
	if(!retry)
		{
		ShowPostInfo(94,"重连命令发送失败\0","D3",Msg_Fault);
		SelfTestErrorHandler();
		}
	IP2366_ClearOCFlag(); //移除OC Flag
	//后处理完毕，此时如果是从VBUS启动的，则显示标记
	if(!IsPoweredByVBUS)return;
	IsBootFromVBUS=true;
	WaitAfterTypeCRemoved=50;
	}
