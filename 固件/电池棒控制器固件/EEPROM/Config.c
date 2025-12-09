#include "Config.h"
#include "ht32.h"
#include "24Cxx.h"
#include "GUI.h"
#include "delay.h"

//配置结构体
CfgUnionDef CfgUnion;
StorageModeDef StorageMode;
bool DCDCOutputBit;
bool UsingBackupConfig=false;

//检测出厂设置时同步不需要更改的内容
void SyncUnResetThings(CfgUnionDef *IN)
	{
  IN->ROMImage.Data.Data.BatteryCurrentCalFactor=CfgData.BatteryCurrentCalFactor;
  IN->ROMImage.Data.Data.BatteryVoltageCalFactor=CfgData.BatteryVoltageCalFactor;
	IN->ROMImage.Data.Data.SystemTempCalFactor=CfgData.SystemTempCalFactor;
	IN->ROMImage.Data.Data.TypeCAmpereCalCharge=CfgData.TypeCAmpereCalCharge;
	IN->ROMImage.Data.Data.TypeCVoltageCal=CfgData.TypeCVoltageCal;
	IN->ROMImage.Data.Data.TypeCAmpereCal=CfgData.TypeCAmpereCal;
	IN->ROMImage.Data.Data.EnableAdvAccess=CfgData.EnableAdvAccess;
	IN->ROMImage.Data.Data.AutoSaveCfg=CfgData.AutoSaveCfg;
	}

//自检过程中尝试写入配置存储
void TryToSaveConfigDuringPost(char Present)
	{
	if(WriteConfiguration(&CfgUnion,true))return;
	//存储器写入失败，报错
	ShowPostInfo(Present,"存储器写入异常\0","E9",Msg_Fault);
	SelfTestErrorHandler();
	}	

//自检过程中尝试读取配置存储	
void TryToReadConfigDuringPost(char Present,bool IsBackup)
	{
	if(ReadConfiguration(&CfgUnion,IsBackup))return;
	//存储器读取失败，报错
	ShowPostInfo(Present,"存储器读取异常\0","E8",Msg_Fault);
	SelfTestErrorHandler();
	}
	
