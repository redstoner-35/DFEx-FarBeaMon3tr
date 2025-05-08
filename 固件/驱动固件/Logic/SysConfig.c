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
static xdata int CurrentIdx=0;
static xdata u8 CurrentCRC=0;

//CRC-8计算 
static u8 PEC8Check(char *DIN,char Len)
{
 unsigned char crcbuf=0xFF;
 char i;
 do
	{
  //载入数据
  crcbuf^=*DIN++;
  //计算
  for(i=8;i;i--)
   {
	 if(crcbuf&0x80)crcbuf=(crcbuf<<1)^0x07;//最高位为1，左移之后和多项式XOR
	 else crcbuf<<=1;//最高位为0，只移位不XOR
	 }
	}
 while(--Len);
 //输出结果
 return crcbuf;
}

//从EEPROM内寻找最后的一组Sys配置
static int SearchSysConfig(SysROMImg *ROMData)
	{
	char i;
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

//读取无极调光配置
void ReadSysConfig(void)
	{
	extern code ModeStrDef ModeSettings[ModeTotalDepth];
	SysROMImg ROMData;
	bit KState=GetIfKeyPressed();
	//读取数据
	CurrentIdx=SearchSysConfig(&ROMData);
	//进行读出数据的校验
	if(!KState&&ROMData.Data.CheckSum==PEC8Check(ROMData.Data.SysConfig.ByteBuf,sizeof(SysStorDef)))
		{
		//校验成功，加载数据
		SysMode=ROMData.Data.SysConfig.Data.IsSystemLocked?Operation_Locked:Operation_Normal;
		SysCfg.LocatorCfg=ROMData.Data.SysConfig.Data.LocatorCfg; 
		SysCfg.RampCurrent=ROMData.Data.SysConfig.Data.SysCurrent;
		IsRampEnabled=ROMData.Data.SysConfig.Data.IsRampEnabled?1:0;
		//存储当前的index值
		CurrentCRC=ROMData.Data.CheckSum;
		CurrentIdx++; //当前位置有数据，需要让index+1移动到未写入的位置
		}
	//校验失败重建数据
	else 
		{
		SysMode=Operation_Normal; //默认处于解锁模式
		SysCfg.LocatorCfg=Locator_Green; //默认是绿灯亮
		RestoreToMinimumSysCurrent();
		IsRampEnabled=0; //默认为挡位模式
		SaveSysConfig(1); //重建数据后立即保存参数
		if(KState)while(GetIfKeyPressed())
			{
			IsHalfBrightness=0;
			MakeFastStrobe(LED_Amber);
			delay_ms(40);
			}
		//进行系统复位
		TriggerSoftwareReset();
		}
	//读取操作完毕，锁定flash	
	SetFlashState(0);
	}

//恢复到无极调光模式的最低电流
void RestoreToMinimumSysCurrent(void)	
	{
	char i;
	extern code ModeStrDef ModeSettings[ModeTotalDepth];
	SysCfg.RampCurrent=100;
	for(i=0;i<ModeTotalDepth;i++)if(ModeSettings[i].ModeIdx==Mode_Ramp)
			SysCfg.RampCurrent=ModeSettings[i].MinCurrent; //找到挡位数据中无极调光的挡位
	}

//保存无极调光配置
void SaveSysConfig(bit IsForceSave)
	{
	char i;
	SysROMImg SavedData;
	//解锁flash（CRC校验模块需要读取Flash所以需要解锁）
	SetFlashState(1);
  //开始进行数据构建
	SavedData.Data.SysConfig.Data.IsSystemLocked=SysMode==Operation_Locked?true:false;
	SavedData.Data.SysConfig.Data.LocatorCfg=SysCfg.LocatorCfg;
  SavedData.Data.SysConfig.Data.SysCurrent=SysCfg.RampCurrent;
	SavedData.Data.SysConfig.Data.IsRampEnabled=IsRampEnabled?true:false;
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
