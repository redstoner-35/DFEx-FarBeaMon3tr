#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"
#include "CapTest.h"
#include "Key.h"
#include "LogSystem.h"

bool IsResetRendered=false;
static bool IsColResetStart=false;
static bool ColResetResult=false;

typedef enum
	{
	Reset_SysConfig,
	Reset_CapTest,
	Reset_ColumbGauge
	}ResetDoneTypeDef;
	
//选择复位哪个
ResetDoneTypeDef RSTType;	

void DisplayResetOK(void)
	{
	if(IsResetRendered)return;
	RenderMenuBG(); //显示背景
	if(RSTType==Reset_ColumbGauge&&!IsColResetStart)	
		{
		ColResetResult=true;
		LCD_ShowChinese(28,23,"正在重置库仑计....",WHITE,LGRAY,0);//正在重置
		if(LogData.DischargeTime)ColResetResult=ResetRunTimeLogArea();; 
		IsColResetStart=true;
		return;
		}
	if(RSTType==Reset_ColumbGauge&&!ColResetResult&&IsColResetStart)
		{
		LCD_ShowChinese(28,23,"库仑计重置失败",RED,LGRAY,0);
		IsResetRendered=true;
		return;
		}
	switch(RSTType)
		{
		case Reset_SysConfig:LCD_ShowChinese(21,23,"已将系统设置恢复为",WHITE,LGRAY,0);break;
		case Reset_CapTest:LCD_ShowChinese(21,23,"已将测容数据恢复为",WHITE,LGRAY,0);break;
		case Reset_ColumbGauge:LCD_ShowChinese(28,23,"已将库仑计恢复为",WHITE,LGRAY,0);break;
		}
	LCD_ShowChinese(47,37,"出厂默认值",WHITE,LGRAY,0);
	LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
	LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
	//已恢复出厂设置
	IsResetRendered=true;
	}

//回到重置出厂设置的主菜单
void BackToResetMenu(void)
	{
	//没有按下
	if(KeyState.KeyEvent!=KeyEvent_ESC)return;	
	if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
	else SwitchingMenu(&RSTMainMenu); //处于退出状态,按下ESC后回到主菜单	
	KeyState.KeyEvent=KeyEvent_None;
	IsResetRendered=false;
	}
//复位库仑计
void ResetColumData(void)
	{
	RSTType=Reset_ColumbGauge;
	IsResetRendered=false;
	IsColResetStart=false;
	}

const MenuConfigDef ResetColMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
  &DisplayResetOK,
	&BackToResetMenu,	
	//不是设置菜单不需要用别的事情
	"重置库仑计数据\0",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&ResetColumData,
	NULL
	};	
	
//复位测容系统
void SelectResetCTest(void)
	{
	RSTType=Reset_CapTest;
	ClearHistoryData();
	IsResetRendered=false;
	WriteCapData(&CTestData.ROMImage.Data,false);
	}
	
const MenuConfigDef ResetCTestMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
  &DisplayResetOK,
	&BackToResetMenu,	
	//不是设置菜单不需要用别的事情
	"重置测容系统数据\0",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&SelectResetCTest,
	NULL
	};

//复位系统配置	
void SelectResetConfig(void)
	{
	RSTType=Reset_SysConfig;
	RestoreDefaultConfig();
	IsResetRendered=false;
	WriteConfiguration(&CfgUnion,false);
	}	

const MenuConfigDef ResetSysConfigtMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
  &DisplayResetOK,
	&BackToResetMenu,	
	//不是设置菜单不需要用别的事情
	"重置系统设置\0",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&SelectResetConfig,
	NULL
	};
