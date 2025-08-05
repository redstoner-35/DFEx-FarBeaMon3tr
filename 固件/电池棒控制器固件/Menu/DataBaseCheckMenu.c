#include "GUI.h"
#include "LogSystem.h"
#include "Key.h"
#include <string.h>
#include <stdio.h>

//函数声明
void BackToResetMenu(void);

//内部变量
static short LogEntryNum=0;
static short BrokenNum=0;
static bool LogChkGUIRendered=false;

void DataBaseCheckHandler(void)
	{
	char CorruptedChkStr[32];
	bool Result;
	if(!LogChkGUIRendered)
		{
		RenderMenuBG(); //显示背景
	  if(LogEntryNum==RunTimeLoggerDepth+2)LCD_ShowChinese(27,40,"日志数据校验失败",RED,LGRAY,0);
    else if(LogEntryNum==RunTimeLoggerDepth+1)LCD_ShowChinese(27,40,"日志数据校验完成",GREEN,LGRAY,0); 
		else 
			{
			//校验运行中渲染背景
			LCD_ShowChinese(21,21,"日志数据校验运行中",WHITE,LGRAY,0); 
			LCD_ShowHybridString(21,35,"进度:",WHITE,LGRAY,0);	
			memset(CorruptedChkStr,0,sizeof(CorruptedChkStr[32]));	
			snprintf(CorruptedChkStr,32," / %d条目",RunTimeLoggerDepth);	
			LCD_ShowHybridString(75,35,CorruptedChkStr,WHITE,LGRAY,0);				
			LCD_ShowHybridString(21,49,"异常:",WHITE,LGRAY,0);	
			LCD_ShowHybridString(82,49,"     条目",WHITE,LGRAY,0);		
			}		 
		LCD_ShowChinese(32,64,"按下",WHITE,LGRAY,0);
		LCD_ShowString(59,64,"ESC",YELLOW,LGRAY,12,0);
		LCD_ShowChinese(86,64,"以退出",WHITE,LGRAY,0);
		LogChkGUIRendered=true;
		}
	//正常检查中
	else if(LogEntryNum<RunTimeLoggerDepth)
	 {
	 LCD_ShowIntNum(54,35,LogEntryNum+1,3,WHITE,LGRAY,12);
	 if(!RunLogModule_VerifyDBHandler(LogEntryNum,&Result))
		{
		LogEntryNum=RunTimeLoggerDepth+2; //如果校验失败则直接跳转至Fail阶段
		LogChkGUIRendered=false;
		}
	 else
		{
		//正常通过检测，指示条目是否异常
    if(Result)BrokenNum++;		
	  LogEntryNum++;			
		LCD_ShowIntNum(54,49,BrokenNum,3,WHITE,LGRAY,12);
		}
	 }
  else if(LogEntryNum==RunTimeLoggerDepth)
	 {
	 //进行Guard Key的检测和修复	 
	 if(!CompareLogCacheKeyIsOK(&Result))
			{
			LogEntryNum=RunTimeLoggerDepth+2; //如果校验失败则直接跳转至Fail阶段
			LogChkGUIRendered=false;
			}	
	 //校验成功完成	 
	 else
			{					
			if(!Result)LogEntryNum=WriteLogCacheKeyArea()?LogEntryNum+1:LogEntryNum+2;
			else LogEntryNum++;         //如果Guard Key损坏则对数据进行修复
			LogChkGUIRendered=false;
			}
	 }
	}
	
void DBCheckInitHandler(void)
	{
	LogEntryNum=0;
	BrokenNum=0;
	LogChkGUIRendered=false;	
	}
	
const MenuConfigDef DatabaseCheckMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
  &DataBaseCheckHandler,
	&BackToResetMenu,	
	//不是设置菜单不需要用别的事情
	"日志数据库校验\0",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&DBCheckInitHandler,
	NULL
	};	
