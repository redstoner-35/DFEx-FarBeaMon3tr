#include "lcd.h"
#include "GUI.h"
#include "Key.h"
#include <stdlib.h>
#include "SwitchICON.h"
#include "LCD_Init.h"

//外部assign用的变量
bool AlwaysTrue=true;
bool AlwaysFalse=false;

//内部变量
static const MenuConfigDef *MenuIdx=&MainMenu; //菜单指针
static char SetupMenuIdx=0;
static bool MarkMenuNeedToReturn=false;
static bool IsSetupRendered=false;
static char GUIDelay=0;

//根据传入的秒数选择时间
void ShowTimeCode(u16 y,long Time)
	{
	long time;
	if(Time>86400*30) //时长超过一个月
		{
		time=Time/(86400*30); //计算出月数
		LCD_ShowIntNum(60,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(79,y,"月\0",WHITE,LGRAY,12,0);
		time=(Time%(86400*30))/86400; //计算出天数
		LCD_ShowIntNum(93,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(111,y,"天\0",WHITE,LGRAY,12,0);
		time=(Time%86400)/3600; //计算出小时数
		LCD_ShowIntNum(124,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(143,y,"时\0",WHITE,LGRAY,12,0);	
		}
	else if(Time>86400) //时长超过一天
		{
		time=Time/86400; //计算出天数
		LCD_ShowIntNum(60,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(79,y,"天\0",WHITE,LGRAY,12,0);
		time=(Time%86400)/3600; //计算出小时数
		LCD_ShowIntNum(93,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(111,y,"时\0",WHITE,LGRAY,12,0);
		time=(Time%3600)/60; //计算出分钟数
		LCD_ShowIntNum(124,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(143,y,"分\0",WHITE,LGRAY,12,0);		  
		}
   else //使用时分秒
		{
		time=Time/3600; //计算出小时数
		LCD_ShowIntNum(60,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(79,y,"时\0",WHITE,LGRAY,12,0);
		time=(Time%3600)/60; //计算出分钟数
		LCD_ShowIntNum(93,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(111,y,"分\0",WHITE,LGRAY,12,0);
		time=Time%60;
		LCD_ShowIntNum(124,y,time,2,YELLOW,LGRAY,12);
		LCD_ShowChinese12x12(143,y,"秒\0",WHITE,LGRAY,12,0);		  
		}
	}

//GUI计时器
void GUIDelayHandler(void)
	{
	if(GUIDelay>0)GUIDelay--;
	}

//切换菜单
void SwitchingMenu(const MenuConfigDef *TargetMenuIdx)
	{
	int i;
	if(TargetMenuIdx==NULL)return;
	//执行退出本菜单前要做的事情
	if(MenuIdx->CustomMenuRender==NULL)SetupMenuIdx=0;
	if(MenuIdx->ThingsToDoBeforeLeave!=NULL)MenuIdx->ThingsToDoBeforeLeave();
	//执行进入新菜单前要做的事情
	if(TargetMenuIdx->ThingsToDoBeforeEnter!=NULL)TargetMenuIdx->ThingsToDoBeforeEnter();
	if(TargetMenuIdx->Type==MenuType_EnumSetup&&TargetMenuIdx->ReadEnumSource!=NULL)
		{
		for(i=0;i<64;i++)if(TargetMenuIdx->Entry[i].IsPlaceHolder)break; //找到enum总数
		i--; //placeholder所在位置-1为最后一条条目
		while(i>=0)
			{
			//找到对应enum值并初始化index
			if(TargetMenuIdx->ReadEnumSource()==TargetMenuIdx->Entry[i].EnumValue)
				{
				SetupMenuIdx=i;
				break;
				}
			i--;
			}
		}
	//进行菜单切换
	ClearScreen(); //清屏
	MenuIdx=TargetMenuIdx;
	IsSetupRendered=false;
	}
//退出枚举编辑器时，计算枚举值并退出
static void CalcEnumValueAndReturn(void)
	{
	if(MenuIdx->FedEnumToWhat!=NULL)MenuIdx->FedEnumToWhat(MenuIdx->Entry[SetupMenuIdx].EnumValue);
	SetupMenuIdx=0;
	}

//菜单背景渲染
void RenderMenuBG(void)
	{
	LCD_Fill(0,0,159,79,LGRAY);
	if(MenuIdx->MenuTitle!=NULL)LCD_ShowHybridString(4,3,MenuIdx->MenuTitle,WHITE,LGRAY,0);
	LCD_DrawRectangle(0,0,159,17,WHITE); 
	LCD_DrawRectangle(0,17,159,79,WHITE);	
	//四个角
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

//执行菜单渲染
void MenuRenderProcess(void)
	{
	int i,CurrentPage,MaxIdx;
	bool IsSel,MasterResult;
	u16 FG;		
	//复位内存
	AlwaysTrue=true;
	AlwaysFalse=false;
	//执行自定义渲染
	if(MenuIdx->Type==MenuType_Custom)
		{
		if(MenuIdx->CustomMenuRender!=NULL)MenuIdx->CustomMenuRender();
		if(MenuIdx->CustomKeyProcess!=NULL)MenuIdx->CustomKeyProcess();
		}
	//设置菜单，使用系统渲染
	else if(MenuIdx->Type==MenuType_Setup)
		{
		if(IsSetupRendered&&KeyState.KeyEvent==KeyEvent_None)return;
    RenderMenuBG(); //渲染背景
   	for(i=0;i<64;i++)if(MenuIdx->Sel[i].IsPlaceHolder)break; //寻找菜单的总条目数
		MaxIdx=i-1;
		if(SetupMenuIdx>MaxIdx)SetupMenuIdx=MaxIdx; //限制idx数目不允许超出
		CurrentPage=SetupMenuIdx/4; //计算当前页数
		for(i=0;i<4;i++)
			{
			if((CurrentPage*4)+i>MaxIdx)break; //限幅
			if(*(MenuIdx->Sel[(CurrentPage*4)+i].IsCanBeSelect))
				{
				IsSel=((CurrentPage*4)+i==SetupMenuIdx)?true:false; //选项可以被选中
				LCD_ShowHybridString(4,20+i*15,MenuIdx->Sel[(CurrentPage*4)+i].SelName,IsSel?CYAN:WHITE,LGRAY,0); //显示选项名称
				}
			else LCD_ShowHybridString(4,20+i*15,MenuIdx->Sel[(CurrentPage*4)+i].SelName,BRRED,LGRAY,0); //显示选项名称
			}
		//额外渲染
		if(MenuIdx->AdditionalRender!=NULL)MenuIdx->AdditionalRender();
		//确认键
		if(KeyState.KeyEvent==KeyEvent_Enter&&MenuIdx->Sel[SetupMenuIdx].ThingsToDoBeforeEnter!=NULL)//执行Enter
			{
			KeyState.KeyEvent=KeyEvent_None;
			MenuIdx->Sel[SetupMenuIdx].ThingsToDoBeforeEnter(); 
			return;
			}
		//退出键
		else if(KeyState.KeyEvent==KeyEvent_ESC&&MenuIdx->ThingsToDoWhenExit!=NULL) //执行退出处理	
			{
			KeyState.KeyEvent=KeyEvent_None;
			MenuIdx->ThingsToDoWhenExit();
			return;
			}
		//上下键执行选择
    else if(KeyState.KeyEvent==KeyEvent_Down)
			{
			//屏幕显示方向为相反
			if(Direction==LCDDisplay_Hori_Invert)do
				{
				if(SetupMenuIdx<MaxIdx)SetupMenuIdx++;
				else SetupMenuIdx=0;
				}
			while(!*(MenuIdx->Sel[SetupMenuIdx].IsCanBeSelect));
			//正常方向为相减
			else do
				{
				if(SetupMenuIdx>0)SetupMenuIdx--;
				else SetupMenuIdx=MaxIdx;
				}		
			while(!*(MenuIdx->Sel[SetupMenuIdx].IsCanBeSelect));
			}
		else if(KeyState.KeyEvent==KeyEvent_Up)
			{
			//屏幕显示方向为正常
			if(Direction!=LCDDisplay_Hori_Invert)do
				{
				if(SetupMenuIdx<MaxIdx)SetupMenuIdx++;
				else SetupMenuIdx=0;
				}
			while(!*(MenuIdx->Sel[SetupMenuIdx].IsCanBeSelect));
			//相反方向为相减
			else do
				{
				if(SetupMenuIdx>0)SetupMenuIdx--;
				else SetupMenuIdx=MaxIdx;
				}		
			while(!*(MenuIdx->Sel[SetupMenuIdx].IsCanBeSelect));
			}			
	  //监测本次处理是否完成
		if(KeyState.KeyEvent==KeyEvent_None)IsSetupRendered=true; //本次已完成渲染
		else IsSetupRendered=false;
		KeyState.KeyEvent=KeyEvent_None;
		}
	//枚举编辑
	else if(MenuIdx->Type==MenuType_EnumSetup)
		{
		if(MarkMenuNeedToReturn)	
			{
			KeyState.KeyEvent=KeyEvent_None;
			if(GUIDelay)return; //显示保存结果
			CalcEnumValueAndReturn();
			MarkMenuNeedToReturn=0;	
			return;
			}
		if(IsSetupRendered&&KeyState.KeyEvent==KeyEvent_None)return;
		//渲染背景
		RenderMenuBG();	
		//显示条目
		for(i=0;i<64;i++)if(MenuIdx->Entry[i].IsPlaceHolder)break; //找到enum总数
		MaxIdx=i-1; //placeholder所在位置-1为最后一条条目
		CurrentPage=SetupMenuIdx/4; //计算当前页数
		for(i=0;i<4;i++)	
			{
			if((CurrentPage*4)+i>MaxIdx)break; //限幅
			IsSel=((CurrentPage*4)+i==SetupMenuIdx)?true:false; //选项可以被选中
			if(MenuIdx->Entry[(CurrentPage*4)+i].IsChinese)LCD_ShowHybridString(4,20+i*15,MenuIdx->Entry[(CurrentPage*4)+i].SelName,IsSel?CYAN:WHITE,LGRAY,0); //中文模式使用中文字库显示选项名称
			else LCD_ShowString(4,20+i*15,MenuIdx->Entry[(CurrentPage*4)+i].SelName,IsSel?CYAN:WHITE,LGRAY,12,0); //英文模式
			}
		//上下键执行选择
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
		//确认保存并退出
		if(KeyState.KeyEvent==KeyEvent_Enter)
			{
			RenderMenuBG();
			LCD_ShowChinese(42,33,"设置已保存\0",WHITE,LGRAY,0);
			MarkMenuNeedToReturn=true;
			GUIDelay=12;
			}
		if(KeyState.KeyEvent==KeyEvent_ESC)CalcEnumValueAndReturn(); //执行对应的退出函数
	  //监测本次处理是否完成
		if(KeyState.KeyEvent==KeyEvent_None)IsSetupRendered=true; //本次已完成渲染
		else IsSetupRendered=false;
		KeyState.KeyEvent=KeyEvent_None;
		}
	//布尔变量list
	else if(MenuIdx->Type==MenuType_BoolListSetup)
		{
		if(IsSetupRendered&&KeyState.KeyEvent==KeyEvent_None)return;
		//渲染背景
		RenderMenuBG();	
		//显示条目
		for(i=0;i<64;i++)if(MenuIdx->BoolEntry[i].IsPlaceHolder)break; //找到enum总数
		MaxIdx=i-1; //placeholder所在位置-1为最后一条条目
		if(SetupMenuIdx>MaxIdx)SetupMenuIdx=MaxIdx; //限制idx数目不允许超出
		CurrentPage=SetupMenuIdx/4; //计算当前页数
		MasterResult=true;
		for(i=0;i<MaxIdx+1;i++)if(MenuIdx->BoolEntry[i].IsMaster&&!*(MenuIdx->BoolEntry[i].ValueSource))MasterResult=false; //对entry进行遍历读取master的结果
		for(i=0;i<4;i++)	
			{
			if((CurrentPage*4)+i>MaxIdx)break; //限幅
			//根据状态选择选项是否被选中
			if(!MasterResult&&!MenuIdx->BoolEntry[i].IsMaster)FG=BRRED; //选项需要隐藏，不允许选中
			else if((CurrentPage*4)+i==SetupMenuIdx)FG=CYAN; //选项被选中
			else FG=WHITE; //指示未选中
			//渲染选项名
			if(MenuIdx->BoolEntry[(CurrentPage*4)+i].IsChinese)LCD_ShowHybridString(4,20+i*15,MenuIdx->BoolEntry[(CurrentPage*4)+i].SelName,FG,LGRAY,0); //中文模式使用中文字库显示选项名称
			else LCD_ShowString(4,20+i*15,MenuIdx->BoolEntry[(CurrentPage*4)+i].SelName,FG,LGRAY,12,0); //英文模式
			//渲染开关状态
		  if(*(MenuIdx->BoolEntry[(CurrentPage*4)+i].ValueSource))LCD_ShowPicture(132,21+(i*15),20,10,SwitchON);
			else LCD_ShowPicture(132,21+(i*15),20,10,SwitchOFF);
			}
		//上下键执行选择
		if(KeyState.KeyEvent==KeyEvent_Down)
			{
			//屏幕显示方向为相反
			if(Direction==LCDDisplay_Hori_Invert)do
				{
				if(SetupMenuIdx<MaxIdx)SetupMenuIdx++;
				else SetupMenuIdx=0;
				}
			while(!MasterResult&&!MenuIdx->BoolEntry[SetupMenuIdx].IsMaster);
			//正常方向为相减
			else do
				{
				if(SetupMenuIdx>0)SetupMenuIdx--;
				else SetupMenuIdx=MaxIdx;
				}		
			while(!MasterResult&&!MenuIdx->BoolEntry[SetupMenuIdx].IsMaster);
			}
		else if(KeyState.KeyEvent==KeyEvent_Up)
			{
			//屏幕显示方向为正常
			if(Direction!=LCDDisplay_Hori_Invert)do
				{
				if(SetupMenuIdx<MaxIdx)SetupMenuIdx++;
				else SetupMenuIdx=0;
				}
			while(!MasterResult&&!MenuIdx->BoolEntry[SetupMenuIdx].IsMaster);
			//相反方向为相减
			else do
				{
				if(SetupMenuIdx>0)SetupMenuIdx--;
				else SetupMenuIdx=MaxIdx;
				}		
			while(!MasterResult&&!MenuIdx->BoolEntry[SetupMenuIdx].IsMaster);
			}				
		//确认键切换状态
		else if(KeyState.KeyEvent==KeyEvent_Enter) //反复切换开关
			{
			if(*(MenuIdx->BoolEntry[SetupMenuIdx].ValueSource))*(MenuIdx->BoolEntry[SetupMenuIdx].ValueSource)=false;
			else *(MenuIdx->BoolEntry[SetupMenuIdx].ValueSource)=true;
			}
		//退出键直接退出
		else if(KeyState.KeyEvent==KeyEvent_ESC&&MenuIdx->ThingsToDoWhenExit!=NULL)MenuIdx->ThingsToDoWhenExit(); //执行对应的退出函数
	  //监测本次处理是否完成
		if(KeyState.KeyEvent==KeyEvent_None)IsSetupRendered=true; //本次已完成渲染
		else IsSetupRendered=false;
		KeyState.KeyEvent=KeyEvent_None;
		}
	}