//将默认配置加载到指定位置
void LoadDefaultConfig(CfgUnionDef *IN,bool IsFactoryOverride)
	{
	//校准系数配置
	if(IsFactoryOverride)
		{
		IN->ROMImage.Data.Data.SystemTempCalFactor=-20;      	//温度校准配置
		IN->ROMImage.Data.Data.BatteryCurrentCalFactor=1007;
		IN->ROMImage.Data.Data.BatteryVoltageCalFactor=1000;
		}
	//存储模式和低压保护配置
	IN->ROMImage.Data.Data.StorageModeINROM=StorageMode_OFF;	
	IN->ROMImage.Data.Data.Vlow=VLow_2V5;
	IN->ROMImage.Data.Data.PreChargeEndVoltage=VLow_3V0;   //预充截止电压设置为3V
	//固定挡位PDO配置	
	IN->ROMImage.Data.Data.FixedPDOCfg.IsEnable12VPDOSet=false;
	IN->ROMImage.Data.Data.FixedPDOCfg.IsEnable15VPDOSet=false;
	IN->ROMImage.Data.Data.FixedPDOCfg.IsEnable9VPDOSet=false;
	if(!CurrentIP2366FW->IsHyperChargeCapable)IN->ROMImage.Data.Data.FixedPDOCfg.IsEnable20VPDOSet=false;
	else IN->ROMImage.Data.Data.FixedPDOCfg.IsEnable20VPDOSet=CurrentIP2366FW->IsExtendPDOCapable;
	IN->ROMImage.Data.Data.FixedPDOCfg.PDO9VICCMAX=3000;
	IN->ROMImage.Data.Data.FixedPDOCfg.PDO12VICCMAX=3000;
	IN->ROMImage.Data.Data.FixedPDOCfg.PDO15VICCMAX=3000;
	if(!CurrentIP2366FW->IsHyperChargeCapable)IN->ROMImage.Data.Data.FixedPDOCfg.PDO20VICCMAX=4000;
	else IN->ROMImage.Data.Data.FixedPDOCfg.PDO20VICCMAX=CurrentIP2366FW->IsExtendPDOCapable?7000:4000;	
	//输入快充和自动省电配置
	IN->ROMImage.Data.Data.EnableSmartSinkPower=CurrentIP2366FW->ExtendedTCSetting;
	IN->ROMImage.Data.Data.MaxSnkPower=CurrentIP2366FW->MaxCapableChgPower;
	IN->ROMImage.Data.Data.EnableAutoPowerSave=false;
	IN->ROMImage.Data.Data.SinkConfig.EnableSinkDPDM=true;
	IN->ROMImage.Data.Data.SinkConfig.EnableSinkPD=true;
	IN->ROMImage.Data.Data.SinkConfig.EnableSinkSCP=true; //输入快充协议
	//输入配置
	IN->ROMImage.Data.Data.VRecharge=Recharge_0V1;
	IN->ROMImage.Data.Data.IStop=IStop_200mA;
	IN->ROMImage.Data.Data.InputConfig.ChargeCurrent=CurrentIP2366FW->IP2366ICCMAX;
	IN->ROMImage.Data.Data.InputConfig.ChargePower=CurrentIP2366FW->MaxCapableChgPower;
	IN->ROMImage.Data.Data.InputConfig.FullVoltage=4200;
	IN->ROMImage.Data.Data.InputConfig.PreChargeCurrent=400;
	IN->ROMImage.Data.Data.InputConfig.IsEnableCharger=true;
	//输出配置
	IN->ROMImage.Data.Data.OutputConfig.IsEnableDPDMOut=true;
	IN->ROMImage.Data.Data.OutputConfig.IsEnableOutput=true;
	IN->ROMImage.Data.Data.OutputConfig.IsEnablePDOut=true;
	IN->ROMImage.Data.Data.OutputConfig.IsEnableSCPOut=true;
	IN->ROMImage.Data.Data.OutputConfig.IsEnableHSCPOut=false; //默认关闭
	//PDO配置
	IN->ROMImage.Data.Data.PDOCFG.EnablePPS1=true;
	IN->ROMImage.Data.Data.PDOCFG.EnablePPS2=true;
	IN->ROMImage.Data.Data.PDOCFG.Enable20V=true;
	IN->ROMImage.Data.Data.PDOCFG.Enable15V=true;
	IN->ROMImage.Data.Data.PDOCFG.Enable12V=true;  
	IN->ROMImage.Data.Data.PDOCFG.Enable9V=true;  //所有PDO都打开
	//安全配置
	IN->ROMImage.Data.Data.EnableExtendedBalance=true; 								 //默认开启自动均衡
	IN->ROMImage.Data.Data.EnableThermalStepdown=true;
  if(IsFactoryOverride)IN->ROMImage.Data.Data.EnableAdvAccess=false; //高级菜单使能不重置
  IN->ROMImage.Data.Data.EnableChargeConfig=false;
  IN->ROMImage.Data.Data.EnableChargPowerConfig=true;
	IN->ROMImage.Data.Data.EnableDischargeConfig=true;
	IN->ROMImage.Data.Data.EnableLVProtectConfig=false;
	IN->ROMImage.Data.Data.EnablePDOConfig=true;
  IN->ROMImage.Data.Data.EnableOTPConfig=false;
	IN->ROMImage.Data.Data.EnableTCCalibration=false;
	IN->ROMImage.Data.Data.OverHeatLockTemp=85;
	//PPS1和PPS2电流
	IN->ROMImage.Data.Data.PPSConfig.PPS1Current=CurrentIP2366FW->IsExtendPDOCapable?6350:3000;
  IN->ROMImage.Data.Data.PPSConfig.PPS2Current=CurrentIP2366FW->IsExtendPDOCapable?6350:3000;
	IN->ROMImage.Data.Data.PPSConfig.IsEnablePPS1Set=CurrentIP2366FW->IsExtendPDOCapable;
	IN->ROMImage.Data.Data.PPSConfig.IsEnablePPS2Set=CurrentIP2366FW->IsExtendPDOCapable;
	//TypeC矫正设置
	if(IsFactoryOverride)
		{
		IN->ROMImage.Data.Data.TypeCAmpereCalCharge=1000;
		IN->ROMImage.Data.Data.TypeCVoltageCal=1000;
		IN->ROMImage.Data.Data.TypeCAmpereCal=1000;
		}
	//显示方向设置
	IN->ROMImage.Data.Data.SleepCfg=System_Sleep_Deep; //默认开启深度睡眠模式
  IN->ROMImage.Data.Data.EnableFastBoot=true;
	IN->ROMImage.Data.Data.EnableLargeMenu=true;
	IN->ROMImage.Data.Data.DisplayDir=LCDDisplay_Hori_Invert;
  //容量测试配置
	IN->ROMImage.Data.Data.AutoSaveCfg=AutoSave_Enabled; //默认是自动存盘模式
	IN->ROMImage.Data.Data.InstantCTest=InstantCTest_NotTriggered;
	//最大PD输入配置
  IN->ROMImage.Data.Data.MaxVPD=CurrentIP2366FW->IsHyperChargeCapable?PDMaxIN_28V:PDMaxIN_20V;
  //均衡系统配置
	IN->ROMImage.Data.Data.BalanceMode=Balance_ChgOnly; //均衡仅在充电时启用
	}

