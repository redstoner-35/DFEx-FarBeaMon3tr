#include "lcd.h"
#include "GUI.h"
#include "Key.h"
#include <stdlib.h>

static bool IsINTEDMNUpdate=false;
static char MinusHold=0;
static char PlusHold=0;
static bool WaitForKeyRelease=false;

//减按钮
static void DisplayMinusButton(bool IsSelect)
	{
	LCD_DrawRectangle(18,26,35,43,BLACK);
	LCD_DrawRectangle(19,27,34,42,DARKBLUE);	
	LCD_Fill(20,28,33,41,IsSelect?LGRAY:LGRAYBLUE); //被选中时换色
	LCD_ShowChar(23,29,'-',IsSelect?WHITE:LBBLUE,IsSelect?LGRAY:LGRAYBLUE,12,0); //显示字符
	}

//加按钮
static void DisplayPlusButton(bool IsSelect)
	{
	LCD_DrawRectangle(123,26,140,43,BLACK);
	LCD_DrawRectangle(124,27,139,42,DARKBLUE);	
	LCD_Fill(125,28,138,41,IsSelect?LGRAY:LGRAYBLUE); //被选中时换色
	LCD_ShowChar(129,29,'+',IsSelect?WHITE:LBBLUE,IsSelect?LGRAY:LGRAYBLUE,12,0); //显示字符
	}
	
//实现菜单按键按下特效的函数
void IntEditMenuKeyEffHandler(void)
	{
	//向上键按下
	if(PlusHold>0)
		{
		if(PlusHold==1)IsINTEDMNUpdate=false; //按键松开动画到时间了重新渲染
		PlusHold--;
		MinusHold=0;
		}
	//向下键按下
	if(MinusHold>0)
		{
		if(MinusHold==1)IsINTEDMNUpdate=false; //按键松开动画到时间了重新渲染
		MinusHold--;
		PlusHold=0;
		}
	}	

//整数编辑进入时的处理
void IntEditInitHandler(void)
	{
	MinusHold=0;
	PlusHold=0;
	IsINTEDMNUpdate=false;
	WaitForKeyRelease=true;
	}	
	
//整数编辑菜单
void IntEditHandler(const intEditMenuCfg *CFG)
	{
	int pos;
	float direct;
	bool IsKeyHold=KeyState.IsUpHold|KeyState.IsDownHold;		
  if(!WaitForKeyRelease&&IsKeyHold&&KeyState.KeyEvent!=KeyEvent_BothEnt)
		{
		pos=IntIncDec(*(CFG->Source),CFG->min,CFG->max,CFG->Step);
		if(pos>*(CFG->Source))PlusHold=4;
		else if(pos<*(CFG->Source))MinusHold=4; //按键反复按下发生数值更改	
		*(CFG->Source)=pos; //将输出的数值保存下来
		DisplayPlusButton(PlusHold?true:false);	
		DisplayMinusButton(MinusHold?true:false);
		//显示字符
		LCD_Fill(45,28,84,41,LGRAY);
		LCD_ShowIntNum(47,29,*(CFG->Source),*(CFG->Source)>9999?5:4,CYAN,LGRAY,12);	
		//显示进度条的Bar		
		LCD_Fill(30,56,130,60,LGRAY);
		LCD_Fill(30,64,130,68,LGRAY);
		LCD_DrawRectangle(30,60,129,63,BLACK);
		LCD_Fill(32,61,128,62,LGRAYBLUE);
		for(pos=61;pos<129;pos+=5)LCD_DrawLine(pos,61,pos,62,YELLOW);
	  //计算进度条游标位置	
		direct=(float)(*CFG->Source)-(float)(CFG->min);	 //VDelta=VIN-Vmin
		direct/=(float)(CFG->max-CFG->min); //百分比=Vdelta/Vmax-Vmin
		direct*=(float)96; //乘以游标最大的位移量96格得到游标的位移值
		pos=(int)direct;
		if(pos<0)pos=0;
		if(pos>96)pos=96; //限制游标的位移距离不能超过96
		//显示进度条位置
		LCD_DrawRectangle(30+pos,56,34+pos,67,BLACK);
		LCD_Fill(31+pos,57,33+pos,66,WHITE);			
		return;	
		}
	if(IsINTEDMNUpdate&&KeyState.KeyEvent==KeyEvent_None)return;//不执行
	RenderMenuBG();
	//显示左边的字符和数值
	DisplayMinusButton(MinusHold?true:false);
	LCD_ShowIntNum(47,29,*(CFG->Source),*(CFG->Source)>9999?5:4,CYAN,LGRAY,12);
	LCD_ShowHybridString(96,29,CFG->Unit,CYAN,LGRAY,0);	
	//显示选项框背景
	LCD_DrawRectangle(43,26,115,43,BLACK);
	LCD_DrawRectangle(44,27,114,42,DARKBLUE);
	//显示加按钮
	DisplayPlusButton(PlusHold?true:false);
	//显示左边的文字
	LCD_ShowHybridString(3,56,CFG->MinName,WHITE,LGRAY,0);
	//显示进度条的Bar		
	LCD_DrawRectangle(30,60,129,63,BLACK);
	LCD_Fill(32,61,128,62,LGRAYBLUE);
	for(pos=61;pos<129;pos+=5)LCD_DrawLine(pos,61,pos,62,YELLOW);
	//计算进度条游标位置	
	direct=(float)(*CFG->Source)-(float)(CFG->min);	 //VDelta=VIN-Vmin
  direct/=(float)(CFG->max-CFG->min); //百分比=Vdelta/Vmax-Vmin
	direct*=(float)96; //乘以游标最大的位移量96格得到游标的位移值
  pos=(int)direct;
  if(pos<0)pos=0;
  if(pos>96)pos=96; //限制游标的位移距离不能超过96
	//显示进度条位置
	LCD_DrawRectangle(30+pos,56,34+pos,67,BLACK);
	LCD_Fill(31+pos,57,33+pos,66,WHITE);
	//显示右边的文字
	LCD_ShowHybridString(132,56,CFG->MaxName,WHITE,LGRAY,0);
	//按键处理
  if(WaitForKeyRelease)
		{
		IsINTEDMNUpdate=true;
		//进入菜单之后要等待用户按下
		if(IsKeyHold)return;
		WaitForKeyRelease=0;
		return;
		}
	if(KeyState.KeyEvent==KeyEvent_BothEnt&&CFG->ThingsToDoWhenExit!=NULL)
		{
		KeyState.KeyEvent=KeyEvent_None; //消除按键事件
		CFG->ThingsToDoWhenExit(); //用户同时按下上下结束数值编辑
	  return;
		}
	pos=IntIncDec(*(CFG->Source),CFG->min,CFG->max,CFG->Step);
	if(pos>*(CFG->Source))PlusHold=4;
	else if(pos<*(CFG->Source))MinusHold=4; //按键反复按下发生数值更改
	else IsINTEDMNUpdate=true; //数值没有发生变更，指示界面更新已完成
	*(CFG->Source)=pos; //将输出的数值保存下来
	}
