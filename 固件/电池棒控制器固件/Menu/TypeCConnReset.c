#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"
#include "Key.h"

typedef enum
	{
	TCResetFSM_Wait,
	TCResetFSM_Break,
	TCResetFSM_WaitForSomeTime,
	TCResetFSM_MakeConnect,
	TCResetFSM_Success,
	TCResetFSM_Failed,
	}TCFSMStateDef;

//内部变量
TCFSMStateDef TCFSMState=TCResetFSM_Wait; //TypeC打断状态机
static char TCFSMWait=0;
static bool IsTCResetUpdated=false;
	
//TypeC重置连接状态机
void TCResetFSM(void)
	{
	TypeCRoleDef Role;
	extern bool IsEnableAdapterEmu;
	extern bool IsSystemOverheating;
	//状态机	
	switch(TCFSMState)
		{
		case TCResetFSM_Wait:break;
	  case TCResetFSM_Break: //打断连接
			 if(!IP2366_SetTypeCRole(TypeC_Disconnect))
				 {
				 TCFSMState=TCResetFSM_Failed;
				 IsTCResetUpdated=false;
				 }
	     else
					{
				  TCFSMWait=50;
				  TCFSMState=TCResetFSM_WaitForSomeTime;
					}
		   break;
		case TCResetFSM_WaitForSomeTime: //等一会
		   if(TCFSMWait>0)TCFSMWait--;
		   else TCFSMState=TCResetFSM_MakeConnect; //准备重连
		   break;
		case TCResetFSM_MakeConnect:
			//根据系统状态确定角色
		  IP2366_ClearOCFlag();
			if(IsSystemOverheating)Role=TypeC_Disconnect;	//系统过热，关闭充放电
			else if(IsEnableAdapterEmu)Role=TypeC_UFP;	//开启适配器模拟，开启放电关闭充电
			else Role=CfgData.OutputConfig.IsEnableOutput?TypeC_DRP:TypeC_DFP;
		  //根据确定的角色决定Type-C模式
		  if(!IP2366_SetTypeCRole(Role))
					TCFSMState=TCResetFSM_Failed;
	     else
		      TCFSMState=TCResetFSM_Success;
			 IsTCResetUpdated=false;
			 break;
		//其余状态
		case TCResetFSM_Success:
		case TCResetFSM_Failed:break;
		}
	}
	
//TypeC重连渲染函数
void TypeCResetRender(void)
	{
	if(IsTCResetUpdated)return;
	RenderMenuBG();
	switch(TCFSMState)
		{
		case TCResetFSM_Success:
			LCD_ShowString(24,26,"Type-C",GREEN,LGRAY,12,0);
			LCD_ShowChinese(67,26,"链路已重置",GREEN,LGRAY,0);  
			LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
			LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
			LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
		  break;
		case TCResetFSM_Failed:
			LCD_ShowString(19,26,"Type-C",RED,LGRAY,12,0);
			LCD_ShowChinese(63,26,"链路重置失败",RED,LGRAY,0);  
			LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
			LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
			LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);		
			break;
		default:
			LCD_ShowChinese(18,32,"正在重置",WHITE,LGRAY,0);
		  LCD_ShowString(70,32,"Type-C",WHITE,LGRAY,12,0);
		  LCD_ShowChinese(114,32,"链路",WHITE,LGRAY,0);
			LCD_ShowChinese(46,47,"请稍后……",WHITE,LGRAY,0);
		  break;
		}
	//渲染完毕
	IsTCResetUpdated=true;
	}
	
void TypeCResetKeyHandler(void)
	{
	if(KeyState.KeyEvent!=KeyEvent_ESC)return;
	if(TCFSMState==TCResetFSM_Success||TCFSMState==TCResetFSM_Failed)
		{
		TCFSMState=TCResetFSM_Wait; //回到等待状态
		if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
		else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
		}
	KeyState.KeyEvent=KeyEvent_None;
	}	
	
void EnterTCResetMenu(void)
	{
	TCFSMWait=0;
	IsTCResetUpdated=false;
	TCFSMState=TCResetFSM_Break;
	}
	
const MenuConfigDef TCResetMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&TypeCResetRender, 
	&TypeCResetKeyHandler, //按键处理
	//不是设置菜单不需要用别的事情
	"Type-C链路重置",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&EnterTCResetMenu, //进入时配置好参数
	NULL 
	};
