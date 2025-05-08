#include "ModeControl.h"
#include "Strobe.h"

//外部频闪Flag
extern volatile bit StrobeFlag;
extern volatile bit LFStrobeFlag;

//内部变量
static bit StrobeFlagSel;
static xdata char StrobeSelIdx; //爆闪选择index
static xdata char StrobeCounter; //爆闪次数计时

//内部爆闪事件顺序
static code char StrobeSeq[]={1,3,10,9,7,8,5,11,6,4};

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
	if(StrobeCounter)StrobeCounter--;
	else
		{
		//装载计数值
		StrobeCounter=StrobeSeq[StrobeSelIdx];
		StrobeSelIdx=StrobeCounter%sizeof(StrobeSeq);
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