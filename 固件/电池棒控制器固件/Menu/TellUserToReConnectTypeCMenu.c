#include "GUI.h"
#include "Key.h"

//内部变量
static bool INFOUIIsRendered=false;

//函数声明
bool SetSystemDischargeState(void);

//提示页面的显示处理
void InfoUserToRemoveCPort(void)
	{
	if(INFOUIIsRendered)return;
	RenderMenuBG();
	LCD_ShowHybridString(10,22,"请拔下Type-C口的电缆",YELLOW,LGRAY,0);	
	LCD_ShowHybridString(10,36,"并在本提示消失后重新",YELLOW,LGRAY,0);	
	LCD_ShowHybridString(10,49," 将电缆连接至端口。",YELLOW,LGRAY,0);	
	INFOUIIsRendered=true;
	}

//提示页面的按键处理
void InfoUserRMTCKeyHandler(void)
	{
	extern bool IsCPortConnected;
	extern bool Is2366Telem;
	//启用测量
	Is2366Telem=true;
	//C口断开连接后，自动退出
	if(!IsCPortConnected||KeyState.KeyEvent==KeyEvent_BothEnt)
		{
		ClearScreen(); //清屏
		SetSystemDischargeState(); //重新使能DCDC
		SwitchingMenu(&MainMenu);
		}
	//清除按键事件处理
	KeyState.KeyEvent=KeyEvent_None;	
	}
	
void ResetCPortINFOUI(void)	
	{
	INFOUIIsRendered=false;
	}

//菜单配置
const MenuConfigDef InfoUserRemoveCCableMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
  &InfoUserToRemoveCPort,
	&InfoUserRMTCKeyHandler,	
	//不是设置菜单不需要用别的事情
	"提示\0",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&ResetCPortINFOUI,
	NULL
	};
