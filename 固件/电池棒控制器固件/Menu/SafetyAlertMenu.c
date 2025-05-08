
#include "Config.h"
#include "Key.h"
#include "GUI.h"

static bool IsRenderSafeAlert=false;

//已启动至测容模式
void DisplayAlertMsg(void)
	{
	if(IsRenderSafeAlert)return;
	RenderMenuBG();
	LCD_ShowChinese(8,20,"错误的充电系统配置将会",YELLOW,LGRAY,0);
	LCD_ShowChinese(8,34,"引起电池起火或爆炸并造",YELLOW,LGRAY,0);
	LCD_ShowChinese(21,48,"成严重人身伤害！",YELLOW,LGRAY,0);	
	LCD_ShowChinese(28,64,"您确定要继续吗？",RED,LGRAY,0);	
	//渲染完成
	IsRenderSafeAlert=true;
	}
	
void LeaveSafeAlmMenu(void)
	{
	if(KeyState.KeyEvent==KeyEvent_BothEnt)SwitchingMenu(&ChgSysSetMenu);
	else if(KeyState.KeyEvent!=KeyEvent_None)SwitchingMenu(&SetMainMenu); //其余任意操作回到主菜单
	KeyState.KeyEvent=KeyEvent_None;
	}	
	
void EnterSafeAlmMode(void)
	{
	IsRenderSafeAlert=false;
	}

const MenuConfigDef SafeAlmMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&DisplayAlertMsg, 
	&LeaveSafeAlmMenu, //按键处理
	//不是设置菜单不需要用别的事情
	"安全警告",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&EnterSafeAlmMode, //进入时配置好参数
	NULL 
	};
