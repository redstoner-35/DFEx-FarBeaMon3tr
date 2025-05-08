#ifndef _SOS_
#define _SOS_

//SOS状态枚举
typedef enum
	{
	SOSState_Prepare,
	SOSState_3Dot,
	SOSState_3DotWait,
	SOSState_3Dash,
	SOSState_3DashWait,
	SOSState_3DotAgain,
	SOSState_Wait,
	}SOSStateDef;	

//SOS时序配置
#define SOSDotTime 2 //SOS信号(.)的时间	
#define SOSDashTime 6 //SOS信号(-)的时间	
#define SOSGapTime 7 //SOS信号在每次显示途中等待的时间
#define SOSFinishGapTime 35 //每轮SOS发出结束后的等待时间	

//函数
char SOSFSM(void);	//SOS状态机处理模块
void SOSTIMHandler(void);//SOS状态机的时序处理
void ResetSOSModule(void);	//复位整个SOS模块
	
#endif