#include "cms8s6990.h"
#include "LVDCtrl.h"

//启动低电压检测模块
void LVD_Start(void)
	{	
  //配置WUT唤醒时间=8uS*256*4096=8388mS
	WUTCRL=0xFF;
	WUTCRH=0xBF;
	}

//关闭低电压检测
void LVD_Disable(void)
	{
	WUTCRH=0;
	WUTCRL=0;
	}