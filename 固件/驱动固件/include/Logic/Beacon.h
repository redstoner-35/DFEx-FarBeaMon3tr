#ifndef _BCON_
#define _BCON_

typedef enum
	{
	BeaconState_Init,
	BeaconState_InfoUser,
	BeaconState_ONStrobe,
	BeaconState_OFFWait,
	}BeaconStateDef;

//参数
#define BeaconOnTime 55 //信标闪烁时间
#define BeaconOFFTime 2 //信标关闭时间
#define BeaconInfoTime 3 //信标在开始之前低亮提示用户的时间
	
//函数
void BeaconFSM_Reset(void); //复位状态机
void BeaconFSM_TIMHandler(void); //关闭时间计时
char BeaconFSM(void); //信标模式状态机

#endif
