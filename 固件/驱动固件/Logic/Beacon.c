#include "Beacon.h"

//内部标志位
static xdata BeaconStateDef State;
static xdata unsigned char BeaconOnTIM;
static xdata unsigned char BeaconOffTIM;

//复位状态机
void BeaconFSM_Reset(void)
	{
	State=BeaconState_InfoUser;
	BeaconOnTIM=0;
	BeaconOffTIM=8*BeaconInfoTime;
	}

//关闭时间计时
void BeaconFSM_TIMHandler(void)
	{
	if(BeaconOffTIM)BeaconOffTIM--;
	}

//信标模式状态机
char BeaconFSM(void)
	{
	switch(State)
		{
		case BeaconState_InfoUser:	
			if(BeaconOffTIM>0)return 2; //当前处于提示状态，提示用户
		  //提示时间到，跳转到OFF阶段准备开始显示
		  BeaconOffTIM=BeaconOFFTime*8;
		  State=BeaconState_OFFWait;
			break;
		//初始化
		case BeaconState_Init:
			BeaconOnTIM=BeaconOnTime;
		  State=BeaconState_ONStrobe;
      return 1;
		//等待阶段
		case BeaconState_ONStrobe:
			BeaconOnTIM--;
			if(BeaconOnTIM>0)return 1;
	    //点亮时间到，熄灭
		  BeaconOffTIM=BeaconOFFTime*8;
		  State=BeaconState_OFFWait;
		  break;
		//等待熄灭阶段结束
		case BeaconState_OFFWait:
		  if(!BeaconOffTIM)State=BeaconState_Init;
		  break;
		}
	//其余状态返回0使LED熄灭
	return 0;
	}
