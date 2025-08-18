#include "IP2366_REG.h"
#include "lcd.h"
#include "GUI.h"
#include "Config.h"
#include "delay.h"
#include "ADC.h"
#include "BalanceMgmt.h"
#include <string.h>
#include <math.h>
#include "AUXPSU.h"
#include "LogSystem.h"

bool IsEnableDischargeAtStor=true;
static char WaitAfterTypeCRemoved;	
bool CurrentStorDisState=true;
bool IsSystemOverheating=false;
static bool IP2366DCDCState=false;

//外部函数
void Force2366ToRestart(void); //强制2366断电重启

//计算存储模式开启时，是否打开放电模块
bool IP2366_IsEnableDischarge(float VBatRaw)
	{
	float VBatt,VbattThres;
	//计算电池电压
  VBatt=VBatRaw/BattCellCount;
	VBatt*=1000; //转mV
	//计算触发阈值
	VbattThres=3500+((float)StorageMode*100);
	//进行施密特处理
  if(VBatt>VbattThres+100)IsEnableDischargeAtStor=true;
	else if(VBatt<=VbattThres)IsEnableDischargeAtStor=false;
	//返回结果
	return IsEnableDischargeAtStor;
	}	

void IP2366_PreInit(void)
	{
	char i=5;
	char VendorString[6];
	char FWIDReport[32];
	IP2366InputDef ICFG;
	IP2366OutConfigDef OCFG;
	IP2366HWRevDef HWID;
	ShowPostInfo(25,"充电IC初配置","07",Msg_Statu);
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
		ShowPostInfo(25,"尝试连接至IC","W3",Msg_Warning);
		i--;
		}
	while(i>0);	
	//尝试了五次仍然失败，提示错误发送
	if(i==0)	
		{
		ShowPostInfo(25,"充电IC通信失败","E5",Msg_Fault);
		SelfTestErrorHandler();
		}
	
	//监测Type-C状态
  if(!IP2366_GetIfInputConnected())
		{
		//输入未连接，禁止输入输出
		IP2366_SetOutputState(&OCFG); //禁用输出		
		IP2366_SetInputState(&ICFG,true);
		}
	//禁止充电器工作
	else IP2366_DisableCharger();	
		
	//读取芯片并且进行比对，比对的同时更新芯片capability set
	ShowPostInfo(25,"充电IC版本检查","08",Msg_Statu);
	memset(VendorString,0,sizeof(VendorString));    
	if(!IP2366_GetFirmwareTimeStamp(VendorString))		
		{
		ShowPostInfo(79,"读取固件版本失败\0","E6",Msg_Fault);
		SelfTestErrorHandler();
		}		
	if(!IP2366_UpdateChipCap(VendorString))
		{
		memset(FWIDReport,0,sizeof(FWIDReport));
		snprintf(FWIDReport,sizeof(FWIDReport),"固件ID:%s",VendorString);
		ShowPostInfo(79,FWIDReport,"E7",Msg_Fault);
		delay_Second(1);
		ShowPostInfo(79,"不受支持的固件版本\0","E7",Msg_Fault);
		SelfTestErrorHandler();	
		}
	//如果芯片支持读取硬件版本，则读取芯片的硬件版本并进行比对
	if(!CurrentIP2366FW->ExtendedROREGCapable)return;
	HWID=IP2366_GetChipHWRev();
	if(HWID!=Hardware_Ver_D)
		{
		if(HWID==Hardware_Ver_Unknown)ShowPostInfo(79,"读取芯片版本失败\0","E6",Msg_Fault);
		else ShowPostInfo(79,"不受支持的硬件版本\0","E7",Msg_Fault); 
		SelfTestErrorHandler();
		}
	}
	
