#include "GUI.h"
#include "Config.h"
#include "ADC.h"
#include "LogSystem.h"
#include <math.h>
#include <string.h>
#include "Key.h"
#include "BalanceMgmt.h"

//菜单内部状态宏定义
typedef enum
	{
	BalMenu_SetTime,
	BalMenu_Running,
	BalMenu_Finished,
	BalMenu_Failed,	
	}BalMenuState;

//内部全局
static int BalanceHour;
static BalMenuState BalState;
	
//全局	
bool IsUpdateBalUI;
bool IsTimeMet;
	
//均衡菜单状态机状态机处理
void BalMenuFSMProcess(void)
	{
	extern int SleepTimer;
	switch(BalState)
		{
		case BalMenu_SetTime:
			 if(KeyState.KeyEvent==KeyEvent_Up&&BalanceHour<10)BalanceHour++;
		   if(KeyState.KeyEvent==KeyEvent_Down&&BalanceHour>1)BalanceHour--; //增减均衡时间
		   if(KeyState.KeyEvent==KeyEvent_ESC)
					{
					if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
					else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
					}
			 if(KeyState.KeyEvent==KeyEvent_Enter)
					{
					BalanceForceEnableTIM=3600*8*BalanceHour;
					BalState=BalMenu_Running;
					IsUpdateBalUI=true;
					}
			 break;
	  //均衡运行中
		case BalMenu_Running:
			  SleepTimer=480; //均衡运行期间禁止系统复位，休眠时间复位为一分钟
				if(KeyState.KeyEvent==KeyEvent_ESC||ADCO.Vbatt<10.1)
					{
					IsUpdateBalUI=true;
					BalanceForceEnableTIM=0;
					BalState=BalMenu_Failed;
					break;
					}
				if(BalanceForceEnableTIM>0)
					{
					if(!(BalanceForceEnableTIM%8)&&!IsTimeMet)
						 {
						 IsUpdateBalUI=true; //每秒更新一次UI
						 IsTimeMet=true; //只需要更新一次就行，避免重复更新
						 LogData.BalanceTime++; //均衡器运行期间累加时间
						 }
					else if(BalanceForceEnableTIM%8)IsTimeMet=false; //非更新时间，进行更新
					break;
					}
				else //均衡已结束，关闭均衡器
					{
					//保存日志清除未均衡充放电容量
					LogData.UnbalanceBatteryAh=0; //本次均衡已完成	
					RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data); //计算运行日志的CRC32
					WriteRuntimeLogToROM(); //保存日志
					//跳转到均衡结束的界面
					IsUpdateBalUI=true;
					BalState=BalMenu_Finished;
					}
				break; 
		//完成和错误状态，按下ESC退出
		case BalMenu_Finished:
		case BalMenu_Failed:
			 if(KeyState.KeyEvent!=KeyEvent_ESC)break;
	     RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data); //计算运行日志的CRC32
			 WriteRuntimeLogToROM(); //保存日志
			 if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
			 else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
			 break;	  
		}
	//按键响应完毕清除事件
	if(KeyState.KeyEvent!=KeyEvent_None)IsUpdateBalUI=true; //UI发生更新，重新绘制
	KeyState.KeyEvent=KeyEvent_None;
	}

//手动均衡菜单的GUI渲染	
void BalMenuGUIHandler(void)
	{
	int H,M,S;
	if(!IsUpdateBalUI&&KeyState.KeyEvent==KeyEvent_None)return;
	RenderMenuBG(); //显示背景
	switch(BalState)
		{
		//设置时间
		case BalMenu_SetTime:	
			LCD_ShowChinese(35,21,"请指定均衡时间",WHITE,LGRAY,0);
			LCD_ShowChinese(32,42,"共计",WHITE,LGRAY,0);
		  LCD_ShowIntNum(54,42,BalanceHour,2,YELLOW,LGRAY,12);
		  LCD_ShowChinese(70,42,"均衡小时",WHITE,LGRAY,12);
			LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
		  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
		  break;
		//均衡运行中
		case BalMenu_Running:
		  LCD_ShowChinese(33,22,"手动均衡运行中",GREEN,LGRAY,0);
		  H=BalanceForceEnableTIM/8;
		  M=(H%3600)/60;
		  S=H%60;
		  H/=3600;
			LCD_ShowHybridString(14,40,"剩余:",WHITE,LGRAY,0);
		  LCD_ShowIntNum(51,40,H,1,GREEN,LGRAY,12);
		  LCD_ShowChinese(62,40,"时",WHITE,LGRAY,12);
		  LCD_ShowIntNum(78,40,M,2,GREEN,LGRAY,12);
		  LCD_ShowChinese(98,40,"分",WHITE,LGRAY,12);
		  LCD_ShowIntNum(114,40,S,2,GREEN,LGRAY,12);
		  LCD_ShowChinese(134,40,"秒",WHITE,LGRAY,12);
		  LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
		  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
		  break;
		//手动均衡完成	
    case BalMenu_Finished:			
			LCD_ShowChinese(33,22,"手动均衡已完成",GREEN,LGRAY,0);
			LCD_ShowChinese(32,41,"共计",WHITE,LGRAY,0);
		  LCD_ShowIntNum(54,41,BalanceHour,2,GREEN,LGRAY,12);
		  LCD_ShowChinese(70,41,"均衡小时",WHITE,LGRAY,12);
		  LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
		  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
		  break;
		//均衡失败
		case BalMenu_Failed:	
			LCD_ShowChinese(28,22,"手动均衡异常结束",RED,LGRAY,0);
			LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
			LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
			LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
		  break;
		}
	IsUpdateBalUI=false;
	}
	
//进入和退出重置系统的时候进行操作
void ResetManuBalModule(void)
	{
	BalanceForceEnableTIM=0;
	BalanceHour=1;
	IsUpdateBalUI=true;
	BalState=BalMenu_SetTime;
	}	
	
//菜单配置
const MenuConfigDef BALTestMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
  &BalMenuGUIHandler,
	&BalMenuFSMProcess,	
	//不是设置菜单不需要用别的事情
	"手动启动均衡\0",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&ResetManuBalModule,
	&ResetManuBalModule
	};
