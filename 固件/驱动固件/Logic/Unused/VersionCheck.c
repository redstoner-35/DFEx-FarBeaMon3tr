#include "ModeControl.h"
#include "VersionCheck.h"
#include "SideKey.h"

//固件时间戳
static code char TimeStamp[]={"250716-1719\0"};

//内部变量
static xdata unsigned char VersionShowTIM;  //电压显示计时器
static xdata unsigned char VersionIndex=0; //版本号字符串index
static xdata unsigned char VersionShowFastStrobeTIM;  //快速闪烁提示计时器
xdata VersionChkFSMDef VChkFSMState=VersionCheck_InAct;
	
//启动显示流程
void VersionCheck_Trigger(void)
	{
	//把状态机配置为init状态开始显示（仅空闲状态）
	if(VChkFSMState==VersionCheck_InAct)
		{
		VChkFSMState=VersionCheck_StartInit;
		VersionShowTIM=15;  //初始点亮一下下
		}
	}
	
void VersionCheck_TIMHandler(void)
	{
	//很简单的倒计时处理
	if(VersionShowTIM)VersionShowTIM--;
	}

//显示模块状态机处理
char VersionCheckFSM(void)
	{
	unsigned char buf;
	switch(VChkFSMState)
		{
		//显示系统未激活
		case VersionCheck_InAct:break;
		//初始化显示系统	
		case VersionCheck_StartInit:	
			//初始提示点亮0.5秒左右表示开始播报
			if(VersionShowTIM>6)return 1;
		  //等待时间到
      if(VersionShowTIM)break;
			VersionIndex=0;
			VersionShowFastStrobeTIM=0;
		  VChkFSMState=VersionCheck_LoadNextNumber; //加载数字开始显示
			break;
		//加载下一个数字
		case VersionCheck_LoadNextNumber:
			if(TimeStamp[VersionIndex]=='-')
				{
				//检测到横杠，停顿3秒
				VChkFSMState=VersionCheck_ShowNumberWait;
				VersionShowTIM=24;
				}
			else //其余字符，正常加载
				{
				buf=TimeStamp[VersionIndex]&0x0F; //ASCII码转数字
				if(!buf)VersionShowFastStrobeTIM=60; //为0，设置快速闪烁计时器闪一下
				else VersionShowTIM=(4*(buf&0x0F))-1; //非0值，按照数字大小配置显示的时长
				VChkFSMState=VersionCheck_ShowNumber;
				}
			//指向下一个字符
			VersionIndex++;
			break;
		//显示数字
		case VersionCheck_ShowNumber:
			if(!VersionShowFastStrobeTIM)
				{
				//非0值正常按照设置参数调整
				if(!VersionShowTIM)
					{
					//本次显示的字符已经是最后一个了，等待用户放开按键后退出
					if(TimeStamp[VersionIndex]=='\0')
						{
						VChkFSMState=VersionCheck_WaitUserRelease;
						break;
						}
					//显示结束，产生10秒的消隐间隔并准备加载下一组数字
					VersionShowTIM=10;
					VChkFSMState=VersionCheck_ShowNumberWait;
					}
				//正常开始显示
				if((VersionShowTIM%4)&0x7E)return 1;
				}
		  else 
				{
				//0值则快速利用主循环对快速闪烁计时器进行累减，产生很短的闪烁
				VersionShowFastStrobeTIM--;
				return 1;
				}
		  break;
    //等待数字之间的间隔
		case VersionCheck_ShowNumberWait:
			if(VersionShowTIM)break;
		  VChkFSMState=VersionCheck_LoadNextNumber;  
		  break;
		//等待用户放开按键
		case VersionCheck_WaitUserRelease:
			if(!getSideKeyNClickAndHoldEvent())VChkFSMState=VersionCheck_InAct;
		  break;
		}
	//默认使灯珠熄灭，返回0
	return 0;
	}