//设置系统的充放电使能和typec角色
bool SetSystemDischargeState(void)
	{
	bool ChgEN,DisEN,IsStepDown;
	extern bool IsEnableAdapterEmu;
	extern bool IsEnableTempChargeOnly;
	extern AutoBalanFSMDef AutoBalState;
	extern bool IsCPortBreaked;
	TypeCRoleDef Role;		
	//计算DCDC使能状态
	if(BalanceForceEnableTIM>0||AutoBalState==AutoBalance_RunningBalance)	
		{
		//手动均衡运行中，关闭充放电
		ChgEN=false;
		DisEN=false;
		if(IsCPortBreaked)Role=TypeC_DFP; //C口被打断之后禁止充电，强制断开source
		else Role=TypeC_Disconnect;
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
			if(!IP2366_UpdataChargePower(CfgData.InputConfig.ChargePower))return false;
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
		if(IsEnableTempChargeOnly)DisEN=false; //临时打开仅充电功能
		else if(StorageMode!=StorageMode_OFF)DisEN=IP2366_IsEnableDischarge(ADCO.Vbatt);  //存储模式开启，使用当前电池电压进行配置是否使能放电
		else DisEN=DCDCOutputBit;
		Role=TypeC_DRP; //存储模式关闭且启用输出设置为DRP
		}		
	if(!IP2366_EnableDCDC(ChgEN,DisEN))return false;
	if(!IP2366_SetTypeCRole(Role))return false;
	//设置成功
	return true;
	}	
	
//存储模式使能放电管理的处理
static void StorageModeDischargeControl(void)
	{
	bool DischargeState;
	extern bool IsEnableAdapterEmu;
	//存储模式关闭，或者不执行检测
	if(StorageMode==StorageMode_OFF)return;
	//开启存储模式，进行检测
	DischargeState=IP2366_IsEnableDischarge(ADCO.Vbatt);
	if(IsEnableAdapterEmu)	
		{
		//适配器模拟模式下，不允许执行下面的重新初始化动作
		CurrentStorDisState=DischargeState;
		return;
		}
	if(CurrentStorDisState==DischargeState)return;
  if(SetSystemDischargeState())CurrentStorDisState=DischargeState;
	}	

//系统过热保护处理	
void SysOverHeatProt(void)
	{
  //执行存储模式管理和自循环管理
	StorageModeDischargeControl();
	//NTC异常
	if(!ADCO.IsNTCOK)return;
	//检测是否过热	
	if(ADCO.Systemp>(float)CfgData.OverHeatLockTemp)IsSystemOverheating=true;
	else if(ADCO.Systemp<(float)CfgData.OverHeatLockTemp-10)IsSystemOverheating=false;			
	//同步DCDC状态
  if(IP2366DCDCState==IsSystemOverheating)return;
	if(SetSystemDischargeState())IP2366DCDCState=IsSystemOverheating;
	}	
	
//进行IP2366的输出初始化	
static bool IP2366_InitOutputSystem(bool StorageInput)
	{
	IP2366OutConfigDef OCFG;
	extern bool IsEnableTempChargeOnly;
	//设置协议
	OCFG.IsEnableDPDMOut=CfgData.OutputConfig.IsEnableDPDMOut;
	OCFG.IsEnableHSCPOut=CfgData.OutputConfig.IsEnableHSCPOut;
	OCFG.IsEnablePDOut=CfgData.OutputConfig.IsEnablePDOut;
	OCFG.IsEnableSCPOut=CfgData.OutputConfig.IsEnableSCPOut;
	if(IsEnableTempChargeOnly)OCFG.IsEnableOutput=false;
	else if(StorageMode!=StorageMode_OFF)
			OCFG.IsEnableOutput=StorageInput; //调用输出配置函数检测电池电压状态
	else 
			OCFG.IsEnableOutput=DCDCOutputBit;
	//写寄存器设置输出模块	
	return IP2366_SetOutputState(&OCFG);
	}	
	
