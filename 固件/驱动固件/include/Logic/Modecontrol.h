/************************************************************************************/
/** \file ModeControl.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个头文件负责声明部分的配置参数、挡位切换系统的初始化和命令函数以及部分
挡位系统输出给其余模块的状态全局变量。

**	History: Initial Release
**	
/*************************************************************************************/
#ifndef _ModeControl_
#define _ModeControl_
/*************************************************************************************/
/*	Include Files
**************************************************************************************/
#include "stdbool.h"
/*************************************************************************************/
/*	Global type definitions('typedef')
**************************************************************************************/

typedef enum	//定位LED设置
	{
	Locator_OFF=0, //定位关闭
	Locator_Green=1, //绿灯
	Locator_Red=2, //红灯
	Locator_Amber=3, //黄灯
	}LocatorLEDDef;	

typedef enum
	{
	LVPROT_Disable=0,  //该挡位关闭低电量保护
	LVPROT_Enable_Jump=1, //该挡位低电量保护开启，当电量低于阈值后执行跳档
	LVPROT_Enable_OFF=2		//该挡位低电量保护开启，当电量低于阈值后立即执行关机
	}LVProtectTypeDef;	
	
//衰减速度设置
typedef enum
	{
	Fading_OFF=0, //关机时禁止拖尾
	Fading_Enable_Slow=3,
	Fading_Enable_Mid=2,
	Fading_Enable_Fast=1  //三档拖尾速度
	}ShutdownFadingDef;	
	
typedef struct
	{
	int RampCurrent;
	int RampBattThres;
	int RampCurrentLimit;
	unsigned char RampLimitReachDisplayTIM;
	unsigned char CfgSavedTIM;
	LocatorLEDDef LocatorCfg;
	ShutdownFadingDef FadingCfg;
	}SysConfigDef;	
	
typedef enum
	{
	Mode_OFF=0, //关机
	Mode_Fault=1, //出现错误
	//极低LM挡位	
	Mode_1Lumen=2, //1流明极低挡位
  Mode_Moon=3, //月光	

	//正常挡位
	Mode_Ramp=4, //无极调光
	Mode_ExtremelyLow=5, //极低亮
	Mode_Low=6, //低亮
	Mode_Mid=7, //中亮
	Mode_MHigh=8,   //中高亮
	Mode_High=9,   //高亮
		
	Mode_Turbo=10, //极亮
	//特殊模式
  Mode_Beacon=11, //信标挡位 		
  Mode_Strobe=12, //爆闪		
	Mode_SOS=13, //SOS挡位
	}ModeIdxDef;
	

typedef struct
	{
  ModeIdxDef ModeIdx;
  int Current; //挡位电流(mA)
	int MinCurrent; //最小电流(mA)，仅无极调光需要
	int LowVoltThres; //低电压检测电压(mV)
	bool IsModeHasMemory; //是否带记忆
	bool IsNeedStepDown; //是否需要降档
	//是否允许进入极亮和爆闪
	bool IsEnterTurboStrobe; 
	//低电量保护设置
  ModeIdxDef ModeWhenLVAutoFall;		//低电量触发保护之后，如果不执行关机则自动跳转的挡位
	LVProtectTypeDef LVConfig;        //低电量保护机制的类型
	//挡位切换设置
  ModeIdxDef ModeTargetWhenH;
	ModeIdxDef ModeTargetWhen1H;	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位
	}ModeStrDef; 

/************************************************************************************/
/* Extern Flags and Variable definition - Exported System Configuration */
/************************************************************************************/
extern bit IsMainMemEnabled; //是否开启主挡位记忆
extern bit IsSpecMemEnabled; //是否开启特殊挡位记忆	
extern bit IsPowerModeEnabled; //功率模式是否开启	
extern bit IsRampEnabled; //是否启用无极调光		
	
/************************************************************************************/
/* Extern Flags and Variable definition - Mode Switching Related */
/************************************************************************************/	
extern bit IsStrobePoweredFromOFF; //是否从关机状态直接一键爆闪		
extern ModeStrDef *CurrentMode; //当前模式结构体
extern xdata ModeIdxDef LastSpecialMode; //特殊功能挡位
extern xdata ModeIdxDef LastMode; //上一个挡位	
extern SysConfigDef SysCfg; //无极调光配置	
extern xdata unsigned char DisplayLockedTIM; //锁定提示计时器
	
/************************************************************************************/
/* Extern Functions definition - Initialization */
/************************************************************************************/	
void ModeFSMInit(void); //初始化状态机		
	
/************************************************************************************/
/* Extern Functions definition - Mode Switching Command Related */
/************************************************************************************/	

//输入指定的Index，从index里面找到目标模式结构体并返回指针
ModeStrDef *FindTargetMode(ModeIdxDef Mode,bool *IsResultOK);
void SwitchToGear(ModeIdxDef TargetMode);	//换到指定挡位	
void ReturnToOFFState(void);							//关机		

/************************************************************************************/
/* Extern Functions definition - Mode Switching Logic and Timing Handler */
/************************************************************************************/	
void ModeFSMTIMHandler(void);//挡位状态机所需的软件定时器处理
void ModeSwitchFSM();//挡位状态机	
void HoldSwitchGearCmdHandler(void); //换挡间隔生成	

/************************************************************************************/
/* Extern Functions definition - Current Query */
/************************************************************************************/	

//获取系统挡位在没有任何外部影响情况下的全部电流
int QuerySystemFullScaleCurrent(void);	

/************************************************************************************/
/* Extern Fast Operation Macro definition */
/************************************************************************************/

//获取当前挡位电流的特殊宏
#define QueryCurrentGearILED() CurrentMode->Current 

#endif /* _ModeControl_ */

/*********************************  End Of File  ************************************/
