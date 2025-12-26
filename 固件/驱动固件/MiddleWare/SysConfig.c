/****************************************************************************/
/** \file SysConfig.c
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个文件为中层设备驱动文件，负责实现驱动的设置参数的非易失性存
储并实现数据存储所需的安全功能（如校验、EEPROM磨损均衡等）

**	History:
				2025年12月26日 10:05 1.针对新增的四击设置项在ROM内添加对应的entry和支持
															 代码以便于实现四击功能选择的自动定义。
														 2.针对新增的四击设置项增加设置该功能的自动宏定义系
															 统便于处理参数。
														 
				2025年12月20日 Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "ModeControl.h"
#include "cms8s6990.h"
#include "stdbool.h"
#include "SysConfig.h"
#include "Strobe.h"
#include "Flash.h"
#include "SideKey.h"
#include "SpecialMode.h"
#include "delay.h"
#include "LEDMgmt.h"
#include "SysReset.h"
#include "DefaultConfiguration.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define') for Parameter Definition
****************************************************************************/

//MCU数据区Flash定义
#define	DataFlashLen 0x3FF  //CMS8S6990单片机的数据区有1KByte，寻址范围是0-3FF


//内部bit域数据存储的Mask定义
#define IsRampEnabled_MSK 0x01 //Ramp是否启用 bit1
#define IsLocked_MSK 0x02  //是否锁定 bit2
#define IsEnableMainMemory_MSK 0x04 //是否启用主挡位记忆 bit3
#define IsEnableSpecMemory_MSK 0x08 //是否启用特殊挡位记忆 bit4
#define PowerECOMode_MSK 0x10 //Power和ECO模式切换 bit5
#define StrobeMode_MSK 0x20  //切换随机变频爆闪和高频爆闪 bit6
#define QuadClickSel_MSK 0x40 //切换四击操作的选择bit bit7

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define') for Parameter Parsing
****************************************************************************/

//自动计算出根据Flash可用区域下可以放置的系统参数组数（这个动了就炸！别手贱！）
#define SysCfgGroupLen (DataFlashLen/sizeof(SysROMImg))-1   

#message " ****************************************************************************"
#message " *                     System Default Configuration                         *"
#message " ****************************************************************************"

//默认夜光颜色处理的自动定义（这个动了就炸！别手贱！）
#define Loc_OFF 0x10
#define Loc_Green 0x11
#define Loc_Yellow 0x12
#define Loc_Red 0x13

#if (Default_LocatorColor == Loc_OFF)
	#define SetLocLED() SysCfg.LocatorCfg=Locator_OFF
  #message " Active Locate LED: Disabled"
#elif (Default_LocatorColor == Loc_Green)
  #define SetLocLED() SysCfg.LocatorCfg=Locator_Green
  #message " Active Locate LED: Enabled with Green Color"
#elif (Default_LocatorColor == Loc_Red)
  #define SetLocLED() SysCfg.LocatorCfg=Locator_Red
  #message " Active Locate LED: Enabled with Red Color"
#elif (Default_LocatorColor == Loc_Yellow)
  #message " Active Locate LED: Enabled with Amber/ Color"
  #define SetLocLED() SysCfg.LocatorCfg=Locator_Amber
#else
	#error "Error 00E:Invalid Default Locator LED Configuration!"
#endif

//渐暗熄灭处理的自动定义（这个动了就炸！别手贱！）
#define BulbEmu_OFF 0x20
#define BulbEmu_Fast 0x21
#define BulbEmu_Mid 0x22
#define BulbEmu_Slow 0x23

#if (Default_IncandescentBulbEmu == BulbEmu_OFF)
	#define SetFading() SysCfg.FadingCfg=Fading_OFF
	#message " Incandescent Bulb Emulation : Disabled"
#elif (Default_IncandescentBulbEmu == BulbEmu_Slow)
	#define SetFading() SysCfg.FadingCfg=Fading_Enable_Slow 
	#message " Incandescent Bulb Emulation : Enabled with Slow Fading Speed"
#elif (Default_IncandescentBulbEmu == BulbEmu_Mid)	
	#define SetFading() SysCfg.FadingCfg=Fading_Enable_Mid
	#message " Incandescent Bulb Emulation : Enabled with middle Fading Speed"
#elif (Default_IncandescentBulbEmu == BulbEmu_Fast)		
	#message " Incandescent Bulb Emulation : Enabled with Fast Fading Speed"
	#define SetFading() SysCfg.FadingCfg=Fading_Enable_Fast
#else
	#error "Error 00F:Invalid Default Incandescent Bulb Emulation Configuration!"
#endif

