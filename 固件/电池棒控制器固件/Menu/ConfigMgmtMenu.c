#include "GUI.h"
#include "Config.h"
#include "Key.h"

//外部变量
extern bool IsResetRendered;
extern bool UsingBackupConfig;
bool IsConfigSaved=false;

//函数声明
void BackToResetMenu(void);

//复位配置文件的显示
void ResetCfgProcessDisp(void)
	{
	IsResetRendered=false;
	}

//更新备用配置文件的入口函数
void DisplayCfgSyncProcessOK(void)
	{
	bool LastBackupState=UsingBackupConfig;
	if(IsResetRendered)return;
	RenderMenuBG(); //显示背景
	//开始同步配置文件
	UsingBackupConfig=true;
	if(WriteConfiguration(&CfgUnion,false))
		{
		LCD_ShowChinese(21,23,"已将当前系统设置更",WHITE,LGRAY,0);
		LCD_ShowHybridString(21,37," 新至备用配置文件",WHITE,LGRAY,0);
		}
	else
		{		
		LCD_ShowChinese(21,23,"更新备用配置文件时",RED,LGRAY,0);
		LCD_ShowHybridString(21,37," 出现异常,请重试",RED,LGRAY,0);
		}
	//写入完毕，回到之前的状态
	UsingBackupConfig=LastBackupState;
	
	LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
	LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
	IsResetRendered=true;
	}

const MenuConfigDef UpdateBackupFileMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
  &DisplayCfgSyncProcessOK,
	&BackToResetMenu,	
	//不是设置菜单不需要用别的事情
	"更新备用配置\0",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&ResetCfgProcessDisp,
	NULL
	};	
	
//读取备用配置文件并恢复
void ReadBackupConfigBack(void)
	{
	CfgUnionDef buf;
  if(IsResetRendered)return;
	RenderMenuBG(); //显示背景
	if(!ReadConfiguration(&buf,true))
		{
		LCD_ShowChinese(21,23,"读取备用配置文件时",RED,LGRAY,0);
		LCD_ShowHybridString(21,37," 出现异常,请重试",RED,LGRAY,0);
		}
	else if(buf.ROMImage.CRCResult!=CalcROMCRC32(&buf))		
		{
		LCD_ShowChinese(21,23,"备用配置文件已损坏",RED,LGRAY,0);
		LCD_ShowHybridString(21,37," 恢复过程无法继续",RED,LGRAY,0);
		}
	else
		{
		ReadConfiguration(&CfgUnion,true);
		LCD_ShowChinese(21,23,"已从备用配置文件内",WHITE,LGRAY,0);
		LCD_ShowHybridString(21,37," 恢复系统设置数据",WHITE,LGRAY,0);
		}		
		
	//读取完毕	
	LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
	LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
	IsResetRendered=true;	
	}
	
const MenuConfigDef RestoreBackupFileMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
  &ReadBackupConfigBack,
	&BackToResetMenu,	
	//不是设置菜单不需要用别的事情
	"恢复备用配置\0",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&ResetCfgProcessDisp,
	NULL
	};	
	
//放弃当前更改的处理	
void DiscardCurrentPendingChanges(void)
  {
	CfgUnionDef buf;
  if(IsResetRendered)return;
	RenderMenuBG(); //显示背景
	if(!ReadConfiguration(&buf,UsingBackupConfig))
		{
		LCD_ShowChinese(21,23,"读取当前配置文件时",RED,LGRAY,0);
		LCD_ShowHybridString(21,37," 出现异常,请重试",RED,LGRAY,0);
		}
	else if(buf.ROMImage.CRCResult!=CalcROMCRC32(&buf))		
		{
		LCD_ShowChinese(21,23,"当前配置文件已损坏",RED,LGRAY,0);
		LCD_ShowHybridString(21,37," 无法撤销设置更改",RED,LGRAY,0);
		}
	else
		{
		ReadConfiguration(&CfgUnion,UsingBackupConfig);
		LCD_ShowChinese(21,37,"已撤销当前设置更改",WHITE,LGRAY,0);
		}		
  
	//读取完毕	
	LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
	LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
	IsResetRendered=true;		
	}
	
