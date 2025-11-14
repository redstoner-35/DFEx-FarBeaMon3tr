#include "GUI.h"
#include "CapTest.h"
#include "Config.h"
#include <math.h>
#include <string.h>
#include "Key.h"
#include <stdio.h>

static bool AutoSetRenderOK=false;

//执行自动的预充电电流update
void PreChargeCurrentAutoUpdate(void)
	{
	float PreChargeCurrent;	
	int ibuf;
	//根据Wh等效换算mAh数
	PreChargeCurrent=LastCData.TotalWh/(3.6*BATTCOUNT);
	PreChargeCurrent*=1000;                               //得到mAh数
		
	//如果根据Wh换算出的电池等效mAh数小于Ah测出的总容量，则使用Ah的结果	
	if(PreChargeCurrent<LastCData.TotalmAH)PreChargeCurrent=LastCData.TotalmAH/10;	
	else PreChargeCurrent/=10;                           
	//限制预充电流结果在0.1至1500mA之间
	if(PreChargeCurrent<100)PreChargeCurrent=100;
	if(PreChargeCurrent>1500)PreChargeCurrent=1500;
  //对预充电流结果进行50mA的归一化处理和取整
	ibuf=(int)PreChargeCurrent;      	
	CfgData.InputConfig.PreChargeCurrent=ibuf/50;  //先处理50mA的部分	
	ibuf=((int)PreChargeCurrent)%50;                 //求余数
	if(ibuf>25)CfgData.InputConfig.PreChargeCurrent++; //如果剩余的余数部分大于LSB的一半，则给结果四舍五入加50mA	
	CfgData.InputConfig.PreChargeCurrent*=50;         //把50mA LSB值换算为实际电流
	//归一化取整后再次限制结果
	if(CfgData.InputConfig.PreChargeCurrent<100)CfgData.InputConfig.PreChargeCurrent=100;
	if(CfgData.InputConfig.PreChargeCurrent>1500)CfgData.InputConfig.PreChargeCurrent=1500;	
	}

void PreChargeAutoSetRender(void)
	{
	char Sbuf[64];
	if(AutoSetRenderOK)return;
	RenderMenuBG();
	if(LastCData.IsDataValid)
		{
    PreChargeCurrentAutoUpdate();
		LCD_ShowHybridString(27,20,"已完成预充电流的",GREEN,LGRAY,0);
		LCD_ShowHybridString(27,34," 自动设置流程。",GREEN,LGRAY,0);
		memset(Sbuf,0,sizeof(Sbuf));	
		snprintf(Sbuf,sizeof(Sbuf)-1,"预充电流:%dmA",CfgData.InputConfig.PreChargeCurrent);		
		LCD_ShowHybridString(27,47,Sbuf,WHITE,LGRAY,0);	
		}		
	else
		{
		LCD_ShowHybridString(27,20,"预充电流自动设置",RED,LGRAY,0);
		LCD_ShowHybridString(27,34," 流程无法继续!",RED,LGRAY,0);
		LCD_ShowHybridString(27,47,"请完成一次测容。",YELLOW,LGRAY,0);
		}
	ShowPressExitToLeave();
	//渲染结束
	AutoSetRenderOK=true;
	}
	
void ResetPreChgAutoSet(void)
	{
	AutoSetRenderOK=false;
	}
	
void AutoSetKeyHandler(void)
	{
	//按下ESC回到充电系统菜单
	if(KeyState.KeyEvent==KeyEvent_ESC)SwitchingMenu(&ChgSysSetMenu);
	//无视其余事件
	KeyState.KeyEvent=KeyEvent_None;
	}
	
const MenuConfigDef PreChargeAutoSetMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
  &PreChargeAutoSetRender,
	&AutoSetKeyHandler,	
	//不是设置菜单不需要用别的事情
	"预充电流自动设置\0",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&ResetPreChgAutoSet,
	NULL
	};
	
