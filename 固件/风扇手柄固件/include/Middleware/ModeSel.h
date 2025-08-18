#ifndef MSEL
#define MSEL

typedef enum
	{
	LVPROT_Disable=0,  //该挡位关闭低电量保护
	LVPROT_Enable_Jump=1, //该挡位低电量保护开启，当电量低于阈值后执行跳档
	LVPROT_Enable_OFF=2		//该挡位低电量保护开启，当电量低于阈值后立即执行关机
	}LVProtectTypeDef;	

typedef struct
	{
	float TurboMinimumDuty;		 //最低占空比
	float TurboMinimumVoltage; //最低电压
	float TurboCurrentVoltage; //当前电压
	float TurboCurrentDuty;  	 //当前占空比
	unsigned int FullSpeedTime;      	 //全速时间
  char TurboRefreshCount;        //极速睡眠刷新
	unsigned int TurboRefreshTIM; //极速睡眠刷新计时器
	}TurboTimedStepDownDef;	

typedef enum
	{
	Mode_OFF=0, //关机
  //常用四个循环挡位
	Mode_UltraLow,
	Mode_Low,
	Mode_Mid,
	Mode_High,
	//全功率爆发	
	Mode_Turbo,
	//无级调节
	Mode_Ramp
	}ModeIdxDef;	

typedef struct
	{
  ModeIdxDef ModeIdx;
	int LowVoltThres; //低电压检测电压(mV)
	unsigned char Speed; //风扇速度百分比(%)
  float TargetVOUT; //目标的输出电压(仅CV模式有效)		
	//低电量保护设置
  ModeIdxDef ModeWhenLVAutoFall;		//低电量触发保护之后，如果不执行关机则自动跳转的挡位
	LVProtectTypeDef LVConfig;        //低电量保护机制的类型
	//挡位切换设置
  ModeIdxDef ModeTargetWhenH;
	ModeIdxDef ModeTargetWhen1H;	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位
	}ModeStrDef; 

//参数配置
#define PWMHoldSwitchDelay 14 //PWM模式换挡延迟
#define HoldSwitchDelay 6 // 长按换挡延迟	
#define SleepTimeOut 5 //休眠状态延时	
#define ModeTotalDepth 7 //系统一共有几个挡位		
#define TurboMaintainTime 100 //极速挡位下维持全速的时间（单位	S）	
#define MaxTurboRefreshCount 6  //极速最大允许的强制刷新降档次数
#define TurboRefreshCountCD 100 //极速挡位强制刷新次数的补充时间（单位S）	
	
//外部引用
extern ModeStrDef *CurrentMode; //当前模式结构体
extern xdata ModeIdxDef LastMode; //上一个挡位	
	
//函数
void AddTurboRefreshCountWhenSleep(void); //睡眠过程中定时唤醒补充极亮强制刷新次数
void TurboTimedStepDownPROC(void); //极速挡位时控降档处理
void HoldSwitchGearCmdHandler(void); //长按换挡处理
void SwitchToGear(ModeIdxDef TargetMode); //换到指定挡位	
void ModeFSMInit(void); //初始化模式状态机		
void ReturnToOFFState(void); //长按关机函数
void ModeSwitchFSM(void); //挡位状态机	
	
#endif
