#ifndef _WD_
#define _WD_

//重载参数
#define WDTReloadSec 2

//喂狗宏定义
#define WatchDog_Feed() WDT_Restart()

//函数
void WatchDog_Init(void); //启动看门狗模块

#endif