const MenuConfigDef DiscardCurrentPendingChangesMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
  &DiscardCurrentPendingChanges,
	&BackToResetMenu,	
	//不是设置菜单不需要用别的事情
	"撤销当前更改\0",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&ResetCfgProcessDisp,
	NULL
	};	
	
//存储当前更改
void SaveCurrentChanges(void)
	{
	if(IsResetRendered)return;
	RenderMenuBG(); //显示背景
	if(!CheckIfConfigIsSame())IsConfigSaved=true; //配置文件发生更改，标记
	if(WriteConfiguration(&CfgUnion,false))
		LCD_ShowChinese(21,37,"已保存当前系统设置",WHITE,LGRAY,0);
	else 
		LCD_ShowChinese(21,37,"系统设置保存失败",RED,LGRAY,0);
	//读取完毕	
	LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
	LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
	IsResetRendered=true;		
	}	
	
const MenuConfigDef SaveSystemSettingMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
  &SaveCurrentChanges,
	&BackToResetMenu,	
	//不是设置菜单不需要用别的事情
	"保存系统设置\0",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&ResetCfgProcessDisp,
	NULL
	};
	
//进入安全设置处理
void EnterRestoreBCfgMenu(void)
	{
	SwitchingMenu(&RestoreBackupFileMenu);
	}
	
void EnterUpdateBCFGMenu(void)
	{
	SwitchingMenu(&UpdateBackupFileMenu);
	}

PasswordInputDef EnterRestoreBCFGVerify=
	{
	"\x8F\xE8\xB3\x93",
	&EnterRestoreBCfgMenu,
	&BackToResetMenu,
	};
	
PasswordInputDef EnterUpdateBCFGVerify=
	{
	"\x8F\xE8\xB3\x93",
	&EnterUpdateBCFGMenu,
	&BackToResetMenu,
	};
	
void VerifyPassWhenUpdateBCFGEnter(void)
	{
	PassWordMenuRender(&EnterUpdateBCFGVerify);
	}
	
void VerifyPassWhenRestoreBCfgEnter(void)
	{
	PassWordMenuRender(&EnterRestoreBCFGVerify);
	}

const MenuConfigDef PSWDVerifyBeforeUpdateBCFGMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&VerifyPassWhenUpdateBCFGEnter, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"管理员安全验证",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数没有事情要做
	&PasswordEnterInit,
	NULL
	};	

const MenuConfigDef PSWDVerifyBeforeRestoreBCFGMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&VerifyPassWhenRestoreBCfgEnter, //渲染函数
	NULL, //按键处理
	//不是设置菜单不需要用别的事情
	"管理员安全验证",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数没有事情要做
	&PasswordEnterInit,
	NULL
	};	
	
//自动存盘配置
const EnumEditEntryDef AutoSaveEnumCfg[3]=
	{
		{
		"退出菜单时自动存盘",
	  true,
		AutoSave_Enabled,
		false,
		},
		{
		"配置菜单内手动存盘",
	  true,
		AutoSave_Disabled,
		false,
		},
		{ //占位符
		"",
	  false,
		100,
		true
		}
	};	

int ReadSaveCfgValue(void)
	{
	//返回充电功率的enum值	
	return (int)CfgData.AutoSaveCfg;
	}	
	
void FedSaveCfgValue(int Input)
	{
	CfgData.AutoSaveCfg=(AutoSaveCfgDef)Input;
	SwitchingMenu(&RSTMainMenu); //处于退出状态,按下ESC后回到主菜单	
	}
	
const MenuConfigDef AutoSaveCfgMenu=
	{
	MenuType_EnumSetup,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	AutoSaveEnumCfg,
  &ReadSaveCfgValue,
  &FedSaveCfgValue,		
	//特殊渲染的处理
	NULL, //渲染函数
	NULL, //按键处理
	//主设置菜单
	"配置自动存盘设置",
	NULL,
	NULL,
	NULL, 
	//进入和退出构造函数没有事情要做
	NULL,
	NULL
	};	
	
	
