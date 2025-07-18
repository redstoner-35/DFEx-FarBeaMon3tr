#include "ModeControl.h"
#include "Strobe.h"
#include "ADCCfg.h"

//外部频闪Flag
extern volatile bit StrobeFlag;
extern volatile bit LFStrobeFlag;

//内部变量
static xdata char StrobeFlagSel;
static xdata unsigned char StrobeSelIdx; //爆闪选择index
static xdata unsigned char StrobeCounter; //爆闪次数计时

//内部爆闪事件顺序
#define RandomCodeDataAddr (volatile unsigned char code *)0x2E80

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
		//对爆闪flag进行处理
		if(StrobeSelIdx&0x28)StrobeFlagSel=(Data.RandADResult>>StrobeFlagSel)&0x03;
		else StrobeFlagSel++;
		StrobeFlagSel&=0x03;
		}
	}
	
//爆闪Flag输出处理
bit StrobeOutputHandler(void)
	{
	//根据爆闪flag选择一组频率		
	switch(StrobeFlagSel)
		{
		case 1:return LFStrobeFlag;
		case 2:return (bit)Data.RandADResult^StrobeFlag;
		case 3:return (bit)Data.RandADResult&LFStrobeFlag;
		}
	//默认情况
	return StrobeFlag;
	}