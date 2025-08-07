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
static bool IsWarningAsserted;
static bool IsINFOAsserted;

const char PostString[][3]=
	{
	"AA",
	"AB",
	"AC"
	};

//初步显示自检屏幕
void PostScreenInit(void)
	{
	IsINFOAsserted=false;		
	IsWarningAsserted=false;
	//显示屏幕
	ClearScreen();	
	LCD_ShowPicture(33,24,91,11,PostSign);
	LCD_DrawRectangle(5,59,153,75,WHITE);
	LCD_DrawRectangle(7,61,8,73,CYAN);
	LCD_Fill(7,61,8,73,CYAN);	
	LCD_ShowString(137,46,"FF",WHITE,BLACK,12,0);		
	}

//根据信息类型选择对应的颜色
static u16 PickColorBasedOnType(MessageTypeDef Type)	
	{
	switch(Type)
		{
		case Msg_POSTOK:return GREEN;
		case Msg_INFO:return ORANGE;
		case Msg_Statu:return CYAN;
		case Msg_Warning:return YELLOW;
		case Msg_Fault:return RED;
		}
	//其余情况返回黑色
	return BLACK;
	}
	
//根据自检结果返回对应的自检ID
const char *QueryPostDoneID(void)
	{
	if(IsWarningAsserted)return PostString[2];
	else if(IsINFOAsserted)return PostString[1];
	//系统自检正常通过，返回AA
	return PostString[0];
	}	
//显示自检信息
void ShowPostInfo(char Present,char *Msg,char *ID,MessageTypeDef Type)
	{
	int i,msglen;
	float len;
	char MsgBuf[40];
	u16 Color;
	//根据消息类型设定颜色和置起flag
	if(Type==Msg_Warning)IsWarningAsserted=true;
	if(Type==Msg_INFO)IsINFOAsserted=true;
	Color=PickColorBasedOnType(Type);            //调用函数根据事件类型选择颜色
	//根据百分比计算进度条要到达的长度
	len=((float)Present/(float)100)*(float)144;
	msglen=(int)(len*(float)10);
	msglen%=10;
	if(msglen>4)i=1;		 
	else i=0;            //对长度数值进行四舍五入
  i+=(int)len;
	if(i>144)i=144;      //限制最大的长度数值不得大于144
  //实现进度条动画效果
	do
		{
		//显示进度条本体
	  LCD_DrawRectangle(7,61,8+Presentage,73,Color);	
		LCD_Fill(7,61,8+Presentage,73,Color);	
		if(Presentage<i)Presentage++;
		if(Presentage>i)Presentage--;
		//进度条动画延时
		if(EnableDetailOutput)delay_ms(3);
		else delay_ms(5);
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
		//显示自检ID
		LCD_ShowString(137,46,ID,WHITE,BLACK,12,0);	
		//显示新的文字
		msglen=strlen(Msg);  //计算长度
		for(i=1;i<=msglen;i++)
			{
			memset(MsgBuf,0,sizeof(MsgBuf));
			memcpy(MsgBuf,Msg,i);
			LCD_ShowHybridString(5,46,Msg,Color,BLACK,0);
			delay_ms(30);
			}
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
