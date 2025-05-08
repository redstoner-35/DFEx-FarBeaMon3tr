#include "GUI.h"
#include "Key.h"

//GUI里面进行整数递增和递减的操作函数
int IntIncDec(int ValueIN,int Min,int Max,int PerStep)
	{
	//单步递增
	if(KeyState.KeyEvent!=KeyEvent_None)
		{
		if(KeyState.KeyEvent==KeyEvent_Up)ValueIN+=PerStep;
    if(KeyState.KeyEvent==KeyEvent_Down)ValueIN-=PerStep;
		KeyState.KeyEvent=KeyEvent_None;
		} 
	//持续按住快速增减
	if(KeyState.IsDownHold||KeyState.IsUpHold)
		{			
		if(KeyState.IsDownHold)ValueIN-=PerStep;
		else if(KeyState.IsUpHold)ValueIN+=PerStep;	
		}
	//数值限幅并返回原有的数值
	if(ValueIN>Max)ValueIN=Max;
	if(ValueIN<Min)ValueIN=Min;
	return ValueIN;
	}

	
//GUI里面进行浮点数递增和递减的操作函数
float FloatIncDec(float ValueIN,float Min,float Max,float PerStep)
	{
	//单步递增
	if(KeyState.KeyEvent!=KeyEvent_None)
		{
		if(KeyState.KeyEvent==KeyEvent_Up)ValueIN+=PerStep;
    if(KeyState.KeyEvent==KeyEvent_Down)ValueIN-=PerStep;
		KeyState.KeyEvent=KeyEvent_None;
		} 
	//持续按住快速增减
	if(KeyState.IsDownHold||KeyState.IsUpHold)
		{
		if(KeyState.IsDownHold)ValueIN-=PerStep;
		else if(KeyState.IsUpHold)ValueIN+=PerStep;	
		}
	//数值限幅并返回原有的数值
	if(ValueIN>Max)ValueIN=Max;
	if(ValueIN<Min)ValueIN=Min;
	return ValueIN;
	}	
