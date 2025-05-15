#ifndef _OC_
#define _OC_

//输出通道状态机
typedef enum
	{
	//输出通道彻底关闭，待机状态
	OutCH_Standby,
	//输出通道正常启动流程
	OutCH_PWMDACPreCharge,  //PWMDAC预偏置
	OutCH_StartAUXPSU, //启动辅助PSU
	OutCH_EnableBoost, //启动主Boost
	OutCH_ReleasePreCharge, //逐步复位PreCharge DAC让输出慢慢SysUp
	OutCH_SubmitDuty, //应用占空比
	//输出通道正常运行阶段
	OutCH_OutputEnabled,
	//安全关闭阶段
	OutCH_GracefulShut,
	OutCH_WaitVOUTDecay,
	//输出通道在爆闪等阶段进入idle(LED断开，输出配置为14.7V)
	OutCH_EnterIdle,	
	OutCH_OutputIdle,
  //输出通道启动失败
	OutCH_PreChargeFailed
	}OutChFSMStateDef;

//输出通道参数设置
#define MainChannelShuntmOhm 1.00 //主通道的检流电阻阻值(mR)
#define CurrentSenseOpAmpGain 100 //电流检测放大器的增益

//输出通道电流参考宏	
#define CalcIREFValue(x) ((x/2)+(x/6))
	
	
//外部参考
extern xdata volatile int Current; //电流值
extern xdata int CurrentBuf; //当前已应用的电流值
extern bit IsCurrentRampUp;  //电流正在上升过程中的标记位（用于和MPPT试探联动）
	
//函数
void SetHBLEDState(bit State); //设置心跳LED
void OutputChannel_Init(void);
void OutputChannel_Calc(void);
void OCFSM_TIMHandler(void);
void OutputChannel_TestRun(void); //输出通道试运行
void OutputChannel_DeInit(void); //输出通道复位
bit GetIfOutputEnabled(void);		//外部获取输出是否正常启用的函数
bit GetIfSystemInPOFFSeq(void);	//获取系统是否在安全关机阶段
	
#endif
