#include "cms8s6990.h"
#include "ModeSel.h"
#include "SysConfig.h"
#include "Flash.h"
#include "SideKey.h"
#include "delay.h"
#include "LEDMgmt.h"
#include "SysReset.h"
#include "OutputChannel.h"

//内部全局
static xdata unsigned int CurrentIdx;
static xdata u8 CurrentCRC;

//CRC-8计算 
static u8 PEC8Check(char *DIN,char Len)
{
 unsigned char crcbuf=0xFF;
 unsigned char i;
 do
	{
  //载入数据
  crcbuf^=*DIN++;
  //计算
  i=8;
	do
   {
	 if(crcbuf&0x80)crcbuf=(crcbuf<<1)^0x07;//最高位为1，左移之后和多项式XOR
	 else crcbuf<<=1;//最高位为0，只移位不XOR
	 }
	while(--i);
	}
 while(--Len);
 //输出结果
 return crcbuf;
}

//从EEPROM内寻找最后的一组系统配置
static int SearchSysConfig(SysROMImg *ROMData)
	{
	unsigned char i;
	int Len=0;
	//解锁flash并开始读取
	SetFlashState(1);
	do
		{		
		for(i=0;i<sizeof(SysROMImageDef);i++)Flash_Operation(DataFlash_Read,i+(Len*sizeof(SysROMImg)),&ROMData->ByteBuf[i]); //从ROM内读取数据
		if(ROMData->Data.CheckSum!=PEC8Check(ROMData->Data.SysConfig.ByteBuf,sizeof(SysStorDef)))break; //找到了没有被写入CRC校验不过的地方，就是你了
		Len++;
		}
	while(Len<SysCfgGroupLen);
	//读取上一组正确的配置
	if(Len>0)Len--;
	for(i=0;i<sizeof(SysROMImageDef);i++)Flash_Operation(DataFlash_Read,i+(Len*sizeof(SysROMImg)),&ROMData->ByteBuf[i]);
	//读取结束，返回上一组有数据的index
	return Len;
	}
//准备初始的系统设置
static void PrepareFactoryDefaultCfg(void)
	{
	RampVoltage=VMinMaxCfg.SysMinVolt;
	RampDuty=(float)VMinMaxCfg.SysMinSpeed;
	IsSystemLocked=0;
	IsEnableIdleLED=1;
	IsEnableBattCfgLock=0;
	IsEnable2SMode=0;
	}	
	
//尝试检测用户进行重置操作	
static void ResetSysConfigToDefault(void)
	{
	unsigned char delay;
	bit BattCfgLockState;
	//如果系统处于锁定状态，则不允许重置出厂设置
  if(IsSystemLocked)return;
	//准备初始的系统设置
	BattCfgLockState=IsEnableBattCfgLock;
  PrepareFactoryDefaultCfg();
	if(BattCfgLockState)IsEnableBattCfgLock=1; //进行配置锁的记忆确保锁定后除了重新刷固件没有解锁的办法
	//保存数据并显示状态
	SaveSysConfig(0); //写数据写成默认值
	SetFlashState(0); //锁定Flash
	//配置指示灯准备显示
	delay=100;
	LEDMode=LED_AmberBlinkFast; //LED模式配置为黄色快闪
	do
		{
		delay_ms(10);
		LEDControlHandler();
		//松开按键后开始计时
		if(GetSideKeyRawGPIOState())delay--;
		}
	while(delay);
	//触发系统重启
	TriggerSoftwareReset();
	}	
	
//显示系统数据存在错误
static void ShowEPROMCorrupted(void)
	{
	unsigned char delay=0xFF;
	//读取操作完毕，锁定flash	
	SetFlashState(0);
	//配置LED模式
	LEDMode=LED_RedBlink; //LED模式配置为红色快闪
	while(--delay)
		{
		delay_ms(10);
		LEDControlHandler();
		}
	//时间到，令系统reboot
	TriggerSoftwareReset();
	}
	
