#ifndef _GUI_
#define _GUI_

//内部包含
#include "lcd.h"
#include "LCD_Init.h"
#include <stdbool.h>
#include <stdlib.h>

typedef enum
	{
	Msg_Statu, //状态信息
	Msg_Warning, //警告
	Msg_POSTOK, //自检结束
	Msg_Fault  //故障
	}MessageTypeDef;

typedef struct
	{
	int *Source; //欲更改的数值来源
	int min;  //最小值
	int max;  //最大值
	int Step; //每次按键的变更的数值
	char *Unit; //单位
	char *MinName;
	char *MaxName; //进度条最大和最小数值的名称
	void (*ThingsToDoWhenExit)(void); //执行退出动作时的处理
	}intEditMenuCfg;

typedef struct
	{
	char *SelName; //该菜单项的名称
	bool IsChinese; //是否为中文
	bool *ValueSource; //指针，布尔值来源
	bool IsPlaceHolder; //是否为占位符
	bool IsMaster;  //字母菜单功能，当被分配为Master的对象设置为OFF时所有的Slave对象无法被编辑，一个菜单只能存在一个Master
	}BoolListEntryDef;


typedef struct
	{
	const char PassWord[4]; //密码(只能是数字),采用LSB存储
	void (*ThingsToDoWhenEnter)(void); //按下确认后要做的事情
	void (*ThingsToDoWhenExit)(void); //按下退出后要做的事情
	}PasswordInputDef;
	
typedef struct
	{
	char *SelName; //该菜单项的名称
	bool IsChinese; //是否为中文
	int EnumValue; //该菜单项对应的enum值
	bool IsPlaceHolder; //是否为占位符
	}EnumEditEntryDef;

typedef struct
	{
	char *SelName; //该菜单项的名称
	bool IsPlaceHolder; //是否为占位符
	bool *IsCanBeSelect; //是否可以被选中
	//进入前需要进行的处理
  void (*ThingsToDoBeforeEnter)(void);
	//退出后回到的菜单
	}SetupMenuSelDef;	

typedef enum
	{
	MenuType_Custom, //自定义菜单，使用自定义渲染和按键处理
	MenuType_Setup, //设置类菜单，使用模板
	MenuType_EnumSetup, //枚举数值编辑类菜单
	MenuType_BoolListSetup, //Bool列表的设置
	}MenuTypeDef;	
	
typedef struct
	{
	MenuTypeDef Type;
	//布尔list类的编辑
  const BoolListEntryDef *BoolEntry; //布尔list的入口		
	//枚举类编辑模式下的转换函数
  const EnumEditEntryDef *Entry;  //所有选项的入口		
	int (*ReadEnumSource)(void); //进入菜单时读取enum的入口
	void (*FedEnumToWhat)(int); //正常保存时将enum的目标值写到哪的函数
	//自定义模式下的渲染函数
	void (*CustomMenuRender)(void); //自定义渲染函数
	void (*CustomKeyProcess)(void); //自定义按键响应函数
	//设置菜单模式下所用
	char *MenuTitle; //设置菜单的名称
	const SetupMenuSelDef *Sel; //可用的选项
	void (*AdditionalRender)(void); //菜单中的额外渲染
	void (*ThingsToDoWhenExit)(void); //退出菜单动作执行之后需要干的事情
	//进入和退出该菜单之前需要做的事情
	void (*ThingsToDoBeforeEnter)(void); 
	void (*ThingsToDoBeforeLeave)(void); //自定义按键响应函数	
	}MenuConfigDef;	
	
//菜单entry
extern const MenuConfigDef SafeAlmMenu;
extern const MenuConfigDef MainMenu;
extern const MenuConfigDef SetMainMenu;
extern const MenuConfigDef PowerSetMenu;	
extern const MenuConfigDef LVSetMenu;
extern const MenuConfigDef DisChgCfgMenu;	
extern const MenuConfigDef ChgSysSetMenu;	
extern const MenuConfigDef CapTestMenu;
extern const MenuConfigDef RSTMainMenu;
extern const MenuConfigDef ResetCTestMenu;
extern const MenuConfigDef ResetSysConfigtMenu;	
extern const MenuConfigDef CapTestHisMenu;	
extern const MenuConfigDef IChargeSetMenu;	
extern const MenuConfigDef PreChargeISetMenu;
extern const MenuConfigDef ChgVSetMenu;	
extern const MenuConfigDef ColHisMenu;
extern const MenuConfigDef ResetColMenu;	
extern const MenuConfigDef ChipStatMenu;	
extern const MenuConfigDef EasySetMainMenu;
extern const MenuConfigDef EnterAdvancedMenu;	
extern const MenuConfigDef PDOCfgMenu;	
extern const MenuConfigDef SecuCfgMenu;	
extern const MenuConfigDef EnterSecuMenu;	
extern const MenuConfigDef AboutMenu;	
extern const MenuConfigDef TSetMenu;	
extern const MenuConfigDef AdapterEmuMenu;	
extern const MenuConfigDef TCResetMenu;
extern const MenuConfigDef DisPlayDirMenu;
extern const MenuConfigDef ActOneShotCTestMenu;
extern const MenuConfigDef RechargeSetMenu;
extern const MenuConfigDef IstopSetMenu;
extern const MenuConfigDef MaxVPDMenu;
extern const MenuConfigDef PowerSetMenuNoEPR;
extern const MenuConfigDef LargeMainMenu;
extern const MenuConfigDef GUIPrefMenu;
extern const MenuConfigDef BalSysSetMenu;
extern const MenuConfigDef BALTestMenu;
extern const MenuConfigDef AutoBALMenu;
extern const MenuConfigDef IstopStdSetMenu;
extern const MenuConfigDef DisChgCfgMenuNoHSCP; //放电系统配置（没有高压HSCP）

//内部数值增减管理
int IntIncDec(int ValueIN,int Min,int Max,int PerStep); //
float FloatIncDec(float ValueIN,float Min,float Max,float PerStep); //GUI里面进行浮点数递增和递减
int iroundf(float IN); //浮点数四舍五入为整数

//密码输入菜单
void PassWordMenuRender(const PasswordInputDef *CFG);
void PasswordEnterInit(void);

//整数编辑菜单
void IntEditMenuKeyEffHandler(void);	//实现菜单按键按下特效的函数
void IntEditHandler(const intEditMenuCfg *CFG); //整数编辑菜单的主处理，包括渲染和按键
void IntEditInitHandler(void);	//整数编辑进入时的处理
	
//给菜单项用的常数
extern bool AlwaysTrue;
extern bool AlwaysFalse;	
	
//自检错误处理
void SelfTestErrorHandler(void);
	
//内部变量
extern bool IsEnableAdvancedMode; //是否开启高级模式
	
//外部调用的显示函数

void ShowTimeCode(u16 y,long Time);  //根据传入的秒数显示时间
void RenderMenuBG(void);	//菜单背景渲染
#define ClearScreen() LCD_Fill(0,0,LCD_W,LCD_H,BLACK)
void SwitchingMenu(const MenuConfigDef *TargetMenuIdx); //切换菜单
void ShowPostInfo(char Present,char *Msg,char *ID,MessageTypeDef Type); //展示自检信息
void MenuRenderProcess(void); //执行菜单渲染
void GUIDelayHandler(void); //GUI计时处理
void PostScreenInit(void); //主菜单初始化	
	
#endif
