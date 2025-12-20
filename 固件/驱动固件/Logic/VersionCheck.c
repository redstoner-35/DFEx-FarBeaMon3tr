/****************************************************************************/
/** \file VersionCheck.c
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个文件为顶层应用层逻辑文件。该文件实现了查询系统固件编译日期
以确定build版本的功能。

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "ModeControl.h"
#include "VersionCheck.h"
#include "SideKey.h"

/****************************************************************************/
/*	Local constant definitions('static const')
****************************************************************************/	

/*********** 固件时间戳 ***********
固件时间戳包含固件编译的年，月，日以
及24小时制时间和分钟。
**********************************/
static code char TimeStamp[]={"25 12 19-13 35"};

/****************************************************************************/
/*	Local variable definitions('static')
****************************************************************************/	
static xdata unsigned char VersionIndex=0; 						//版本号字符串index
static xdata unsigned char VersionShowFastStrobeTIM;  //快速闪烁提示计时器

/****************************************************************************/
/*	Global & Extern variable and Flag definitions('extern')
*****************************************************************************/

//通用软件计时变量
extern xdata unsigned char CommonSysFSMTIM;

//版本检查系统状态变量
xdata VersionChkFSMDef VChkFSMState=VersionCheck_InAct;

/****************************************************************************/
/* Global Function implementation - Logic & Timing Handler
*****************************************************************************/

//启动显示流程
void VersionCheck_Trigger(void)
	{
	//把状态机配置为init状态开始显示（仅空闲状态）
	if(VChkFSMState==VersionCheck_InAct)
		{
		VChkFSMState=VersionCheck_StartInit;
		CommonSysFSMTIM=15;  //初始点亮一下下
		}
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
			if(CommonSysFSMTIM>6)return 1;
		  //等待时间到
      if(CommonSysFSMTIM)break;
			VersionIndex=0;
			VersionShowFastStrobeTIM=0;
		  VChkFSMState=VersionCheck_LoadNextNumber; //加载数字开始显示
			break;
		//加载下一个数字
		case VersionCheck_LoadNextNumber:
			if(TimeStamp[VersionIndex]=='-'||TimeStamp[VersionIndex]==' ')
				{
				//检测到横杠，停顿4.5秒,如果是空格则停顿2.5秒
				VChkFSMState=VersionCheck_ShowNumberWait;
				if(TimeStamp[VersionIndex]=='-')CommonSysFSMTIM=36;
				else CommonSysFSMTIM=20;
				}
			else //其余字符，正常加载
				{
				buf=TimeStamp[VersionIndex]&0x0F; //ASCII码转实际数值
				if(!buf)VersionShowFastStrobeTIM=50; //为0，设置快速闪烁计时器闪一下
				else CommonSysFSMTIM=(buf*4)-1; //非0值，按照数字大小配置显示的时长
				VChkFSMState=VersionCheck_ShowNumber;
				}
			//指向下一个字符
			VersionIndex++;
			break;
		//显示数字
		case VersionCheck_ShowNumber:
			if(!VersionShowFastStrobeTIM)
				{
				//慢闪显示计时器计时结束，产生1.5秒间隔并跳转至数字加载阶段
				if(!CommonSysFSMTIM)
					{
					CommonSysFSMTIM=12;
					VChkFSMState=VersionCheck_ShowNumberWait;
					break;
					}
				//正常开始显示
				if((CommonSysFSMTIM%4)&0x7E)return 1;
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
			if(CommonSysFSMTIM)break;
		   //本次显示的字符已经是最后一个了(下个字符是NULL)，显示结束了
	    if(TimeStamp[VersionIndex]=='\0')VChkFSMState=VersionCheck_InAct;
	    //本次字符显示结束但是还有字符，并准备加载下一组数字
		  else VChkFSMState=VersionCheck_LoadNextNumber;  
		  break;
		}
	//默认使灯珠熄灭，返回0
	return 0;
	}
/*********************************  End Of File  ************************************/
