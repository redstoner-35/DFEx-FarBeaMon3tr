#include "lcd.h"
#include <string.h>
#include "delay.h"
#include "GUI.h"

//�ڲ�����
extern const unsigned char PostSign[2002];
static int Presentage=0;
static MessageTypeDef LastType=Msg_Statu;
static short LastMsgLen=0;
bool EnableDetailOutput=false;

//������ʾ�Լ���Ļ
void PostScreenInit(void)
	{
	ClearScreen();	
	LCD_ShowPicture(33,24,91,11,PostSign);
	LCD_DrawRectangle(5,59,153,75,WHITE);
	LCD_DrawRectangle(7,61,8,73,CYAN);
	LCD_Fill(7,61,8,73,CYAN);	
	LCD_ShowString(137,46,"00",WHITE,BLACK,12,0);		
	}

//������Ϣ����ѡ���Ӧ����ɫ
static u16 PickColorBasedOnType(MessageTypeDef Type)	
	{
	switch(Type)
		{
		case Msg_POSTOK:return GREEN;
		case Msg_Statu:return CYAN;
		case Msg_Warning:return YELLOW;
		case Msg_Fault:return RED;
		}
	//����������غ�ɫ
	return BLACK;
	}
//��ʾ�Լ���Ϣ
void ShowPostInfo(char Present,char *Msg,char *ID,MessageTypeDef Type)
	{
	int i;
	float len;
	//������Ϣ�����趨��ɫ
	u16 Color=PickColorBasedOnType(Type);;
	//���ݰٷֱȼ��������Ҫ����ĳ���
	len=((float)Present)/(float)100*(float)143;
	i=(int)len;
  //ʵ�ֽ���������Ч��
	do
		{
		//��ʾ����������
	  LCD_DrawRectangle(7,61,8+Presentage,73,Color);	
		LCD_Fill(7,61,8+Presentage,73,Color);	
		if(Presentage<i)Presentage++;
		if(Presentage>i)Presentage--;
		delay_ms(1);
		}
	while(Presentage!=i);
	//�����һ���Լ���Ϣ�����쳣���������������
	if(!EnableDetailOutput&&LastType!=Msg_Statu&&Type==Msg_Statu)for(i=5;i<132;i++)	
		{
		LCD_Fill(5,46,i,58,BLACK);
		LCD_DrawLine(i,46,i,58,i<LastMsgLen?WHITE:BLACK);
		delay_ms(4);
		}
	//��ʾID��������
	if(EnableDetailOutput||Type!=Msg_Statu)
		{
		//���������������Ч��
		if(EnableDetailOutput||(LastType!=Msg_Statu&&Type!=Msg_Statu))for(i=5;i<132;i++)	
			{
			LCD_Fill(5,46,i,58,BLACK);
			LCD_DrawLine(i,46,i,58,i<LastMsgLen?WHITE:BLACK);
			delay_ms(2);
			}
		//��ʾ�µ�����
		LCD_ShowHybridString(5,46,Msg,Color,BLACK,0);
		LCD_ShowString(137,46,ID,WHITE,BLACK,12,0);		
		}
	else LCD_ShowString(137,46,ID,WHITE,BLACK,12,0);	
	//������һ����Ϣ�ĳ���
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
	LastType=Type; //ͬ�����δ������Ϣ����
	}