//恢复默认设置(重置一切东西)
static void RestoreDefaultConfig(void)
	{
  LoadDefaultConfig(&CfgUnion,true);
	}

//设置里面恢复出厂但是不重置校准配置
void RestoreFactoryWithoutSomeSettings(void)
	{
	 LoadDefaultConfig(&CfgUnion,false);
	}

//计算CRC32
unsigned int CalcROMCRC32(CfgUnionDef *IN)
	{
	int i;
	unsigned int result;
	unsigned char StorBuf;
  CKCU_PeripClockConfig_TypeDef CLKConfig={{0}};
	//初始化CRC32        
	CLKConfig.Bit.CRC = 1;
	CKCU_PeripClockConfig(CLKConfig,ENABLE);//启用CRC-32时钟 
	CRC_DeInit(HT_CRC);//清除配置
	HT_CRC->SDR = 0x0;//CRC-32 poly: 0x04C11DB7  
	HT_CRC->CR = CRC_32_POLY | CRC_BIT_RVS_WR | CRC_BIT_RVS_SUM | CRC_BYTE_RVS_SUM | CRC_CMPL_SUM;
	//写数据
	for(i=0;i<sizeof(SystemCfgDef);i++)wb(&HT_CRC->DR,IN->ROMImage.Data.ByteBuf[i]);
	result=HT_CRC->CSR;
	CRC_DeInit(HT_CRC);//清除CRC结果
	HT_CRC->SDR = 0x0;//CRC-32 poly: 0x04C11DB7  
	HT_CRC->CR = CRC_32_POLY | CRC_BIT_RVS_WR | CRC_BIT_RVS_SUM | CRC_BYTE_RVS_SUM | CRC_CMPL_SUM;
	for(i=0;i<16;i++)
		{
		switch(result&0x03)
			{
			case 0:StorBuf='R';break;
			case 1:StorBuf='?';break;
			case 2:StorBuf='@';break;
			case 3:StorBuf='n';break;
			}
		result>>=2; //右移两位
		StorBuf^=0x01<<(i%8); //和i的8次结果进行XOR
		wb(&HT_CRC->DR,StorBuf+i);
		}
  //读取数据结果
	result=HT_CRC->CSR;
	CRC_DeInit(HT_CRC);
	CKCU_PeripClockConfig(CLKConfig,DISABLE);//禁用CRC-32时钟节省电力
	//返回结果
	return result^0x5AA53453;
	}

