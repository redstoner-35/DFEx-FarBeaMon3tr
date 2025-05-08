#include "GUI.h"
#include "Key.h"
#include "delay.h"

void ShutSysOFF(void);
void SetDebugPortState(bool IsEnable);

//自检错误处理
void SelfTestErrorHandler(void)
	{
	extern bool SensorRefreshFlag;
	extern bool IsEnablePowerOFF;
	//显示出错
	LCD_Fill(5,3,159,18,BLACK);
	LCD_Fill(33,16,123,42,BLACK);
	LCD_ShowChinese(5,3,"系统初始化失败！",RED,BLACK,0);
	LCD_ShowChinese(5,17,"按任意键重试或关闭系统",WHITE,BLACK,0);
	KeyState.KeyEvent=KeyEvent_None;
	SetDebugPortState(true); //出现故障的时候打开debug口
	while(1)
		{
		SideKey_LogicHandler(); 
		if(KeyState.KeyEvent!=KeyEvent_None) //按下任意键关机
			{
			IsEnablePowerOFF=true;
			KeyState.KeyEvent=KeyEvent_None;
			ShutSysOFF();
			}
		if(!SensorRefreshFlag)continue;
		SideKey_TIMCallback(); 
		SensorRefreshFlag=false;
		}
	}
