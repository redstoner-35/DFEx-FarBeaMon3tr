#ifndef _ModeControl_
#define _ModeControl_

#include "stdbool.h"
//定位LED设置
typedef enum
	{
	Locator_OFF=0, //定位关闭
	Locator_Green=1, //绿灯
	Locator_Red=2, //红灯
	Locator_Amber=3, //黄灯
	}LocatorLEDDef;	

typedef struct
	{
	int RampCurrent;
	int RampBattThres;
	int RampCurrentLimit;
	char RampLimitReachDisplayTIM;
	char CfgSavedTIM;
	LocatorLEDDef LocatorCfg;
	}SysConfigDef;	
	
typedef enum
	{
	Mode_OFF=0, //关机
	Mode_Fault, //出现错误
		
	Mode_Ramp, //无极调光
	Mode_1Lumen, //1流明极低挡位
  Mode_Moon, //月光
	Mode_ExtremelyLow, //极低亮
	Mode_Low, //低亮
	Mode_Mid, //中亮
	Mode_MHigh,   //中高亮
	Mode_High,   //高亮
		
	Mode_Turbo, //极亮
	//特殊模式
  Mode_Beacon, //信标挡位 		
  Mode_Strobe, //爆闪		
	Mode_SOS, //SOS挡位
	}ModeIdxDef;
	

typedef struct
	{
  ModeIdxDef ModeIdx;
  int Current; //挡位电流(mA)
	int MinCurrent; //最小电流(mA)，仅无极调光需要
	int LowVoltThres; //低电压检测电压(mV)
	bool IsModeHasMemory; //是否带记忆
	bool IsNeedStepDown; //是否需要降档
	}ModeStrDef; 

//外部引用
extern xdata char DisplayLockedTIM; //锁定提示计时器
extern ModeStrDef *CurrentMode; //当前模式结构体
extern xdata ModeIdxDef LastMode; //上一个挡位	
extern SysConfigDef SysCfg; //无极调光配置	
extern bit IsRampEnabled; //是否启用无极调光	
	
//特殊宏定义
#define QueryCurrentGearILED() CurrentMode->Current //获取当前挡位的电流函数
	
//参数配置
#define HoldSwitchDelay 6 // 长按换挡延迟	
#define SleepTimeOut 5 //休眠状态延时	
#define ModeTotalDepth 14 //系统一共有几个挡位			
	
//函数
void ModeFSMTIMHandler(void);//挡位状态机所需的软件定时器处理
void ModeSwitchFSM();//挡位状态机	
void SwitchToGear(ModeIdxDef TargetMode);//换到指定挡位
void ReturnToOFFState(void);//关机	
void HoldSwitchGearCmdHandler(void); //换挡间隔生成	
void ModeFSMInit(void); //初始化状态机	

#endif
