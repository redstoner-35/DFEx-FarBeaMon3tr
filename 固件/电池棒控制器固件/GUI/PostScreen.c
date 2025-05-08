#include "lcd.h"
#include <string.h>
#include "delay.h"
#include "GUI.h"

//内部变量
extern const unsigned char PostSign[2002];
static int Presentage=0;
static MessageTypeDef LastType=Msg_Statu;
static short LastMsgLen=0;
bool EnableDetailOutput=false;

//初步显示自检屏幕
void PostScreenInit(void)
	{
	ClearScreen();	
	LCD_ShowPicture(33,24,91,11,PostSign);
	LCD_DrawRectangle(5,59,153,75,WHITE);
	LCD_DrawRectangle(7,61,8,73,CYAN);
	LCD_Fill(7,61,8,73,CYAN);	
	LCD_ShowString(137,46,"00",WHITE,BLACK,12,0);		
	}

//根据信息类型选择对应的颜色
static u16 PickColorBasedOnType(MessageTypeDef Type)	
	{
	switch(Type)
		{
		case Msg_POSTOK:return GREEN;
		case Msg_Statu:return CYAN;
		case Msg_Warning:return YELLOW;
		case Msg_Fault:return RED;
		}
	//其余情况返回黑色
	return BLACK;
	}
//显示自检信息
void ShowPostInfo(char Present,char *Msg,char *ID,MessageTypeDef Type)
	{
	int i;
	float len;
	//根据消息类型设定颜色
	u16 Color=PickColorBasedOnType(Type);;
	//根据百分比计算进度条要到达的长度
	len=((float)Present)/(float)100*(float)143;
	i=(int)len;
  //实现进度条动画效果
	do
		{
		//显示进度条本体
	  LCD_DrawRectangle(7,61,8+Presentage,73,Color);	
		LCD_Fill(7,61,8+Presentage,73,Color);	
		if(Presentage<i)Presentage++;
		if(Presentage>i)Presentage--;
		delay_ms(1);
		}
	while(Presentage!=i);
	//如果下一条自检信息不是异常，则进行消隐动画
	if(!EnableDetailOutput&&LastType!=Msg_Statu&&Type==Msg_Statu)for(i=5;i<132;i++)	
		{
		LCD_Fill(5,46,i,58,BLACK);
		LCD_DrawLine(i,46,i,58,i<LastMsgLen?WHITE:BLACK);
		delay_ms(4);
		}
	//显示ID和新文字
	if(EnableDetailOutput||Type!=Msg_Statu)
		{
		//制造出文字消隐的效果
		if(EnableDetailOutput||(LastType!=Msg_Statu&&Type!=Msg_Statu))for(i=5;i<132;i++)	
			{
			LCD_Fill(5,46,i,58,BLACK);
			LCD_DrawLine(i,46,i,58,i<LastMsgLen?WHITE:BLACK);
			delay_ms(2);
			}
		//显示新的文字
		LCD_ShowHybridString(5,46,Msg,Color,BLACK,0);
		LCD_ShowString(137,46,ID,WHITE,BLACK,12,0);		
		}
	else LCD_ShowString(137,46,ID,WHITE,BLACK,12,0);	
	//计算上一条消息的长度
	LastMsgLen=0;
	i=0;
  while(Msg[i]!=0)
		{
		i++;
		LastMsgLen++;		
		if(i==20)break;
		}		
	LastMsgLen=(8+LastMsgLen)*6;
	if(LastMsgLen>131)LastMsgLen=131;		
	LastType=Type; //同步本次处理的消息类型
	}