//读取数据
bool ReadConfiguration(CfgUnionDef *Out,bool IsUsingBackup)
	{
	return !M24C512_PageRead(Out->ByteBuf,IsUsingBackup?sizeof(CfgUnionDef):0x0000,sizeof(CfgUnionDef));
	}
	
//写入数据
bool WriteConfiguration(CfgUnionDef *IN,bool ForceUpdate)
	{
	unsigned int CRCResult,bufReselt;
	CfgUnionDef buf;
	//计算并填写当前ROM的CRC,和ROM里面的比对,相同则不写入
	CRCResult=CalcROMCRC32(IN);
	if(!ReadConfiguration(&buf,UsingBackupConfig))return false;
	bufReselt=CalcROMCRC32(&buf);
	//强制更新模式关闭，ROM内数据没有损坏且当前配置一样则跳过写入并返回true
	if(!ForceUpdate&&CRCResult==buf.ROMImage.CRCResult&&bufReselt==buf.ROMImage.CRCResult)return true;
	//正常写入
	IN->ROMImage.CRCResult=CRCResult;
	return !M24C512_PageWrite(IN->ByteBuf,UsingBackupConfig?sizeof(CfgUnionDef):0x0000,sizeof(CfgUnionDef));
	}
	
//检查指定的配置文件状态
bool CheckIfConfigOK(bool IsBackup)
	{
	CfgUnionDef buf;
	if(!ReadConfiguration(&buf,IsBackup))return false;
	if(CalcROMCRC32(&buf)!=buf.ROMImage.CRCResult)return false;
	//检查通过，返回true
	return true;
	}	
	
//检查EEPROM和当前的配置是否相同
bool CheckIfConfigIsSame(void)
	{
	unsigned int CRCResult;
	CfgUnionDef buf;
	//计算并填写当前ROM的CRC,和ROM里面的比对
	CRCResult=CalcROMCRC32(&CfgUnion);
	if(!ReadConfiguration(&buf,UsingBackupConfig))return false;
	//配置一样
	if(CRCResult==buf.ROMImage.CRCResult)return true;
	return false;
	}
	