//根据系统配置重新初始化2366
void IP2366_ReInitBasedOnConfig(void)
	{
	IP2366InputDef ICFG;
	IP2366OutConfigDef OCFG;
	extern bool IsEnableAdapterEmu;
	extern bool OCState;
	extern bool IsEnableTempChargeOnly;
	//设置低电保护、PDO参数和输出参数
	OCFG.IsEnableDPDMOut=CfgData.OutputConfig.IsEnableDPDMOut;
	OCFG.IsEnablePDOut=CfgData.OutputConfig.IsEnablePDOut;
	OCFG.IsEnableSCPOut=CfgData.OutputConfig.IsEnableSCPOut;
	if(IsSystemOverheating||IsEnableTempChargeOnly)OCFG.IsEnableOutput=false;
	else if(IsEnableAdapterEmu)OCFG.IsEnableOutput=true;
	else if(StorageMode!=StorageMode_OFF)OCFG.IsEnableOutput=CurrentStorDisState;  //存储模式开启，使用当前电池电压进行配置
	else OCFG.IsEnableOutput=DCDCOutputBit;
			
	IP2366_SetVLowVolt(CfgData.Vlow);
	if(!IsBootFromVBUS&&!OCState)IP2366_SetOutputState(&OCFG);	//如果是在过充阶段或者是单独插着Type-C，那就不能设置输出，不然单片机直接断电了
	IP2366_SetPDOBroadCast(&CfgData.PDOCFG);
	//设置输入参数
	switch(StorageMode)
		{
		//根据存储模式配置输出
		case StorageMode_OFF:ICFG.FullVoltage=CfgData.InputConfig.FullVoltage;break; //存储模式禁用,使用系统设置的最高电压
		case StorageMode_3V6:ICFG.FullVoltage=3600;break; //存储模式启用，电池最大电压3.6V
		case StorageMode_3V7:ICFG.FullVoltage=3700;break; //存储模式启用，电池最大电压3.7V
	  case StorageMode_3V8:ICFG.FullVoltage=3800;break;	//存储模式启用，电池最大电压3.8V
		}		
	ICFG.ChargePower=CfgData.InputConfig.ChargePower;	
	ICFG.PreChargeCurrent=CfgData.InputConfig.PreChargeCurrent; //其他参数照常填写
	if(IsEnableAdapterEmu)ICFG.IsEnableCharger=false; //关闭充电器
	else ICFG.IsEnableCharger=IsSystemOverheating?false:true; //充电器如果过热触发则关闭
	//填写ICCMAX
	ICFG.ChargeCurrent=OCFG.IsEnableOutput?CurrentIP2366FW->IP2366ICCMAX:CfgData.InputConfig.ChargeCurrent; //初始化时如果开启输出则按照固件能力填写				
	IP2366_SetInputState(&ICFG,IsBootFromVBUS?false:true);
	//设置Type-C模式
	if(!IsBootFromVBUS&&!OCState) //如果是在过充阶段或者是单独插着Type-C，那就不能发送TCRST命令不然单片机直接断电了
		{
		if(!IsEnableAdapterEmu)
			{
			IP2366_SetTypeCRole(TypeC_Disconnect);
			delay_ms(200);	
			IP2366_SetTypeCRole(TypeC_DRP);
			}
		else IP2366_SetTypeCRole(TypeC_UFP); //开启适配器模拟启用UFP模式
		delay_ms(100);
		}
	//设置PPS和fixed PDO参数
	IP2366_SetDeepSleepModeEnabled(CfgData.SleepCfg==System_Sleep_Deep?true:false);
	IP2366_SetFixedPDO(&CfgData.FixedPDOCfg);
	IP2366_SetPPSCurrent(&CfgData.PPSConfig);
  //设置OCP和清除OC Flag		
	IP2366_SetOTPSign();
	IP2366_ClearOCFlag(); //移除OC Flag
	}	

//根据配置动态计算和设置2366的限流
void IP2366_SetIBatLIMBaseOnSysCfg(void)
	{
	int Current;
	//计算动态限流
	if(IP2366_GetIfInputConnected())Current=CfgData.InputConfig.ChargeCurrent; //充电模式已连接，设置为充电限流
	else Current=CfgData.InputConfig.ChargeCurrent>CurrentIP2366FW->IP2366ICCMAX?CfgData.InputConfig.ChargeCurrent:CurrentIP2366FW->IP2366ICCMAX; //为了确保放电正常运行，当充电未接入时设置为9.7A限流(如果电池限流比这个大那就设置为9700)
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
	if(!DCDCOutputBit)return; //输出被关闭不需要动态设置限流
  IP2366_SetIBatLIMBaseOnSysCfg(); //动态设置
	}