//读取无极调速配置
void ReadSysConfig(void)
	{
	xdata SysROMImg ROMData;
	//读取数据
	CurrentIdx=SearchSysConfig(&ROMData);
	//进行读出数据的校验
	if(ROMData.Data.CheckSum==PEC8Check(ROMData.Data.SysConfig.ByteBuf,sizeof(SysStorDef)))
		{
		//校验成功，加载数据
		IsEnableBattCfgLock=ROMData.Data.SysConfig.Data.BitfieldMem1&IsEnableBattCfgLock_MSK?1:0;
		IsEnable2SMode=ROMData.Data.SysConfig.Data.BitfieldMem1&IsEnable2SMode_MSK?1:0;
		IsEnableIdleLED=ROMData.Data.SysConfig.Data.BitfieldMem1&IsEnableIdleLED_MSK?1:0;
		IsSystemLocked=ROMData.Data.SysConfig.Data.BitfieldMem1&IsLocked_MSK?1:0;
		RampDuty=ROMData.Data.SysConfig.Data.RampDuty;
		RampVoltage=ROMData.Data.SysConfig.Data.RampVout;
		//存储当前的index值
		CurrentCRC=ROMData.Data.CheckSum;
		CurrentIdx++; //当前位置有数据，需要让index+1移动到未写入的位置
		
		//用户按下按键，重置设置并重启
		if(!GetSideKeyRawGPIOState())ResetSysConfigToDefault();
		}
	//校验失败重建数据
	else 
		{
		PrepareFactoryDefaultCfg(); 
		IsSystemLocked=1;  //系统刷写固件后首次上电，使系统处于锁定状态
		SaveSysConfig(1);  //重建数据后立即保存参数
		ShowEPROMCorrupted(); //显示EEPROM损坏
		}
	//读取操作完毕，锁定flash	
	SetFlashState(0);
	}

//保存无极调速配置
void SaveSysConfig(bit IsForceSave)
	{
	unsigned char i,BFBuf=0;
	xdata SysROMImg SavedData;
	//解锁flash（CRC校验模块需要读取Flash所以需要解锁）
	SetFlashState(1);
  //开始进行数据构建
	if(IsSystemLocked)BFBuf|=IsLocked_MSK;										 //是否锁定
	if(IsEnableIdleLED)BFBuf|=IsEnableIdleLED_MSK;             //是否开启有源夜光
	if(IsEnable2SMode)BFBuf|=IsEnable2SMode_MSK;               //是否开启2S模式
	if(IsEnable2SMode)BFBuf|=IsEnableBattCfgLock_MSK;          //是否开启电池配置锁
		
	SavedData.Data.SysConfig.Data.BitfieldMem1=BFBuf;
	SavedData.Data.SysConfig.Data.RampDuty=RampDuty;
	SavedData.Data.SysConfig.Data.RampVout=RampVoltage;
	SavedData.Data.CheckSum=PEC8Check(SavedData.Data.SysConfig.ByteBuf,sizeof(SysStorDef)); //计算CRC
	//进行数据比对
	if(!IsForceSave&&SavedData.Data.CheckSum==CurrentCRC)
		{
		SetFlashState(0);//读取操作完毕，锁定flash	
	  return; //跳过保存操作，数据相同	
		}
	//数据需要保存，开始检测是否需要擦除
	if(IsForceSave||CurrentIdx>=SysCfgGroupLen) 
		{
		//数据已经写满了，对扇区0和1进行完全擦除
		Flash_Operation(DataFlash_Erase,0x200,&i);  //扇区2=512-1023
		Flash_Operation(DataFlash_Erase,0,&i);      //扇区1=0-511
		//从第0个位置开始写入
		CurrentIdx=0;
		}
	//写入数据
	for(i=0;i<sizeof(SysROMImageDef);i++)Flash_Operation(DataFlash_Write,i+(CurrentIdx*sizeof(SysROMImg)),&SavedData.ByteBuf[i]);	
	CurrentIdx++; //本index已被写入，标记写到下个idx
	CurrentCRC=SavedData.Data.CheckSum; //保存本次index的CRC8
	SetFlashState(0);//写入操作完毕，锁定flash	
	}	