//系统默认工作模式的配置自动定义（这个动了就炸！别手贱！）
#define RegMode_5Step 0x31
#define RegMode_Ramp 0x32

#if (Default_RegMode == RegMode_5Step)
  #define SetRampBit() IsRampEnabled=0
	#message "Regular Mode Group : 5-Mode Step Loop(0.2-1-2-4-8.5A)"
#elif(Default_RegMode == RegMode_Ramp)
  #define SetRampBit() IsRampEnabled=1
	#message "Regular Mode Group : StepLess Adjust from 0.2A to 10A"
#else
	#error "Error 010:Invalid Default Regular Mode Group Configuration!"
#endif

//配置系统挡位记忆功能的参数自动定义（这个动了就炸！别手贱！）
#define ModeMemory_OFF 0x40
#define ModeMemory_MainOnly 0x41
#define ModeMemory_SpecOnly 0x42
#define ModeMemory_All 0x43

#if (Default_ModeMemory == ModeMemory_OFF)
    #define SetModeMem() do{IsMainMemEnabled=0;IsSpecMemEnabled=0;}while(0)
		#message "Mode Memory : Disabled"
#elif (Default_ModeMemory == ModeMemory_MainOnly)	
    #define SetModeMem() do{IsMainMemEnabled=1;IsSpecMemEnabled=0;}while(0)
		#message "Mode Memory : Regular Mode Only"
#elif (Default_ModeMemory == ModeMemory_SpecOnly)	
    #define SetModeMem() do{IsMainMemEnabled=0;IsSpecMemEnabled=1;}while(0)
		#message "Mode Memory : Special Mode Only"
#elif (Default_ModeMemory == ModeMemory_All)		
    #define SetModeMem() do{IsMainMemEnabled=1;IsSpecMemEnabled=1;}while(0)
		#message "Mode Memory : Both Special Mode and Regular Mode"		
#else
	#error "Error 011:Invalid Default Mode Memory Configuration!"
#endif
	
//配置系统挡位记忆功能的参数自动定义（这个动了就炸！别手贱！）	
#define StrobeMode_RandFreq 0x50
#define StrobeMode_Fix16Hz 0x51	

#if (Default_StrobeMode == StrobeMode_RandFreq)
		#define SetStrobeMode() EnableRandomStrobe=1
		#message "Strobe Mode : Random Frequency and Duty Cycle"
#elif (Default_StrobeMode == StrobeMode_Fix16Hz)
		#define SetStrobeMode() EnableRandomStrobe=0
		#message "Strobe Mode : Fixed Frequency at 16Hz and 50% Duty Cycle"		
#else
	#error "Error 012:Invalid Strobe Mode Configuration!"
#endif
		
//配置系统极亮模式取向的参数自动定义（这个动了就炸！别手贱！）	
#define TurboProfile_PowerMode 0x60
#define TurboProfile_ECOMode 0x61

#if (Default_TurboProfile == TurboProfile_PowerMode)
		#define SetTurboProfile() IsPowerModeEnabled=1
		#message "Turbo Profile : Power Mode (Maximum Output with higher temperature)"
#elif (Default_TurboProfile == TurboProfile_ECOMode)	
		#define SetTurboProfile() IsPowerModeEnabled=0
		#message "Turbo Profile : ECO Mode (Reduced Output with optimal temperature)"
#else
	#error "Error 013:Invalid Turbo Profile Configuration!"
#endif

//配置系统四击后进入的模式的参数自动定义（这个动了就炸！别手贱！）	
#define QuadClickMode_TacMode 0x70
#define QuadClickMode_FuckDogMode 0x71

#if (Default_QuadClickMode == QuadClickMode_TacMode)
    #define SetQuadClickMode() QuadClickSel=0
		#message "Quad Click Config : Enter Tac Mode"
#elif (Default_QuadClickMode == QuadClickMode_FuckDogMode)
    #define SetQuadClickMode() QuadClickSel=1
		#message "Quad Click Config : Enter NightWalk+FuckDog Mode"
#else
	#error "Error 016:Invalid Quad Click Operation Configuration!"
#endif	

#message "****************************************************************************"

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

//存储类型声明
typedef struct
	{
	int RampCurrent;
  unsigned char BitfieldMem1;
	ShutdownFadingDef FadingCfg;
	LocatorLEDDef LocatorCfg;
	}SysStorDef;
	
typedef union
	{
	SysStorDef Data;
	char ByteBuf[sizeof(SysStorDef)];
	}SysDataUnion;

typedef struct
	{
	SysDataUnion SysConfig;
	char CheckSum;
	}SysROMImageDef;