//开机时读取配置
void LoadConfig(void)
	{
	int CRCResult;
	extern bool EnableDetailOutput;	
	bool IsNeedToUpgrade=false,result;
  CfgUnionDef BackUpCfg;
	//读取数据	
	ShowPostInfo(30,"加载系统配置文件\0","09",Msg_Statu);
	TryToReadConfigDuringPost(30,false);
	//检查配置
	CRCResult=CalcROMCRC32(&CfgUnion);
	if(CRCResult!=CfgChecksum)
		{
		ShowPostInfo(32,"尝试读取备用配置\0","0A",Msg_Statu);
		delay_ms(300);
		//主文件配置数据损坏，尝试读取备用	
		TryToReadConfigDuringPost(30,true);
		CRCResult=CalcROMCRC32(&CfgUnion);	
		if(CRCResult!=CfgChecksum)
			{
			//备用文件也坏了,重置所有数据
			ShowPostInfo(32,"无可用的配置文件","W4",Msg_Warning);
			delay_Second(1);
			RestoreDefaultConfig();
			UsingBackupConfig=false;
			TryToSaveConfigDuringPost(32);  //写主配置文件 
			UsingBackupConfig=true;
			TryToSaveConfigDuringPost(32); 	//写备用配置文件	
			ShowPostInfo(32,"已加载出厂设置","W4",Msg_Warning);	
			delay_Second(1);	
			}
		else
			{
			ShowPostInfo(32,"主用配置文件损坏","0B",Msg_Warning);
			delay_ms(300);	
			//尝试对主用数据进行覆盖
			UsingBackupConfig=false;
			TryToSaveConfigDuringPost(32);
			UsingBackupConfig=true;
			ShowPostInfo(32,"已加载备用文件","0B",Msg_Warning);
			delay_ms(300);
			}
		}
	else
		{
		ShowPostInfo(32,"检查备用配置文件\0","42",Msg_Statu);
		//系统检查通过，检查备用配置文件
		if(!ReadConfiguration(&BackUpCfg,true))
			{
			ShowPostInfo(32,"存储器读取异常\0","E8",Msg_Fault);
			SelfTestErrorHandler();
			}
		//CRC32比对处理
		CRCResult=CalcROMCRC32(&BackUpCfg);
		if(CRCResult!=BackUpCfg.ROMImage.CRCResult)
			{
			ShowPostInfo(32,"备用配置文件损坏","WB",Msg_Warning);
			delay_ms(300);	
			UsingBackupConfig=true;
			TryToSaveConfigDuringPost(32);  //写主配置文件 	
			ShowPostInfo(32,"已覆盖为主用配置","WB",Msg_Warning);
			delay_ms(300);
			}
		}
	ShowPostInfo(35,"检查系统配置数据\0","0C",Msg_Statu);
	//对PD数据进行修正	
	if(CfgData.InputConfig.ChargeCurrent>CurrentIP2366FW->IP2366ICCMAX)
		{
		ShowPostInfo(35,"峰值电流配置非法\0","W5",Msg_Warning);
		delay_Second(1);
    CfgData.InputConfig.ChargeCurrent=CurrentIP2366FW->IP2366ICCMAX;
		IsNeedToUpgrade=true;	
		}		
	if(CfgData.InputConfig.ChargePower>CurrentIP2366FW->MaxCapableChgPower)
		{
		//充电功率超过芯片capable的能力，修正
		ShowPostInfo(35,"充电功率配置非法\0","WC",Msg_Warning);
		delay_Second(1);
		CfgData.InputConfig.ChargePower=CurrentIP2366FW->MaxCapableChgPower;	
		IsNeedToUpgrade=true;		
		}
	if(CfgData.MaxVPD==PDMaxIN_20V&&CfgData.InputConfig.ChargePower==Power_140W)	
		{
		ShowPostInfo(35,"PD输入配置非法\0","W6",Msg_Warning);
		delay_Second(1);
		CfgData.InputConfig.ChargePower=CurrentIP2366FW->MaxCapableChgPower;	
		IsNeedToUpgrade=true;	
		}
	//如果2366固件不支持高级Sink配置，则始终禁用超级省电和智能输入功率并且设置最大Sink功率
	if(CfgData.EnableAutoPowerSave)result=true;
	else if(CfgData.MaxSnkPower!=CurrentIP2366FW->MaxCapableChgPower)result=true;
	else if(CfgData.EnableSmartSinkPower)result=true;
	else result=false;
		
	if(!CurrentIP2366FW->ExtendedTCSetting&&result)
		{
		CfgData.EnableAutoPowerSave=false;
		CfgData.MaxSnkPower=CurrentIP2366FW->MaxCapableChgPower;
		CfgData.EnableSmartSinkPower=false;
		ShowPostInfo(35,"Sink配置非法\0","WD",Msg_Warning);
		delay_Second(1);
		IsNeedToUpgrade=true;					
		}
		
	if(CfgData.PreChargeEndVoltage<VLow_2V8)
		{
		CfgData.PreChargeEndVoltage=VLow_2V8;
		ShowPostInfo(35,"涓流电压配置非法\0","WE",Msg_Warning);
		delay_Second(1);
		IsNeedToUpgrade=true;		
		}		

	//需要更新配置
	if(IsNeedToUpgrade)
		{
		TryToSaveConfigDuringPost(35);		
		ShowPostInfo(35,"已进行自动修正\0","0D",Msg_Warning);
		delay_Second(1);
		}
	//读取配置后如果关闭快速启动则显示剩下的操作
	DCDCOutputBit=CfgData.OutputConfig.IsEnableOutput; //更新输出bit
	StorageMode=CfgData.StorageModeINROM; //从ROM内更新存储模式的处理
	if(!CfgData.EnableFastBoot)EnableDetailOutput=true;
	else EnableDetailOutput=false;
	}
