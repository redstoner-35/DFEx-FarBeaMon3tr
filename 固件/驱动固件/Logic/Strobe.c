/****************************************************************************/
/** \file Strobe.c
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个文件为顶层应用层逻辑文件。该文件实现了系统中三击爆闪挡位的
高频爆闪和随机变频脉冲闪的功能。

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "ModeControl.h"
#include "OutputChannel.h"
#include "ADCCfg.h"
#include "delay.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros - for Parameter Definition
****************************************************************************/
//内部爆闪事件顺序(这个千万不能乱改，会炸的！)
#define RandomCodeDataAddr (volatile unsigned char code *)0x2E80


/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/
bit EnableRandomStrobe;                  //系统配置位，是否开启随机变频爆闪

/****************************************************************************/
/*	Local variable and Flag definitions('static')
*****************************************************************************/
static xdata char StrobeFlagSel;
static xdata unsigned char StrobeSelIdx; 	//爆闪选择index
static xdata unsigned char StrobeCounter; //爆闪次数计时


/****************************************************************************/
/* Global Function implementation - Initialization
*****************************************************************************/	

void ResetStrobeModule(void)	//爆闪控制器复位函数
	{
	StrobeFlagSel=0;
	StrobeSelIdx=0;
	StrobeCounter=0;
	}

/****************************************************************************/
/* Global Function implementation - Logic & Strobe Output Handler
*****************************************************************************/		
	
//爆闪状态机处理
void RandStrobeHandler(void)
	{
	int IdxCalc;
	volatile unsigned char code *RandData=RandomCodeDataAddr;
	//进行随机模块重装载
	if(StrobeCounter)StrobeCounter--;
	else
		{
		//装载计数值
		RandData=RandomCodeDataAddr;                //取地址
		StrobeCounter=RandData[StrobeSelIdx]&0x0F;
		//调用ADC传过来的随机AD值进行处理
		IdxCalc=Data.RandADResult^(int)StrobeCounter;
		IdxCalc>>=(Data.RandADResult^StrobeSelIdx)&0x07;	
		StrobeSelIdx=(unsigned char)(IdxCalc&0x7F);
		//对爆闪flag的数值进行赋值
		if(StrobeSelIdx&0x28)
	     StrobeFlagSel=Data.RandADResult>>StrobeFlagSel;  //使用随机结果作为Strbobe Result
		else 
	    StrobeFlagSel++;                                  //线性递增结果
		//因为爆闪flag允许的范围是0-3，进行限幅
		StrobeFlagSel&=0x03;
		}
	}
	
//爆闪Flag输出处理
bit StrobeOutputHandler(void)
	{
	//系统还未完成启动，此时禁止爆闪模块运行确保系统已经完成LED预偏置再启动
	if(!IsOutputStarted)return 1;	
	//根据爆闪flag选择一组频率		
	if(EnableRandomStrobe)switch(StrobeFlagSel)
		{
		case 1:return LFStrobeFlag;
		case 2:return (bit)Data.RandADResult^StrobeFlag;
		case 3:
		return (bit)Data.RandADResult&LFStrobeFlag;
		}
	//默认情况
	return StrobeFlag;
	}
/*********************************  End Of File  ************************************/
