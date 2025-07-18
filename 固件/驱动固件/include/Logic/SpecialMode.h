#ifndef _SPMode_
#define _SPMode_

#include "ModeControl.h"
#include "LEDMgmt.h"

//特殊操作枚举
typedef enum
	{
	Operation_Normal=0, //正常操作
	Operation_Locked=1, //锁定模式
	Operation_TacTurbo=2, //战术模式(极亮)
	Operation_TacStrobe=3, //锁定模式
	}SpecialOperationDef;	

//外部引用
extern SpecialOperationDef SysMode; //特殊功能
extern bit IsDisplayLocked; //显示锁定
	
//函数
void PowerToNormalMode(ModeIdxDef Mode);//开启到普通模式
void TryEnterTurboStrobeProcess(char Count);	//尝试进入极亮和爆闪的实际内部处理
SpecialOperationDef SpecialModeOperation(char Click);//特殊功能切换的处理
void EnterMoonProcess(void); //进入月光模式的处理
	
#endif
