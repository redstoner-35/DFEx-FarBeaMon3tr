/****************************************************************************/
/** \file Beacon.c
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个文件为顶层应用层逻辑文件。该文件实现了系统中瞬时信标爆闪的
信标脉冲闪的功能。

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "OutputChannel.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros - for Parameter Definition
****************************************************************************/
#define BeaconOnTime 60 	//信标闪烁时间
#define BeaconOFFTime 3 	//信标关闭时间(秒)
#define BeaconInfoTime 3 	//信标在开始之前低亮提示用户的时间(秒)

/****************************************************************************/
/*	Local type definitions('typedef')
*****************************************************************************/
typedef enum
	{
	BeaconState_Init,
	BeaconState_InfoUser,
	BeaconState_ONStrobe,
	BeaconState_OFFWait,
	}BeaconStateDef;
	
/****************************************************************************/
/*	Local variable and Flag definitions('static')
*****************************************************************************/
static xdata BeaconStateDef State;
static xdata unsigned char BeaconOnTIM;
static xdata unsigned char BeaconOffTIM;

/****************************************************************************/
/* Global Function implementation - Initialization
*****************************************************************************/	

void BeaconFSM_Reset(void)	//复位信标系统的状态机到最初状态
	{
	State=BeaconState_InfoUser;
	BeaconOnTIM=0;
	BeaconOffTIM=8*BeaconInfoTime;
	}

/****************************************************************************/
/* Global Function implementation - Logic & Timing Handler
*****************************************************************************/	
	
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
			//当前处于等待输出启动或者提示状态，提示用户
			if(!IsOutputStarted||BeaconOffTIM>0)return 2; 
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
/*********************************  End Of File  ************************************/
