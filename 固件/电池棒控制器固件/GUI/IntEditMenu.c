#include "lcd.h"
#include "GUI.h"
#include "Key.h"
#include <stdlib.h>

static bool IsINTEDMNUpdate=false;
static char MinusHold=0;
static char PlusHold=0;
static bool WaitForKeyRelease=false;

//����ť
static void DisplayMinusButton(bool IsSelect)
	{
	LCD_DrawRectangle(18,26,35,43,BLACK);
	LCD_DrawRectangle(19,27,34,42,DARKBLUE);	
	LCD_Fill(20,28,33,41,IsSelect?LGRAY:LGRAYBLUE); //��ѡ��ʱ��ɫ
	LCD_ShowChar(23,29,'-',IsSelect?WHITE:LBBLUE,IsSelect?LGRAY:LGRAYBLUE,12,0); //��ʾ�ַ�
	}

//�Ӱ�ť
static void DisplayPlusButton(bool IsSelect)
	{
	LCD_DrawRectangle(123,26,140,43,BLACK);
	LCD_DrawRectangle(124,27,139,42,DARKBLUE);	
	LCD_Fill(125,28,138,41,IsSelect?LGRAY:LGRAYBLUE); //��ѡ��ʱ��ɫ
	LCD_ShowChar(129,29,'+',IsSelect?WHITE:LBBLUE,IsSelect?LGRAY:LGRAYBLUE,12,0); //��ʾ�ַ�
	}
	
//ʵ�ֲ˵�����������Ч�ĺ���
void IntEditMenuKeyEffHandler(void)
	{
	//���ϼ�����
	if(PlusHold>0)
		{
		if(PlusHold==1)IsINTEDMNUpdate=false; //�����ɿ�������ʱ����������Ⱦ
		PlusHold--;
		MinusHold=0;
		}
	//���¼�����
	if(MinusHold>0)
		{
		if(MinusHold==1)IsINTEDMNUpdate=false; //�����ɿ�������ʱ����������Ⱦ
		MinusHold--;
		PlusHold=0;
		}
	}	

//�����༭����ʱ�Ĵ���
void IntEditInitHandler(void)
	{
	MinusHold=0;
	PlusHold=0;
	IsINTEDMNUpdate=false;
	WaitForKeyRelease=true;
	}	
	
//�����༭�˵�
void IntEditHandler(const intEditMenuCfg *CFG)
	{
	int pos;
	float direct;
	bool IsKeyHold=KeyState.IsUpHold|KeyState.IsDownHold;		
  if(!WaitForKeyRelease&&IsKeyHold&&KeyState.KeyEvent!=KeyEvent_BothEnt)
		{
		pos=IntIncDec(*(CFG->Source),CFG->min,CFG->max,CFG->Step);
		if(pos>*(CFG->Source))PlusHold=4;
		else if(pos<*(CFG->Source))MinusHold=4; //�����������·�����ֵ����	
		*(CFG->Source)=pos; //���������ֵ��������
		DisplayPlusButton(PlusHold?true:false);	
		DisplayMinusButton(MinusHold?true:false);
		//��ʾ�ַ�
		LCD_Fill(45,28,84,41,LGRAY);
		LCD_ShowIntNum(47,29,*(CFG->Source),*(CFG->Source)>9999?5:4,CYAN,LGRAY,12);	
		//��ʾ��������Bar		
		LCD_Fill(30,56,130,60,LGRAY);
		LCD_Fill(30,64,130,68,LGRAY);
		LCD_DrawRectangle(30,60,129,63,BLACK);
		LCD_Fill(32,61,128,62,LGRAYBLUE);
		for(pos=61;pos<129;pos+=5)LCD_DrawLine(pos,61,pos,62,YELLOW);
	  //����������α�λ��	
		direct=(float)(*CFG->Source)-(float)(CFG->min);	 //VDelta=VIN-Vmin
		direct/=(float)(CFG->max-CFG->min); //�ٷֱ�=Vdelta/Vmax-Vmin
		direct*=(float)96; //�����α�����λ����96��õ��α��λ��ֵ
		pos=(int)direct;
		if(pos<0)pos=0;
		if(pos>96)pos=96; //�����α��λ�ƾ��벻�ܳ���96
		//��ʾ������λ��
		LCD_DrawRectangle(30+pos,56,34+pos,67,BLACK);
		LCD_Fill(31+pos,57,33+pos,66,WHITE);			
		return;	
		}
	if(IsINTEDMNUpdate&&KeyState.KeyEvent==KeyEvent_None)return;//��ִ��
	RenderMenuBG();
	//��ʾ��ߵ��ַ�����ֵ
	DisplayMinusButton(MinusHold?true:false);
	LCD_ShowIntNum(47,29,*(CFG->Source),*(CFG->Source)>9999?5:4,CYAN,LGRAY,12);
	LCD_ShowHybridString(96,29,CFG->Unit,CYAN,LGRAY,0);	
	//��ʾѡ��򱳾�
	LCD_DrawRectangle(43,26,115,43,BLACK);
	LCD_DrawRectangle(44,27,114,42,DARKBLUE);
	//��ʾ�Ӱ�ť
	DisplayPlusButton(PlusHold?true:false);
	//��ʾ��ߵ�����
	LCD_ShowHybridString(3,56,CFG->MinName,WHITE,LGRAY,0);
	//��ʾ��������Bar		
	LCD_DrawRectangle(30,60,129,63,BLACK);
	LCD_Fill(32,61,128,62,LGRAYBLUE);
	for(pos=61;pos<129;pos+=5)LCD_DrawLine(pos,61,pos,62,YELLOW);
	//����������α�λ��	
	direct=(float)(*CFG->Source)-(float)(CFG->min);	 //VDelta=VIN-Vmin
  direct/=(float)(CFG->max-CFG->min); //�ٷֱ�=Vdelta/Vmax-Vmin
	direct*=(float)96; //�����α�����λ����96��õ��α��λ��ֵ
  pos=(int)direct;
  if(pos<0)pos=0;
  if(pos>96)pos=96; //�����α��λ�ƾ��벻�ܳ���96
	//��ʾ������λ��
	LCD_DrawRectangle(30+pos,56,34+pos,67,BLACK);
	LCD_Fill(31+pos,57,33+pos,66,WHITE);
	//��ʾ�ұߵ�����
	LCD_ShowHybridString(132,56,CFG->MaxName,WHITE,LGRAY,0);
	//��������
  if(WaitForKeyRelease)
		{
		IsINTEDMNUpdate=true;
		//����˵�֮��Ҫ�ȴ��û�����
		if(IsKeyHold)return;
		WaitForKeyRelease=0;
		return;
		}
	if(KeyState.KeyEvent==KeyEvent_BothEnt&&CFG->ThingsToDoWhenExit!=NULL)
		{
		KeyState.KeyEvent=KeyEvent_None; //���������¼�
		CFG->ThingsToDoWhenExit(); //�û�ͬʱ�������½�����ֵ�༭
	  return;
		}
	pos=IntIncDec(*(CFG->Source),CFG->min,CFG->max,CFG->Step);
	if(pos>*(CFG->Source))PlusHold=4;
	else if(pos<*(CFG->Source))MinusHold=4; //�����������·�����ֵ����
	else IsINTEDMNUpdate=true; //��ֵû�з��������ָʾ������������
	*(CFG->Source)=pos; //���������ֵ��������
	}
