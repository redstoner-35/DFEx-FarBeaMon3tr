#include "ht32.h"
#include "WatchDog.h"

//启动看门狗模块
void WatchDog_Init(void)
	{
	//复位WDT
  WDT_DeInit();
	//设置时钟源和分频器	
	RTC_LSILoadTrimData();
  WDT_SourceConfig(WDT_SOURCE_LSI); //使用LSI作为时钟
	WDT_SetPrescaler(WDT_PRESCALER_64);	 //设置分频值为 32K/64 = 500 Hz
	//设置重载值	
	WDT_SetReloadValue(500*WDTReloadSec); //设置WDT重载计数器值，2秒后才重载
	WDT_SetDeltaValue((500*WDTReloadSec)+10);	 //增量检测功能关闭，解决退出适配器模拟时系统重启的bug
	WDT_Restart(); //对计数器进行重载
  //启动复位指令和看门狗本体
	WDT_ResetCmd(ENABLE);
  WDT_Cmd(ENABLE);          
  //锁定看门狗配置避免配置被意外更改		
  WDT_ProtectCmd(ENABLE);           
	}
