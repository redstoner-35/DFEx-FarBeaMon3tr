#include "Config.h"
#include "ht32.h"
#include "24Cxx.h"
#include "GUI.h"
#include "delay.h"

//配置结构体
CfgUnionDef CfgUnion;

//将魔人配置加载到指定位置
void LoadDefaultConfig(CfgUnionDef *IN)
	{
	extern bool IsEnable17AMode;	
	IN->ROMImage.Data.Data.Vlow=VLow_2V8;
	//输入配置
	IN->ROMImage.Data.Data.VRecharge=Recharge_0V1;
	IN->ROMImage.Data.Data.IStop=IStop_200mA;
	IN->ROMImage.Data.Data.InputConfig.ChargeCurrent=IsEnable17AMode?IP2366_ICCMAX:9700;
	IN->ROMImage.Data.Data.InputConfig.ChargePower=Power_100W;
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
	IN->ROMImage.Data.Data.PDOCFG.Enable9V=true;
	//安全配置
	IN->ROMImage.Data.Data.EnableThermalStepdown=true;
	IN->ROMImage.Data.Data.EnableAdvAccess=false;
  IN->ROMImage.Data.Data.EnableChargeConfig=false;
  IN->ROMImage.Data.Data.EnableChargPowerConfig=true;
	IN->ROMImage.Data.Data.EnableDischargeConfig=true;
	IN->ROMImage.Data.Data.EnableLVProtectConfig=true;
	IN->ROMImage.Data.Data.EnablePDOConfig=true;
  IN->ROMImage.Data.Data.EnableOTPConfig=false;
	IN->ROMImage.Data.Data.OverHeatLockTemp=90;
	//显示方向设置
  IN->ROMImage.Data.Data.EnableFastBoot=true;
	IN->ROMImage.Data.Data.EnableLargeMenu=true;
	IN->ROMImage.Data.Data.DisplayDir=LCDDisplay_Hori_Invert;
  //容量测试配置
	IN->ROMImage.Data.Data.InstantCTest=InstantCTest_NotTriggered;
	//最大PD输入配置
  IN->ROMImage.Data.Data.MaxVPD=IsEnable17AMode?PDMaxIN_28V:PDMaxIN_20V;
  //均衡系统配置
	IN->ROMImage.Data.Data.BalanceMode=Balance_ChgDisOnly; //均衡仅在充放电时启用
	}

//恢复默认设置
void RestoreDefaultConfig(void)
	{
  LoadDefaultConfig(&CfgUnion);
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
bool ReadConfiguration(CfgUnionDef *Out)
	{
	return !M24C512_PageRead(Out->ByteBuf,0x0000,sizeof(CfgUnionDef));
	}
	
//写入数据
bool WriteConfiguration(CfgUnionDef *IN,bool ForceUpdate)
	{
	unsigned int CRCResult,bufReselt;
	CfgUnionDef buf;
	//计算并填写当前ROM的CRC,和ROM里面的比对,相同则不写入
	CRCResult=CalcROMCRC32(IN);
	if(!ReadConfiguration(&buf))return false;
	bufReselt=CalcROMCRC32(&buf);
	//强制更新模式关闭，ROM内数据没有损坏且当前配置一样则跳过写入并返回true
	if(!ForceUpdate&&CRCResult==buf.ROMImage.CRCResult&&bufReselt==buf.ROMImage.CRCResult)return true;
	//正常写入
	IN->ROMImage.CRCResult=CRCResult;
	return !M24C512_PageWrite(IN->ByteBuf,0x0000,sizeof(CfgUnionDef));
	}
	
//检查EEPROM和当前的配置是否相同
bool CheckIfConfigIsSame(void)
	{
	unsigned int CRCResult;
	CfgUnionDef buf;
	//计算并填写当前ROM的CRC,和ROM里面的比对
	CRCResult=CalcROMCRC32(&CfgUnion);
	if(!ReadConfiguration(&buf))return false;
	//配置一样
	if(CRCResult==buf.ROMImage.CRCResult)return true;
	return false;
	}
	
//开机时读取配置
void LoadConfig(void)
	{
	int CRCResult;
	extern bool EnableDetailOutput;	
	bool IsNeedToUpgrade=false;
	//读取数据	
	ShowPostInfo(30,"加载系统配置\0","0D",Msg_Statu);
	if(!ReadConfiguration(&CfgUnion))
		{
		ShowPostInfo(30,"存储器读取异常\0","E5",Msg_Fault);
		SelfTestErrorHandler();
		}
	//检查配置
	CRCResult=CalcROMCRC32(&CfgUnion);
	if(CRCResult!=CfgChecksum)
		{
		ShowPostInfo(30,"配置数据损坏","0E",Msg_Warning);
		delay_Second(1);
		RestoreDefaultConfig();
		if(!WriteConfiguration(&CfgUnion,true))
			{
			ShowPostInfo(30,"存储器写入异常","E6",Msg_Fault);
			SelfTestErrorHandler();
			}		
		ShowPostInfo(30,"已加载出厂设置","0E",Msg_Warning);	
		delay_Second(1);	
		}
	//对PD数据进行修正	
	if(CfgData.InputConfig.ChargeCurrent>IP2366_ICCMAX)
		{
		ShowPostInfo(30,"峰值电流配置非法\0","1F",Msg_Warning);
		delay_Second(1);
    CfgData.InputConfig.ChargeCurrent=IP2366_ICCMAX;
		IsNeedToUpgrade=true;	
		}		
	if(CfgData.MaxVPD==PDMaxIN_20V&&(CfgData.InputConfig.ChargePower==Power_140W||CfgData.InputConfig.ChargePower==Power_100W))	
		{
		ShowPostInfo(30,"系统配置非法\0","0F",Msg_Warning);
		delay_Second(1);
		CfgData.InputConfig.ChargePower=Power_65W;	
		IsNeedToUpgrade=true;	
		}
	//需要更新配置
	if(IsNeedToUpgrade)
		{
		if(!WriteConfiguration(&CfgUnion,true))
			{
			ShowPostInfo(30,"存储器写入异常\0","E6",Msg_Fault);
			SelfTestErrorHandler();
			}		
		else
			{			
			ShowPostInfo(30,"已进行自动修正\0","0F",Msg_Warning);
			delay_Second(1);
			}
		}
	//读取配置后如果关闭快速启动则显示剩下的操作
	if(!CfgData.EnableFastBoot)EnableDetailOutput=true;
	else EnableDetailOutput=false;
	}
