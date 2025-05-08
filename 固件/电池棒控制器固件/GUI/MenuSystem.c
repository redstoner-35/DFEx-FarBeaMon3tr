#include "lcd.h"
#include "GUI.h"
#include "Key.h"
#include <stdlib.h>
#include "SwitchICON.h"
#include "LCD_Init.h"

//�ⲿassign�õı���
bool AlwaysTrue=true;
bool AlwaysFalse=false;

//�ڲ�����
static const MenuConfigDef *MenuIdx=&MainMenu; //�˵�ָ��
static char SetupMenuIdx=0;
static bool MarkMenuNeedToReturn=false;
static bool IsSetupRendered=false;
static char GUIDelay=0;

//���ݴ��������ѡ��ʱ��
void ShowTimeCode(u16 y,long Time)
	{
	long time;
	if(Time>86400*30) //ʱ������һ����
		{
		time=Time/(86400*30); //���������
		LCD_ShowIntNum(60,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(79,y,"��\0",WHITE,LGRAY,12,0);
		time=(Time%(86400*30))/86400; //���������
		LCD_ShowIntNum(93,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(111,y,"��\0",WHITE,LGRAY,12,0);
		time=(Time%86400)/3600; //�����Сʱ��
		LCD_ShowIntNum(124,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(143,y,"ʱ\0",WHITE,LGRAY,12,0);	
		}
	else if(Time>86400) //ʱ������һ��
		{
		time=Time/86400; //���������
		LCD_ShowIntNum(60,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(79,y,"��\0",WHITE,LGRAY,12,0);
		time=(Time%86400)/3600; //�����Сʱ��
		LCD_ShowIntNum(93,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(111,y,"ʱ\0",WHITE,LGRAY,12,0);
		time=(Time%3600)/60; //�����������
		LCD_ShowIntNum(124,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(143,y,"��\0",WHITE,LGRAY,12,0);		  
		}
   else //ʹ��ʱ����
		{
		time=Time/3600; //�����Сʱ��
		LCD_ShowIntNum(60,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(79,y,"ʱ\0",WHITE,LGRAY,12,0);
		time=(Time%3600)/60; //�����������
		LCD_ShowIntNum(93,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(111,y,"��\0",WHITE,LGRAY,12,0);
		time=Time%60;
		LCD_ShowIntNum(124,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(143,y,"��\0",WHITE,LGRAY,12,0);		  
		}
	}

//GUI��ʱ��
void GUIDelayHandler(void)
	{
	if(GUIDelay>0)GUIDelay--;
	}

//�л��˵�
void SwitchingMenu(const MenuConfigDef *TargetMenuIdx)
	{
	int i;
	if(TargetMenuIdx==NULL)return;
	//ִ���˳����˵�ǰҪ��������
	if(MenuIdx->CustomMenuRender==NULL)SetupMenuIdx=0;
	if(MenuIdx->ThingsToDoBeforeLeave!=NULL)MenuIdx->ThingsToDoBeforeLeave();
	//ִ�н����²˵�ǰҪ��������
	if(TargetMenuIdx->ThingsToDoBeforeEnter!=NULL)TargetMenuIdx->ThingsToDoBeforeEnter();
	if(TargetMenuIdx->Type==MenuType_EnumSetup&&TargetMenuIdx->ReadEnumSource!=NULL)
		{
		for(i=0;i<64;i++)if(TargetMenuIdx->Entry[i].IsPlaceHolder)break; //�ҵ�enum����
		i--; //placeholder����λ��-1Ϊ���һ����Ŀ
		while(i>=0)
			{
			//�ҵ���Ӧenumֵ����ʼ��index
			if(TargetMenuIdx->ReadEnumSource()==TargetMenuIdx->Entry[i].EnumValue)
				{
				SetupMenuIdx=i;
				break;
				}
			i--;
			}
		}
	//���в˵��л�
	ClearScreen(); //����
	MenuIdx=TargetMenuIdx;
	IsSetupRendered=false;
	}
//�˳�ö�ٱ༭��ʱ������ö��ֵ���˳�
static void CalcEnumValueAndReturn(void)
	{
	if(MenuIdx->FedEnumToWhat!=NULL)MenuIdx->FedEnumToWhat(MenuIdx->Entry[SetupMenuIdx].EnumValue);
	SetupMenuIdx=0;
	}

//�˵�������Ⱦ
void RenderMenuBG(void)
	{
	LCD_Fill(0,0,159,79,LGRAY);
	if(MenuIdx->MenuTitle!=NULL)LCD_ShowHybridString(4,3,MenuIdx->MenuTitle,WHITE,LGRAY,0);
	LCD_DrawRectangle(0,0,159,17,WHITE); 
	LCD_DrawRectangle(0,17,159,79,WHITE);	
	//�ĸ���
	LCD_DrawPoint(1,1,WHITE);
	LCD_DrawPoint(1,2,WHITE);
	LCD_DrawPoint(1,2,WHITE);
		
	LCD_DrawPoint(157,1,WHITE);
	LCD_DrawPoint(158,1,WHITE);
	LCD_DrawPoint(158,2,WHITE);		
		
	LCD_DrawPoint(1,15,WHITE);
	LCD_DrawPoint(1,16,WHITE);
	LCD_DrawPoint(2,16,WHITE);		
		
	LCD_DrawPoint(158,15,WHITE);
	LCD_DrawPoint(158,16,WHITE);
	LCD_DrawPoint(157,16,WHITE);			
	}		

//ִ�в˵���Ⱦ
void MenuRenderProcess(void)
	{
	int i,CurrentPage,MaxIdx;
	bool IsSel,MasterResult;
	u16 FG;		
	//��λ�ڴ�
	AlwaysTrue=true;
	AlwaysFalse=false;
	//ִ���Զ�����Ⱦ
	if(MenuIdx->Type==MenuType_Custom)
		{
		if(MenuIdx->CustomMenuRender!=NULL)MenuIdx->CustomMenuRender();
		if(MenuIdx->CustomKeyProcess!=NULL)MenuIdx->CustomKeyProcess();
		}
	//���ò˵���ʹ��ϵͳ��Ⱦ
	else if(MenuIdx->Type==MenuType_Setup)
		{
		if(IsSetupRendered&&KeyState.KeyEvent==KeyEvent_None)return;
    RenderMenuBG(); //��Ⱦ����
   	for(i=0;i<64;i++)if(MenuIdx->Sel[i].IsPlaceHolder)break; //Ѱ�Ҳ˵�������Ŀ��
		MaxIdx=i-1;
		if(SetupMenuIdx>MaxIdx)SetupMenuIdx=MaxIdx; //����idx��Ŀ��������
		CurrentPage=SetupMenuIdx/4; //���㵱ǰҳ��
		for(i=0;i<4;i++)
			{
			if((CurrentPage*4)+i>MaxIdx)break; //�޷�
			if(*(MenuIdx->Sel[(CurrentPage*4)+i].IsCanBeSelect))
				{
				IsSel=((CurrentPage*4)+i==SetupMenuIdx)?true:false; //ѡ����Ա�ѡ��
				LCD_ShowHybridString(4,20+i*15,MenuIdx->Sel[(CurrentPage*4)+i].SelName,IsSel?CYAN:WHITE,LGRAY,0); //��ʾѡ������
				}
			else LCD_ShowHybridString(4,20+i*15,MenuIdx->Sel[(CurrentPage*4)+i].SelName,BRRED,LGRAY,0); //��ʾѡ������
			}
		//������Ⱦ
		if(MenuIdx->AdditionalRender!=NULL)MenuIdx->AdditionalRender();
		//ȷ�ϼ�
		if(KeyState.KeyEvent==KeyEvent_Enter&&MenuIdx->Sel[SetupMenuIdx].ThingsToDoBeforeEnter!=NULL)//ִ��Enter
			{
			KeyState.KeyEvent=KeyEvent_None;
			MenuIdx->Sel[SetupMenuIdx].ThingsToDoBeforeEnter(); 
			return;
			}
		//�˳���
		else if(KeyState.KeyEvent==KeyEvent_ESC&&MenuIdx->ThingsToDoWhenExit!=NULL) //ִ���˳�����	
			{
			KeyState.KeyEvent=KeyEvent_None;
			MenuIdx->ThingsToDoWhenExit();
			return;
			}
		//���¼�ִ��ѡ��
    else if(KeyState.KeyEvent==KeyEvent_Down)
			{
			//��Ļ��ʾ����Ϊ�෴
			if(Direction==LCDDisplay_Hori_Invert)do
				{
				if(SetupMenuIdx<MaxIdx)SetupMenuIdx++;
				else SetupMenuIdx=0;
				}
			while(!*(MenuIdx->Sel[SetupMenuIdx].IsCanBeSelect));
			//��������Ϊ���
			else do
				{
				if(SetupMenuIdx>0)SetupMenuIdx--;
				else SetupMenuIdx=MaxIdx;
				}		
			while(!*(MenuIdx->Sel[SetupMenuIdx].IsCanBeSelect));
			}
		else if(KeyState.KeyEvent==KeyEvent_Up)
			{
			//��Ļ��ʾ����Ϊ����
			if(Direction!=LCDDisplay_Hori_Invert)do
				{
				if(SetupMenuIdx<MaxIdx)SetupMenuIdx++;
				else SetupMenuIdx=0;
				}
			while(!*(MenuIdx->Sel[SetupMenuIdx].IsCanBeSelect));
			//�෴����Ϊ���
			else do
				{
				if(SetupMenuIdx>0)SetupMenuIdx--;
				else SetupMenuIdx=MaxIdx;
				}		
			while(!*(MenuIdx->Sel[SetupMenuIdx].IsCanBeSelect));
			}			
	  //��Ȿ�δ����Ƿ����
		if(KeyState.KeyEvent==KeyEvent_None)IsSetupRendered=true; //�����������Ⱦ
		else IsSetupRendered=false;
		KeyState.KeyEvent=KeyEvent_None;
		}
	//ö�ٱ༭
	else if(MenuIdx->Type==MenuType_EnumSetup)
		{
		if(MarkMenuNeedToReturn)	
			{
			KeyState.KeyEvent=KeyEvent_None;
			if(GUIDelay)return; //��ʾ������
			CalcEnumValueAndReturn();
			MarkMenuNeedToReturn=0;	
			return;
			}
		if(IsSetupRendered&&KeyState.KeyEvent==KeyEvent_None)return;
		//��Ⱦ����
		RenderMenuBG();	
		//��ʾ��Ŀ
		for(i=0;i<64;i++)if(MenuIdx->Entry[i].IsPlaceHolder)break; //�ҵ�enum����
		MaxIdx=i-1; //placeholder����λ��-1Ϊ���һ����Ŀ
		CurrentPage=SetupMenuIdx/4; //���㵱ǰҳ��
		for(i=0;i<4;i++)	
			{
			if((CurrentPage*4)+i>MaxIdx)break; //�޷�
			IsSel=((CurrentPage*4)+i==SetupMenuIdx)?true:false; //ѡ����Ա�ѡ��
			if(MenuIdx->Entry[(CurrentPage*4)+i].IsChinese)LCD_ShowHybridString(4,20+i*15,MenuIdx->Entry[(CurrentPage*4)+i].SelName,IsSel?CYAN:WHITE,LGRAY,0); //����ģʽʹ�������ֿ���ʾѡ������
			else LCD_ShowString(4,20+i*15,MenuIdx->Entry[(CurrentPage*4)+i].SelName,IsSel?CYAN:WHITE,LGRAY,12,0); //Ӣ��ģʽ
			}
		//���¼�ִ��ѡ��
    if(KeyState.KeyEvent==KeyEvent_Down)
			{
			if(Direction==LCDDisplay_Hori_Invert)	
				{
				if(SetupMenuIdx<MaxIdx)SetupMenuIdx++;
				else SetupMenuIdx=0;
				}
			else
				{
				if(SetupMenuIdx>0)SetupMenuIdx--;
				else SetupMenuIdx=MaxIdx;					
				}	
			}
		if(KeyState.KeyEvent==KeyEvent_Up)
			{
			if(Direction!=LCDDisplay_Hori_Invert)	
				{
				if(SetupMenuIdx<MaxIdx)SetupMenuIdx++;
				else SetupMenuIdx=0;
				}
			else
				{
				if(SetupMenuIdx>0)SetupMenuIdx--;
				else SetupMenuIdx=MaxIdx;					
				}	
			}		
		//ȷ�ϱ��沢�˳�
		if(KeyState.KeyEvent==KeyEvent_Enter)
			{
			RenderMenuBG();
			LCD_ShowChinese(42,33,"�����ѱ���\0",WHITE,LGRAY,0);
			MarkMenuNeedToReturn=true;
			GUIDelay=12;
			}
		if(KeyState.KeyEvent==KeyEvent_ESC)CalcEnumValueAndReturn(); //ִ�ж�Ӧ���˳�����
	  //��Ȿ�δ����Ƿ����
		if(KeyState.KeyEvent==KeyEvent_None)IsSetupRendered=true; //�����������Ⱦ
		else IsSetupRendered=false;
		KeyState.KeyEvent=KeyEvent_None;
		}
	//��������list
	else if(MenuIdx->Type==MenuType_BoolListSetup)
		{
		if(IsSetupRendered&&KeyState.KeyEvent==KeyEvent_None)return;
		//��Ⱦ����
		RenderMenuBG();	
		//��ʾ��Ŀ
		for(i=0;i<64;i++)if(MenuIdx->BoolEntry[i].IsPlaceHolder)break; //�ҵ�enum����
		MaxIdx=i-1; //placeholder����λ��-1Ϊ���һ����Ŀ
		if(SetupMenuIdx>MaxIdx)SetupMenuIdx=MaxIdx; //����idx��Ŀ��������
		CurrentPage=SetupMenuIdx/4; //���㵱ǰҳ��
		MasterResult=true;
		for(i=0;i<MaxIdx+1;i++)if(MenuIdx->BoolEntry[i].IsMaster&&!*(MenuIdx->BoolEntry[i].ValueSource))MasterResult=false; //��entry���б�����ȡmaster�Ľ��
		for(i=0;i<4;i++)	
			{
			if((CurrentPage*4)+i>MaxIdx)break; //�޷�
			//����״̬ѡ��ѡ���Ƿ�ѡ��
			if(!MasterResult&&!MenuIdx->BoolEntry[i].IsMaster)FG=BRRED; //ѡ����Ҫ���أ�������ѡ��
			else if((CurrentPage*4)+i==SetupMenuIdx)FG=CYAN; //ѡ�ѡ��
			else FG=WHITE; //ָʾδѡ��
			//��Ⱦѡ����
			if(MenuIdx->BoolEntry[(CurrentPage*4)+i].IsChinese)LCD_ShowHybridString(4,20+i*15,MenuIdx->BoolEntry[(CurrentPage*4)+i].SelName,FG,LGRAY,0); //����ģʽʹ�������ֿ���ʾѡ������
			else LCD_ShowString(4,20+i*15,MenuIdx->BoolEntry[(CurrentPage*4)+i].SelName,FG,LGRAY,12,0); //Ӣ��ģʽ
			//��Ⱦ����״̬
		  if(*(MenuIdx->BoolEntry[(CurrentPage*4)+i].ValueSource))LCD_ShowPicture(132,21+(i*15),20,10,SwitchON);
			else LCD_ShowPicture(132,21+(i*15),20,10,SwitchOFF);
			}
		//���¼�ִ��ѡ��
		if(KeyState.KeyEvent==KeyEvent_Down)
			{
			//��Ļ��ʾ����Ϊ�෴
			if(Direction==LCDDisplay_Hori_Invert)do
				{
				if(SetupMenuIdx<MaxIdx)SetupMenuIdx++;
				else SetupMenuIdx=0;
				}
			while(!MasterResult&&!MenuIdx->BoolEntry[SetupMenuIdx].IsMaster);
			//��������Ϊ���
			else do
				{
				if(SetupMenuIdx>0)SetupMenuIdx--;
				else SetupMenuIdx=MaxIdx;
				}		
			while(!MasterResult&&!MenuIdx->BoolEntry[SetupMenuIdx].IsMaster);
			}
		else if(KeyState.KeyEvent==KeyEvent_Up)
			{
			//��Ļ��ʾ����Ϊ����
			if(Direction!=LCDDisplay_Hori_Invert)do
				{
				if(SetupMenuIdx<MaxIdx)SetupMenuIdx++;
				else SetupMenuIdx=0;
				}
			while(!MasterResult&&!MenuIdx->BoolEntry[SetupMenuIdx].IsMaster);
			//�෴����Ϊ���
			else do
				{
				if(SetupMenuIdx>0)SetupMenuIdx--;
				else SetupMenuIdx=MaxIdx;
				}		
			while(!MasterResult&&!MenuIdx->BoolEntry[SetupMenuIdx].IsMaster);
			}				
		//ȷ�ϼ��л�״̬
		else if(KeyState.KeyEvent==KeyEvent_Enter) //�����л�����
			{
			if(*(MenuIdx->BoolEntry[SetupMenuIdx].ValueSource))*(MenuIdx->BoolEntry[SetupMenuIdx].ValueSource)=false;
			else *(MenuIdx->BoolEntry[SetupMenuIdx].ValueSource)=true;
			}
		//�˳���ֱ���˳�
		else if(KeyState.KeyEvent==KeyEvent_ESC&&MenuIdx->ThingsToDoWhenExit!=NULL)MenuIdx->ThingsToDoWhenExit(); //ִ�ж�Ӧ���˳�����
	  //��Ȿ�δ����Ƿ����
		if(KeyState.KeyEvent==KeyEvent_None)IsSetupRendered=true; //�����������Ⱦ
		else IsSetupRendered=false;
		KeyState.KeyEvent=KeyEvent_None;
		}
	}
