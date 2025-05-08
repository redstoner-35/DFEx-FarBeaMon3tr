#include "GUI.h"
#include "LogSystem.h"
#include "ADC.h"
#include <string.h>
#include "Key.h"
#include "BalanceMgmt.h"
	
extern bool IsUpdateBalUI;
extern bool IsTimeMet;

//自动均衡渲染
void AutoBalMenuRenderHandler(void)
	{
	int H,M,S;
	extern int SleepTimer;	
	//检测是否需要渲染
	SleepTimer=480; //均衡运行期间禁止系统复位，休眠时间复位为一分钟	
	if(!(BalanceForceEnableTIM%8)&&!IsTimeMet)
		{
		IsUpdateBalUI=true; //每秒更新一次UI
		IsTimeMet=true; //只需要更新一次就行，避免重复更新
		LogData.BalanceTime++;
		}
	else if(BalanceForceEnableTIM%8)IsTimeMet=false; //非更新时间，进行更新		
	//时间到，回到主界面	
	if(BalanceForceEnableTIM<=0)
		{
		//保存日志
		LogData.UnbalanceBatteryAh=0; //本次均衡已完成	
		RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data); //计算运行日志的CRC32
		WriteRuntimeLogToROM(); //保存日志
		//回到主界面
		ClearScreen(); //清屏
		SwitchingMenu(&MainMenu);
		}
	if(!IsUpdateBalUI&&KeyState.KeyEvent==KeyEvent_None)return;
	//实际的渲染流程
	RenderMenuBG(); //显示背景	
	LCD_ShowChinese(33,22,"自动均衡运行中",GREEN,LGRAY,0);
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
	//渲染完毕，复位
	IsUpdateBalUI=false;
	}
	
//自动均衡按键处理
void AutoBalMenuKeyHandler(void)
	{
	if(KeyState.KeyEvent==KeyEvent_ESC||ADCO.Vbatt<10.1)
		{
		IsUpdateBalUI=true;
		BalanceForceEnableTIM=0;
		RunLogEntry.CurrentDataCRC=CalcRunLogCRC32(&RunLogEntry.Data); //计算运行日志的CRC32
		WriteRuntimeLogToROM(); //保存日志
		ClearScreen(); //清屏
	  SwitchingMenu(&MainMenu);
		}
	//清除按键事件
  KeyState.KeyEvent=KeyEvent_None;
	}
	
//启动均衡
void EnableAutoBal(void)	
	{
	IsUpdateBalUI=true;
	BalanceForceEnableTIM=3600*8*5; //5个小时
	}

//菜单配置
const MenuConfigDef AutoBALMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
  &AutoBalMenuRenderHandler,
	&AutoBalMenuKeyHandler,	
	//不是设置菜单不需要用别的事情
	"自动均衡修正\0",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&EnableAutoBal,
	NULL
	};
