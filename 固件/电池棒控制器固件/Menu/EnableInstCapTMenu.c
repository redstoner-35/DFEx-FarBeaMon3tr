#include "Config.h"
#include "Key.h"
#include "GUI.h"

static bool IsRenderICTestMode=false;

//已启动至测容模式
void DisplaySuccEnteredCapStart(void)
	{
	if(IsRenderICTestMode)return;
	RenderMenuBG();
	LCD_ShowChinese(14,21,"已激活一次性测容模式",GREEN,LGRAY,0);
	LCD_ShowChinese(7,36,"下次启动时若符合条件，",WHITE,LGRAY,0);
	LCD_ShowChinese(7,49,"则系统将会自动开始测容",WHITE,LGRAY,0);
	//指示按下什么键
	LCD_ShowChinese(32,64,"按下",WHITE,LGRAY,0);
	LCD_ShowString(59,64,"ESC",YELLOW,LGRAY,12,0);
	LCD_ShowChinese(86,64,"以退出",WHITE,LGRAY,0);		
	//渲染完成
	IsRenderICTestMode=true;
	}
	
void LeaveICTMenu(void)
	{
	if(KeyState.KeyEvent==KeyEvent_ESC)
		{
		if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
		else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
		}	
	KeyState.KeyEvent=KeyEvent_None;
	}	
	
void EnableICTestMode(void)
	{
	CfgData.InstantCTest=InstantCTest_Armed;
	WriteConfiguration(&CfgUnion,false);
	IsRenderICTestMode=false;
	}

const MenuConfigDef ActOneShotCTestMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&DisplaySuccEnteredCapStart, 
	&LeaveICTMenu, //按键处理
	//不是设置菜单不需要用别的事情
	"激活一次性测容",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&EnableICTestMode, //进入时配置好参数
	NULL 
	};
