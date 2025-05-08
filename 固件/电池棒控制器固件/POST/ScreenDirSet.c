#include "LCD_Init.h"
#include "Config.h"
#include "ht32.h"
#include "GUI.h"
#include "Pindefs.h"

void SetScreenDirection(void)
	{
	//��ȡ���������
	if(Direction==CfgData.DisplayDir)return; //����һ�²���Ҫ����
	Direction=CfgData.DisplayDir;
	if(Direction==LCDDisplay_Vert_Normal||Direction==LCDDisplay_Vert_Invert)
		{
		Direction=LCDDisplay_Hori_Normal;
		CfgData.DisplayDir=LCDDisplay_Hori_Normal;
		WriteConfiguration(&CfgUnion,true); 
		}
	//���³�ʼ����ĻӦ����Ļ����
	LCD_Init();
	LCD_EnableBackLight(); 
	}

void ApplyScreenDirection(void)
	{
	ShowPostInfo(72,"������ʾ����\0","2F",Msg_Statu);
	SetScreenDirection(); //Ӧ����Ļ����
	PostScreenInit(); //������ʾ�Լ����
	}