typedef union
	{
	SysROMImageDef Data;
	char ByteBuf[sizeof(SysROMImageDef)];
	}SysROMImg;

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/
static xdata unsigned int CurrentIdx;
static xdata u8 CurrentCRC;

/****************************************************************************/
/*	Function implementation - local('static')
****************************************************************************/		
	
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
	UnlockFlash();
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
	//复位无极调光和系统锁定的存储值
	SysMode=Operation_Normal; 
	LoadMinimumRampCurrentToRAM();	
	//复位系统参数
	SetLocLED();
	SetFading();
	SetRampBit();
	SetModeMem();
	SetStrobeMode();
	SetTurboProfile();
	SetQuadClickMode();
	}	
	
//显示系统数据存在错误
static void ShowEPROMCorrupted(void)
	{
	unsigned char delay=0xFF;
	//读取操作完毕，锁定flash	
	LockFlash();
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
	
/****************************************************************************/
/*	Global Function implementation - decleared in header files with('extern')
*****************************************************************************/	
	
//尝试检测用户进行重置操作	
void ResetSysConfigToDefault(void)
	{
	unsigned char delay;
	//准备初始的系统设置
  PrepareFactoryDefaultCfg();
	//保存数据并显示状态
	SaveSysConfig(0); 
	LockFlash(); 						//写数据写成默认值并且锁定Flash
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
		EnableRandomStrobe=ROMData.Data.SysConfig.Data.BitfieldMem1&StrobeMode_MSK?1:0;
		QuadClickSel=ROMData.Data.SysConfig.Data.BitfieldMem1&QuadClickSel_MSK?1:0;
		SysMode=ROMData.Data.SysConfig.Data.BitfieldMem1&IsLocked_MSK?Operation_Locked:Operation_Normal;
		
		SysCfg.LocatorCfg=ROMData.Data.SysConfig.Data.LocatorCfg; 
		if(!IsMainMemEnabled)LoadMinimumRampCurrentToRAM();             //无记忆模式每次都读取最低电流
		else SysCfg.RampCurrent=ROMData.Data.SysConfig.Data.RampCurrent;
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
		SysMode=Operation_Locked; //首次重建数据跳转为锁定状态
		SaveSysConfig(1); //重建数据后立即保存参数
		ShowEPROMCorrupted(); //显示EEPROM损坏
		}
	//读取操作完毕，锁定flash	
	LockFlash();
	}

//恢复到无极调光模式的最低电流
void LoadMinimumRampCurrentToRAM(void)	
	{
	bool Result;
	ModeStrDef *Mode=FindTargetMode(Mode_Ramp,&Result);
	if(Result)SysCfg.RampCurrent=Mode->MinCurrent; //找到挡位数据中无极调光的挡位
	else SysCfg.RampCurrent=200; //默认恢复为200mA
	}

//保存无极调光配置
void SaveSysConfig(bit IsForceSave)
	{
	unsigned char i;
	unsigned char BFBuf=0;
	SysROMImg SavedData;
	//解锁flash（CRC校验模块需要读取Flash所以需要解锁）
	UnlockFlash();
  //开始进行数据构建
	if(SysMode==Operation_Locked)BFBuf|=IsLocked_MSK;			//是否锁定
	if(IsRampEnabled)BFBuf|=IsRampEnabled_MSK;						//是否开启无极调光
	if(IsMainMemEnabled)BFBuf|=IsEnableMainMemory_MSK;		//是否启用主挡位记忆
	if(IsSpecMemEnabled)BFBuf|=IsEnableSpecMemory_MSK;    //是否启用特殊功能挡位记忆
	if(IsPowerModeEnabled)BFBuf|=PowerECOMode_MSK;        //是否启用POWER模式
	if(EnableRandomStrobe)BFBuf|=StrobeMode_MSK;          //是否启用随机变频爆闪	
	if(QuadClickSel)BFBuf|=QuadClickSel_MSK;              //四击的模式选择
		
	SavedData.Data.SysConfig.Data.BitfieldMem1=BFBuf;
	SavedData.Data.SysConfig.Data.FadingCfg=SysCfg.FadingCfg;
	SavedData.Data.SysConfig.Data.LocatorCfg=SysCfg.LocatorCfg;
  SavedData.Data.SysConfig.Data.RampCurrent=SysCfg.RampCurrent;
	SavedData.Data.CheckSum=PEC8Check(SavedData.Data.SysConfig.ByteBuf,sizeof(SysStorDef)); //计算CRC
	//进行数据比对
	if(!IsForceSave&&SavedData.Data.CheckSum==CurrentCRC)
		{
		LockFlash(); //读取操作完毕，锁定flash	
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
	LockFlash();  //写入操作完毕，锁定flash	
	}	
/*********************************  End Of File  ************************************/
