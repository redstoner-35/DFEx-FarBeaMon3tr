#include "GUI.h"
#include "Key.h"

//GUI����������������͵ݼ��Ĳ�������
int IntIncDec(int ValueIN,int Min,int Max,int PerStep)
	{
	//��������
	if(KeyState.KeyEvent!=KeyEvent_None)
		{
		if(KeyState.KeyEvent==KeyEvent_Up)ValueIN+=PerStep;
    if(KeyState.KeyEvent==KeyEvent_Down)ValueIN-=PerStep;
		KeyState.KeyEvent=KeyEvent_None;
		} 
	//������ס��������
	if(KeyState.IsDownHold||KeyState.IsUpHold)
		{			
		if(KeyState.IsDownHold)ValueIN-=PerStep;
		else if(KeyState.IsUpHold)ValueIN+=PerStep;	
		}
	//��ֵ�޷�������ԭ�е���ֵ
	if(ValueIN>Max)ValueIN=Max;
	if(ValueIN<Min)ValueIN=Min;
	return ValueIN;
	}

	
//GUI������и����������͵ݼ��Ĳ�������
float FloatIncDec(float ValueIN,float Min,float Max,float PerStep)
	{
	//��������
	if(KeyState.KeyEvent!=KeyEvent_None)
		{
		if(KeyState.KeyEvent==KeyEvent_Up)ValueIN+=PerStep;
    if(KeyState.KeyEvent==KeyEvent_Down)ValueIN-=PerStep;
		KeyState.KeyEvent=KeyEvent_None;
		} 
	//������ס��������
	if(KeyState.IsDownHold||KeyState.IsUpHold)
		{
		if(KeyState.IsDownHold)ValueIN-=PerStep;
		else if(KeyState.IsUpHold)ValueIN+=PerStep;	
		}
	//��ֵ�޷�������ԭ�е���ֵ
	if(ValueIN>Max)ValueIN=Max;
	if(ValueIN<Min)ValueIN=Min;
	return ValueIN;
	}	