//如果存在电池，则在拔出TypeC之后的一段时间重新初始化
void IP2366_ReConfigOutWhenTypeCOFF(void)	
	{
	extern short VBUSReConnectTimeCounter;
	if(!IsBootFromVBUS)return;
	//电池电压不足或者当前电池电流过低，复位等待计时器
  if(ADCO.Vbatt<9.6||ADCO.Ibatt<0.6)WaitAfterTypeCRemoved=50;
	//电池电压正常	
	else if(WaitAfterTypeCRemoved>0)WaitAfterTypeCRemoved--;
	else
		{
		VBUSReConnectTimeCounter=48; 	//系统即将从安全模式切换到正常充电，置位消隐定时器避免容量测试误认为C口被拔出
		IsBootFromVBUS=false;
		AUXPSU_SetIPDState(true);
		AUXPSU_ConnectTCtoIP2366();		//充电达到足够时长之后切换回IP2366进行高功率充电
		IP2366_ReInitBasedOnConfig(); //重新初始化2366
		IP2366_SetTypeCRole(TypeC_DRP);
		}
	}
//在继电器切换CC pin之后等待芯片重新连接
static void IP2366_WaitChipInitAfterChangeCC(void)	
	{
	int i=40;
	do
		{
		if(IP2366_DetectIfPresent())return;
		delay_ms(100);
		}
	while(--i);
	//等待超时处理
	ShowPostInfo(25,"充电IC通信失败","E5",Msg_Fault);
	SelfTestErrorHandler();
	}

