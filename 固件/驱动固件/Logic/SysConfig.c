#include "ModeControl.h"
#include "cms8s6990.h"
#include "stdbool.h"
#include "SysConfig.h"
#include "Flash.h"
#include "SideKey.h"
#include "SpecialMode.h"
#include "delay.h"
#include "LEDMgmt.h"
#include "SysReset.h"

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

//从EEPROM内寻找最后的一组Sys配置
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
	SysMode=Operation_Normal; //默认处于解锁模式
	SysCfg.LocatorCfg=Locator_Green; //默认是绿灯亮
	SysCfg.FadingCfg=Fading_OFF; //关闭电流拖尾
	RestoreToMinimumSysCurrent();
	IsMainMemEnabled=1;
	IsSpecMemEnabled=0; //开启正常挡位记忆，关闭特殊挡位记忆
	IsRampEnabled=0; //默认为挡位模式
	IsPowerModeEnabled=0; //默认为ECO模式
	}	
	
//尝试检测用户进行重置操作	
void ResetSysConfigToDefault(void)
	{
	unsigned char delay;
	//准备初始的系统设置
  PrepareFactoryDefaultCfg();
	//保存数据并显示状态
	SaveSysConfig(0); //写数据写成默认值
	SetFlashState(0); //锁定Flash
	//配置指示灯准备显示
	delay=100;
	IsHalfBrightness=0;
	LEDMode=LED_AmberBlink; //LED模式配置为黄色快闪
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
	IsHalfBrightness=0;
	LEDMode=LED_RedBlink; //LED模式配置为红色快闪
	while(--delay)
		{
		delay_ms(10);
		LEDControlHandler();
		}
	//时间到，令系统reboot
	TriggerSoftwareReset();
	}
	
//读取无极调光配置
void ReadSysConfig(void)
	{
	SysROMImg ROMData;
	//读取数据
	CurrentIdx=SearchSysConfig(&ROMData);
	//进行读出数据的校验
	if(ROMData.Data.CheckSum==PEC8Check(ROMData.Data.SysConfig.ByteBuf,sizeof(SysStorDef)))
		{
		//校验成功，加载数据
		IsPowerModeEnabled=ROMData.Data.SysConfig.Data.BitfieldMem1&PowerECOMode_MSK?1:0;
		IsMainMemEnabled=ROMData.Data.SysConfig.Data.BitfieldMem1&IsEnableMainMemory_MSK?1:0;
		IsSpecMemEnabled=ROMData.Data.SysConfig.Data.BitfieldMem1&IsEnableSpecMemory_MSK?1:0;
		IsRampEnabled=ROMData.Data.SysConfig.Data.BitfieldMem1&IsRampEnabled_MSK?1:0;
		SysMode=ROMData.Data.SysConfig.Data.BitfieldMem1&IsLocked_MSK?Operation_Locked:Operation_Normal;
		
		SysCfg.LocatorCfg=ROMData.Data.SysConfig.Data.LocatorCfg; 
		SysCfg.RampCurrent=ROMData.Data.SysConfig.Data.SysCurrent;
		SysCfg.FadingCfg=ROMData.Data.SysConfig.Data.FadingCfg;      //加载其余系统设置
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
		SysMode=Operation_Locked;   //出厂写PROM的时候默认是锁定
		SaveSysConfig(1); //重建数据后立即保存参数
		ShowEPROMCorrupted(); //显示EEPROM损坏
		}
	//读取操作完毕，锁定flash	
	SetFlashState(0);
	}

//恢复到无极调光模式的最低电流
void RestoreToMinimumSysCurrent(void)	
	{
	unsigned char i;
	extern code ModeStrDef ModeSettings[ModeTotalDepth];
	for(i=0;i<ModeTotalDepth;i++)if(ModeSettings[i].ModeIdx==Mode_Ramp)
			SysCfg.RampCurrent=ModeSettings[i].MinCurrent; //找到挡位数据中无极调光的挡位
	}

//保存无极调光配置
void SaveSysConfig(bit IsForceSave)
	{
	unsigned char i;
	unsigned char BFBuf=0;
	SysROMImg SavedData;
	//解锁flash（CRC校验模块需要读取Flash所以需要解锁）
	SetFlashState(1);
  //开始进行数据构建
	if(SysMode==Operation_Locked)BFBuf|=IsLocked_MSK;			//是否锁定
	if(IsRampEnabled)BFBuf|=IsRampEnabled_MSK;						//是否开启无极调光
	if(IsMainMemEnabled)BFBuf|=IsEnableMainMemory_MSK;	//是否启用主挡位记忆
	if(IsSpecMemEnabled)BFBuf|=IsEnableSpecMemory_MSK;    //是否启用特殊功能挡位记忆
	if(IsPowerModeEnabled)BFBuf|=PowerECOMode_MSK;        //是否启用POWER模式
		
	SavedData.Data.SysConfig.Data.BitfieldMem1=BFBuf;
	SavedData.Data.SysConfig.Data.FadingCfg=SysCfg.FadingCfg;
	SavedData.Data.SysConfig.Data.LocatorCfg=SysCfg.LocatorCfg;
  SavedData.Data.SysConfig.Data.SysCurrent=SysCfg.RampCurrent;
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
