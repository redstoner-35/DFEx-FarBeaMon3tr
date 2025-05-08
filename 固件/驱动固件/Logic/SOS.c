#include "ModeControl.h"
#include "SOS.h"

//状态位
static xdata SOSStateDef SOSState; //全局变量状态位
static xdata char SOSTIM;  //SOS计时

//SOS状态机的跳转处理
static void SOSFSM_Jump(SOSStateDef State,char Time)
{
	if(SOSTIM)return; //显示未结束
	SOSTIM=Time; 
	SOSState=State;  //进入延时等待阶段
}

//SOS状态机的时序处理
void SOSTIMHandler(void)
{
	//对计时器数值进行递减
	if(SOSTIM>0)SOSTIM--;
}

//复位整个SOS模块
void ResetSOSModule(void)
{
	SOSState=SOSState_Prepare;
	SOSTIM=0;
}
//SOS定时器状态监测
static bit SOSTIMDetect(char Time)
{
//触发定时判断
if((SOSTIM%(Time*2))>(Time-1))return 1;
//关闭状态返回0
return 0;
}

//SOS状态机处理模块
char SOSFSM(void)
{
	switch(SOSState)
		{
		//准备阶段
		case SOSState_Prepare:
			 SOSTIM=0;
			 SOSFSM_Jump(SOSState_3Dot,(3*SOSDotTime*2)-1);
		   break;
		//第一和第二次三点
		case SOSState_3DotAgain:
		case SOSState_3Dot:
       if(SOSTIMDetect(SOSDotTime))return 1; //当前状态需要LED电流，返回1
			 if(SOSState==SOSState_3Dot)SOSFSM_Jump(SOSState_3DotWait,SOSGapTime);  //进入延时等待阶段
		   else SOSFSM_Jump(SOSState_Wait,SOSFinishGapTime);//进入延时等待阶段
		   break;
		//三点结束后的等待延时阶段
	  case SOSState_3DotWait:
			 SOSFSM_Jump(SOSState_3Dash,(3*SOSDashTime*2)-1);
		   break;
		//三划
		case SOSState_3Dash:
			 if(SOSTIMDetect(SOSDashTime))return 1; //当前状态需要LED电流，返回1	
		   SOSFSM_Jump(SOSState_3DashWait,SOSGapTime);
		   break;			
		//三划结束后的等待延时阶段
	  case SOSState_3DashWait:
			 SOSFSM_Jump(SOSState_3DotAgain,(3*SOSDotTime*2)-1);
		   break;		
	  //本轮信号发出完毕，等待
	  case SOSState_Wait:	
			 SOSFSM_Jump(SOSState_Prepare,0);//回到准备状态
		   break;
		}
	//其余情况返回0关闭防反接，确保尾按可以正确响应
	return 0;
}