//后初始化
void IP2366_PostInit(void)
	{
	bool IsCmdSendOK;		
	extern bool EnableDetailOutput;
	bool Result;
	IP2366VBUSStateDef VBUSState;
	IP2366InputDef ICFG;	
	float VdroopRate;
	int retry=5,i;
	char VendorString[5];
	ShowPostInfo(78,"充电IC后配置\0","28",Msg_Statu);
	//检查系统是否由VBUS供电
	i=0;
  do
		{
		delay_ms(100);
		ADC_GetResult();
		VdroopRate=PORVBatVolt-ADCO.Vbatt; //计算电池电压变化
    if(PORVBatVolt<(2.50*BattCellCount))
			{
			i=10;
			break; //电池初始化时电压异常，报错
			}
		else if(VdroopRate<0.05)break;
		i++;
		}
	while(i<10);	
  if(i==10)//监测到电池异常，触发保护	
    {		
		ShowPostInfo(78,"电池电压异常\0","W8",Msg_Warning);
		delay_Second(1);
		ShowPostInfo(78,"电池是否正确安装?","W8",Msg_Warning);
		delay_Second(1);
		if(!IsBootFromVBUS)
			{
			RunLogEntry.LastDataCRC=CalcRunLogCRC32(&RunLogEntry.Data); //强制刷新旧日志的结果
			IsBootFromVBUS=true;
			RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data); //计算运行日志的CRC32
			ShowPostInfo(78,"系统将进入安全模式\0","W8",Msg_Warning);
			WriteRuntimeLogToROM(); //保存日志				
			//电池电量异常，此时系统进入Fail-Safe Boot模式，强制C口取电为系统供电	
			ForceEnableAdvPM();
			AUXPSU_SetIPDState(true);
			AUXPSU_ConnectTCtoIPD();
			delay_Second(1);
			}
		}
	else if(IsBootFromVBUS&&ADCO.Vbatt>(3.50*BattCellCount))
		{
		ShowPostInfo(78,"电池电压已恢复\0","49",Msg_INFO);
		delay_Second(1);
		ShowPostInfo(78,"系统将退出安全模式\0","49",Msg_INFO);
		RunLogEntry.LastDataCRC=CalcRunLogCRC32(&RunLogEntry.Data); //强制刷新旧日志的结果
		IsBootFromVBUS=false;	
		RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data); //计算运行日志的CRC32
		WriteRuntimeLogToROM(); //保存日志		
		ForceEnableAdvPM();
		AUXPSU_ConnectTCtoIP2366();	//令磁保持继电器将CC线连接到IP2366
		//等待芯片重新协商
		IP2366_WaitChipInitAfterChangeCC();
		}
	//设置芯片睡眠功能
	if(!IP2366_SetDeepSleepModeEnabled(CfgData.SleepCfg==System_Sleep_Deep?true:false))
		{
		ShowPostInfo(78,"设置系统模式失败\0","F1",Msg_Fault);
		SelfTestErrorHandler();
		}
	else if(CfgData.SleepCfg!=System_Sleep_Deep)
		{
		ShowPostInfo(78,"即插即用模式已开启\0","29",Msg_INFO);
		delay_ms(400);
		ShowPostInfo(78,"待机功耗将增大\0","29",Msg_INFO);
		delay_ms(400);
		}
	//再次检测固件版本
	ShowPostInfo(79,"二次读取芯片版本\0","2A",Msg_Statu);
  if(!IP2366_GetFirmwareTimeStamp(VendorString))		
		{
		ShowPostInfo(79,"读取芯片版本失败\0","E6",Msg_Fault);
		SelfTestErrorHandler();
		}		
	if(strncmp(CurrentIP2366FW->FWID,VendorString,5))	
		{
		ShowPostInfo(79,"芯片版本数据错误\0","FB",Msg_Fault);
		SelfTestErrorHandler();		
		}
	//判断是否支持超充以及是否开启超充
	if(CfgData.MaxVPD==PDMaxIN_28V)Result=true;
	else if(CfgData.InputConfig.ChargeCurrent>CurrentIP2366FW->IP2366ICCMAX)Result=true;
	else if(CfgData.InputConfig.ChargePower>CurrentIP2366FW->MaxCapableChgPower)Result=true;
	else Result=false;
	if(CurrentIP2366FW->IsHyperChargeCapable)
		{
		ShowPostInfo(79,"超充模式已启用\0","2B",Msg_Statu);
		if(CfgData.IStop==IStop_100mA||CfgData.IStop==IStop_150mA)
			{
			ShowPostInfo(78,"停充电流非法\0","W9",Msg_Warning);
			delay_Second(1);
			ShowPostInfo(78,"已自动修正\0","W9",Msg_Warning);
			delay_Second(1);
			CfgData.IStop=IStop_200mA;
			if(!WriteConfiguration(&CfgUnion,true))
				{
				ShowPostInfo(30,"存储器写入异常\0","E9",Msg_Fault);
				SelfTestErrorHandler();
				}
			}
		if(EnableDetailOutput)delay_ms(300);
		}
   else if(Result)
		{
		ShowPostInfo(78,"芯片为公版固件\0","2C",Msg_Warning);
		delay_Second(1);
		ShowPostInfo(78,"超充模式已禁用\0","2C",Msg_Warning);
		delay_Second(1);
		//将峰值限流调回去
		CfgData.MaxVPD=PDMaxIN_20V;
		CfgData.InputConfig.ChargeCurrent=CurrentIP2366FW->IP2366ICCMAX;
		CfgData.InputConfig.ChargePower=CurrentIP2366FW->MaxCapableChgPower;
		if(!WriteConfiguration(&CfgUnion,true))
			{
			ShowPostInfo(80,"存储器写入异常\0","E9",Msg_Fault);
			SelfTestErrorHandler();
			}		
		}
	//检查50mA per LSB的PDO支持
	if(!CurrentIP2366FW->IsExtendPDOCapable&&CfgData.FixedPDOCfg.PDO20VICCMAX>4000)
		{
		ShowPostInfo(78,"芯片为公版固件\0","2D",Msg_Warning);
		delay_Second(1);
		ShowPostInfo(78,"20V高功率已禁用\0","2D",Msg_Warning);
		delay_Second(1);
		//关闭20V PDO的4A以上设置
		CfgData.FixedPDOCfg.PDO20VICCMAX=4000;
		CfgData.FixedPDOCfg.IsEnable20VPDOSet=false;
		if(!WriteConfiguration(&CfgUnion,true))
			{
			ShowPostInfo(80,"存储器写入异常\0","E9",Msg_Fault);
			SelfTestErrorHandler();
			}	
		}
	//检查
	else if(!CurrentIP2366FW->IsExtendPDOCapable&&(CfgData.FixedPDOCfg.PDO20VICCMAX%50))
		{
		ShowPostInfo(78,"20V PDO电流值非法\0","WA",Msg_Warning);
		delay_Second(1);
		ShowPostInfo(78,"已自动修正\0","WA",Msg_Warning);
		delay_Second(1);
		//修正PDO结果
		i=CfgData.FixedPDOCfg.PDO20VICCMAX/50;
		i*=50;
	  if(i>7000)i=7000;
		if(i<1000)i=1000;
		CfgData.FixedPDOCfg.PDO20VICCMAX=i;
		if(!WriteConfiguration(&CfgUnion,true))
			{
			ShowPostInfo(80,"存储器写入异常\0","E9",Msg_Fault);
			SelfTestErrorHandler();
			}				
		}
	//检查高压SCP支持	
	if(!!CurrentIP2366FW->IsHSCPCapable&&CfgData.OutputConfig.IsEnableHSCPOut)
		{
		ShowPostInfo(78,"芯片为公版固件\0","2E",Msg_Warning);
		delay_Second(1);
		ShowPostInfo(78,"高功率SCP已禁用\0","2E",Msg_Warning);
		delay_Second(1);
		//关闭高功率SCP功能
		CfgData.OutputConfig.IsEnableHSCPOut=false;
		if(!WriteConfiguration(&CfgUnion,true))
			{
			ShowPostInfo(80,"存储器写入异常\0","E9",Msg_Fault);
			SelfTestErrorHandler();
			}		
		}
	//设置再充电参数
	ShowPostInfo(80,"设置再充电参数\0","2F",Msg_Statu);	
	if(!IP2366_SetReChargeParam(CfgData.VRecharge,CfgData.IStop))
		{
		ShowPostInfo(80,"再充参数设置失败\0","F2",Msg_Fault);
		SelfTestErrorHandler();
		}
	//设置低压保护参数
	ShowPostInfo(82,"设置低压保护电压\0","30",Msg_Statu);	
	if(!IP2366_SetVLowVolt(CfgData.Vlow))	
		{
		ShowPostInfo(82,"低压保护设置失败\0","F3",Msg_Fault);
		SelfTestErrorHandler();
		}
	//开始写寄存器
	CurrentStorDisState=IP2366_IsEnableDischarge(ADCO.Vbatt);	
	if(!IsBootFromVBUS)
		{
		ShowPostInfo(85,"设置放电系统\0","31",Msg_Statu);	
		if(!IP2366_InitOutputSystem(CurrentStorDisState))
			{
			ShowPostInfo(85,"放电系统设置失败\0","F4",Msg_Fault);
			SelfTestErrorHandler();
			}
		}
	//设置输入
	ShowPostInfo(88,"设置充电系统\0","32",Msg_Statu);
	switch(StorageMode)
		{
		//根据存储模式配置输出
		case StorageMode_OFF:ICFG.FullVoltage=CfgData.InputConfig.FullVoltage;break; //存储模式禁用,使用系统设置的最高电压
		case StorageMode_3V6:ICFG.FullVoltage=3600;break; //存储模式启用，电池最大电压3.6V
		case StorageMode_3V7:ICFG.FullVoltage=3700;break; //存储模式启用，电池最大电压3.7V
	  case StorageMode_3V8:ICFG.FullVoltage=3800;break;	//存储模式启用，电池最大电压3.8V
		}
	ICFG.ChargePower=CfgData.InputConfig.ChargePower;	
	ICFG.PreChargeCurrent=CfgData.InputConfig.PreChargeCurrent; //其他参数照常填写
	ICFG.IsEnableCharger=true; //充电器始终开启
	ICFG.ChargeCurrent=DCDCOutputBit?CurrentIP2366FW->IP2366ICCMAX:CfgData.InputConfig.ChargeCurrent; //初始化时如果开启输出则填写9.7A	
  //设置PDO输入
	if(!IP2366_SetInputState(&ICFG,true))
		{
		ShowPostInfo(88,"充电系统设置失败\0","F5",Msg_Fault);
		SelfTestErrorHandler();
		}	
	//设置PDO参数
	ShowPostInfo(92,"设置PDO广播\0","33",Msg_Statu);
	IsCmdSendOK=IP2366_SetFixedPDO(&CfgData.FixedPDOCfg); //设置固定挡位的PDO参数
	if(!IsCmdSendOK||!IP2366_SetPDOBroadCast(&CfgData.PDOCFG))
		{
		if(!IsCmdSendOK)ShowPostInfo(92,"PDO电流设置失败\0","F7",Msg_Fault);
		else ShowPostInfo(92,"PDO广播设置失败\0","F6",Msg_Fault);
		SelfTestErrorHandler();		
		}
	//设置PPS配置
	ShowPostInfo(93,"设置PPS广播\0","34",Msg_Statu);
	if((CfgData.PPSConfig.PPS1Current>3000||CfgData.PPSConfig.PPS2Current>3000)&&!CurrentIP2366FW->IsExtendPDOCapable)	
		{
		ShowPostInfo(78,"芯片为公版固件\0","35",Msg_Warning);
		delay_Second(1);
		ShowPostInfo(78,"PPS电流已重置\0","35",Msg_Warning);
		delay_Second(1);
		//重置PPS电流为芯片默认值
		CfgData.PPSConfig.PPS1Current=3000;
		CfgData.PPSConfig.PPS2Current=3000;
		CfgData.PPSConfig.IsEnablePPS1Set=false;
		CfgData.PPSConfig.IsEnablePPS2Set=false;
		if(!WriteConfiguration(&CfgUnion,true))
			{
			ShowPostInfo(80,"存储器写入异常\0","E9",Msg_Fault);
			SelfTestErrorHandler();
			}		
		}		
	//写入寄存器设置PPS
	if(!IP2366_SetPPSCurrent(&CfgData.PPSConfig))
		{
		ShowPostInfo(93,"PPS广播设置失败\0","F8",Msg_Fault);
		SelfTestErrorHandler();		
		}	
	//发送重新握手的指令
	ShowPostInfo(94,"发送TypeC重连命令\0","36",Msg_Statu);
	IP2366_GetVBUSState(&VBUSState);
  if(!IsBootFromVBUS)do
		{
		IsCmdSendOK=IP2366_SetTypeCRole(TypeC_Disconnect);
		delay_ms(200);
		IsCmdSendOK&=IP2366_SetTypeCRole(TypeC_DRP);
		delay_ms(200);
		IsCmdSendOK&=IP2366_SetOTPSign();
		if(IsCmdSendOK)break;
		retry--;
		ShowPostInfo(94,"重连命令重试中\0","37",Msg_Warning);
		}
	while(retry>0);
	//充电器不太聪明，强制输出20V为了避免芯片充电拉爆，强制充电器reset然后掉回去5V
	else if(VBUSState.VBUSVolt>13.0)	
		{
		ShowPostInfo(93,"充电器电压过高\0","4A",Msg_Warning);
		delay_ms(400);
		ShowPostInfo(93,"正在尝试重置\0","4A",Msg_Warning);
		delay_Second(1);	
		ShowPostInfo(93,"可能需要1分钟\0","4A",Msg_Warning);		
		delay_ms(300);
		ShowPostInfo(93,"请耐心等待\0","4A",Msg_Warning);
		delay_ms(200);
		Force2366ToRestart();
		AUXPSU_SetIPDState(false); //这里需要关闭Type-C IPD使系统掉电

		retry=200;
		do
			{
			retry--;
			delay_ms(30);
			}
		while(retry);
		ShowPostInfo(93,"充电器重置超时\0","FE",Msg_Fault);
		SelfTestErrorHandler();	
		}
	//否则只执行设置OTP
	else do
		{
		IsCmdSendOK=IP2366_SetOTPSign();
		if(IsCmdSendOK)break;
		retry--;
		delay_ms(10);
		ShowPostInfo(94,"重连命令重试中\0","37",Msg_Warning);
		}
	while(retry>0);
	//尝试失败
	if(!retry)
		{
		ShowPostInfo(94,"重连命令发送失败\0","F9",Msg_Fault);
		SelfTestErrorHandler();
		}
	IP2366_ClearOCFlag(); //移除OC Flag		
	//后处理完毕，此时如果是从VBUS启动的，则复位定时器
	if(IsBootFromVBUS)WaitAfterTypeCRemoved=50;	
	}
