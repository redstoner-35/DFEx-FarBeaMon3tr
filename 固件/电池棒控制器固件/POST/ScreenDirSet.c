#include "LCD_Init.h"
#include "Config.h"
#include "ht32.h"
#include "GUI.h"
#include "Pindefs.h"

void SetScreenDirection(void)
	{
	//读取并检查配置
	if(Direction==CfgData.DisplayDir)return; //方向一致不需要重设
	Direction=CfgData.DisplayDir;
	if(Direction==LCDDisplay_Vert_Normal||Direction==LCDDisplay_Vert_Invert)
		{
		Direction=LCDDisplay_Hori_Normal;
		CfgData.DisplayDir=LCDDisplay_Hori_Normal;
		WriteConfiguration(&CfgUnion,true); 
		}
	//重新初始化屏幕应用屏幕方向
	LCD_Init();
	LCD_EnableBackLight(); 
	}

void ApplyScreenDirection(void)
	{
	ShowPostInfo(72,"设置显示方向\0","2F",Msg_Statu);
	SetScreenDirection(); //应用屏幕方向
	PostScreenInit(); //重新显示自检界面
	}
