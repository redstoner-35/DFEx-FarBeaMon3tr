#include "ModeControl.h"
#include "Strobe.h"
#include "ADCCfg.h"

//外部频闪Flag
extern volatile bit StrobeFlag;
extern volatile bit LFStrobeFlag;

//内部变量
static bit StrobeFlagSel;
static xdata unsigned char StrobeSelIdx; //爆闪选择index
static xdata unsigned char StrobeCounter; //爆闪次数计时

//内部爆闪事件顺序
static code char StrobeSeq[]={1,9,8,5,2,3,7,4,6};

//爆闪控制器复位
void ResetStrobeModule(void)
	{
	StrobeFlagSel=0;
	StrobeSelIdx=0;
	StrobeCounter=0;
	}

//爆闪状态机处理
void RandStrobeHandler(void)
	{
	int IdxCalc;
	if(StrobeCounter)StrobeCounter--;
	else
		{
		//装载计数值
		StrobeCounter=StrobeSeq[StrobeSelIdx];
		//调用ADC传过来的随机AD值进行处理
		IdxCalc=Data.RandADResult^(int)StrobeCounter;
		IdxCalc>>=(Data.RandADResult^StrobeSelIdx)&0x07;	
		StrobeSelIdx=(unsigned char)(IdxCalc%sizeof(StrobeSeq));
		//取反爆闪flag
		StrobeFlagSel=~StrobeFlagSel;
		}
	}
	
//爆闪Flag输出处理
bit StrobeOutputHandler(void)
	{
	//根据爆闪flag选择一组频率
	return StrobeFlagSel?StrobeFlag:LFStrobeFlag;
	}