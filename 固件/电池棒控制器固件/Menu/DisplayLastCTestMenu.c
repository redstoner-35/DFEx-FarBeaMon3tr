#include "GUI.h"
#include "CapTest.h"
#include <math.h>
#include "Key.h"
#include "Config.h"

//内部变量
static bool ShowMenuState;
static bool IsUpdateCUI=false;

//显示GUI下半部分
void ShowLowerPart(void)
	{
	u16 Color;
	float Temp;
	//显示温度
	LCD_ShowChinese(3,21,"测容时最高温度",WHITE,LGRAY,0);
	if(LastCData.MaxChargeTemp==-100)LCD_ShowChinese(103,21,"不可用",RED,LGRAY,0);
	else
		{
		Temp=LastCData.MaxChargeTemp;
		if(Temp<0)Color=DARKBLUE;	
		else if(Temp<10)Color=BLUE;
		else if(Temp<CfgData.OverHeatLockTemp-20)Color=GREEN;
		else if(Temp<CfgData.OverHeatLockTemp-8)Color=YELLOW;
		else Color=RED;
		//负数温度
		if(Temp<0)
			{
			Temp*=-1; //转正数
			LCD_ShowChar(98,21,'-',Color,LGRAY,12,0);
			if(Temp<10)LCD_ShowFloatNum1(107,21,Temp,2,Color,LGRAY,12); //9.99显示
			else LCD_ShowFloatNum1(107,21,Temp,1,Color,LGRAY,12); //99.9显示
			}
		//正数温度
		else if(Temp<10)LCD_ShowFloatNum1(98,21,Temp,3,Color,LGRAY,12); //9.999显示
		else if(Temp<100)LCD_ShowFloatNum1(98,21,Temp,2,Color,LGRAY,12); //99.99显示
		else LCD_ShowFloatNum1(98,21,Temp,1,Color,LGRAY,12); //999.9显示
		//显示摄氏度符号
		LCD_ShowChinese12x12(143,21,"℃",WHITE,LGRAY,12,0); //显示汉字
		}
	//显示最大倍率
	Temp=LastCData.MaxChargeRatio;
	if(Temp<1.0)Color=GREEN;
  else if(Temp<2.0)Color=YELLOW;
  else Color=RED;  //根据倍率显示颜色
	LCD_ShowChinese(3,35,"最大充电倍率",WHITE,LGRAY,0);
	if(LastCData.MaxChargeRatio<10)LCD_ShowFloatNum1(98,35,LastCData.MaxChargeRatio,3,Color,LGRAY,12); //9.999显示
	else if(LastCData.MaxChargeRatio<100)LCD_ShowFloatNum1(98,35,LastCData.MaxChargeRatio,2,Color,LGRAY,12); //99.99显示
	else LCD_ShowFloatNum1(98,35,LastCData.MaxChargeRatio,1,Color,LGRAY,12); //99.99显示
	LCD_ShowChar(147,35,'C',WHITE,LGRAY,12,0);
	//显示测容开始电压
	LCD_ShowChinese(3,49,"测容开始电压",WHITE,LGRAY,0);
	LCD_ShowFloatNum1(98,49,LastCData.StartVbatt,2,WHITE,LGRAY,12);
	LCD_ShowChar(147,49,'V',WHITE,LGRAY,12,0);
	//电池最高电压
	Temp=LastCData.MaxVbatt/4;
	if(Temp<4.21)Color=GREEN;
  else if(Temp<4.25)Color=YELLOW;
  else Color=RED;  //根据截止电压状态显示状态
	LCD_ShowChinese(3,64,"电池最高电压",WHITE,LGRAY,0);
	LCD_ShowFloatNum1(98,64,LastCData.MaxVbatt,2,Color,LGRAY,12);
	LCD_ShowChar(147,64,'V',WHITE,LGRAY,12,0);	
	}

//显示菜单的上半部分
void ShowUpperPart(void)
	{
	float buf;
	//显示时间
	LCD_ShowChinese(3,21,"充电时长",WHITE,LGRAY,0);
	ShowTimeCode(21,LastCData.ChargeTime);
	//显示充入电量
  LCD_ShowChinese(3,35,"充入能量",WHITE,LGRAY,0);		
	buf=LastCData.TotalWh;
	if(buf<10)LCD_ShowFloatNum1(87,35,buf,3,WHITE,LGRAY,12); //9.999显示
	else if(buf<100)LCD_ShowFloatNum1(87,35,buf,2,WHITE,LGRAY,12); //99.99显示	
	else if(buf<1000)LCD_ShowFloatNum1(87,35,buf,1,WHITE,LGRAY,12); //999.9显示
	else LCD_ShowIntNum(87,35,iroundf(buf),5,WHITE,LGRAY,12); //9999显示
	LCD_ShowString(138,35,"Wh",WHITE,LGRAY,12,0);
	//显示冲入Ah数
	LCD_ShowChinese(3,49,"充入容量",WHITE,LGRAY,0);		
	buf=LastCData.TotalmAH;
	if(buf<10000) //小于10Ah使用mAH显示
		{
		LCD_ShowIntNum(87,49,iroundf(buf),4,WHITE,LGRAY,12);
		LCD_ShowString(129,49,"mAh",WHITE,LGRAY,12,0);
		}
	else //使用浮点显示
		{
	  buf/=(float)1000;			
		if(buf<100)LCD_ShowFloatNum1(87,49,buf,2,WHITE,LGRAY,12);  //99.99显示
		else if(buf<100)LCD_ShowFloatNum1(87,49,buf,1,WHITE,LGRAY,12);  //999.9显示
		else LCD_ShowIntNum(87,49,iroundf(buf),5,WHITE,LGRAY,12); //9999显示
		LCD_ShowString(138,49,"Ah",WHITE,LGRAY,12,0);
		}
	//显示最高充电电流
	LCD_ShowChinese(3,64,"最高充电电流",WHITE,LGRAY,0);		
	LCD_ShowFloatNum1(87,64,LastCData.MaxChargeCurrent,2,WHITE,LGRAY,12);
	LCD_ShowChar(147,64,'A',WHITE,LGRAY,12,0);	
	}
//容量显示的按键处理
void CapHisKeyHandler(void)
	{
	//上下翻页
	if(KeyState.KeyEvent==KeyEvent_Down)ShowMenuState=true;
	if(KeyState.KeyEvent==KeyEvent_Up)ShowMenuState=false;
	//退出
	if(KeyState.KeyEvent==KeyEvent_ESC)
		{
		if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
		else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
		}
	if(KeyState.KeyEvent!=KeyEvent_None)
		{
		IsUpdateCUI=true;
		KeyState.KeyEvent=KeyEvent_None;
		}
  else IsUpdateCUI=false;
	}

//显示GUI
void ShowCapHisGUI(void)
	{
	if(!IsUpdateCUI)return;
	RenderMenuBG();
	if(!ShowMenuState)ShowUpperPart();
	else ShowLowerPart();
	IsUpdateCUI=false;
	}
	
void ResetHisMenuToUpper(void)
	{
	//每次进入时重置数据
	IsUpdateCUI=true;
	ShowMenuState=false;
	}	
	
const MenuConfigDef CapTestHisMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&ShowCapHisGUI, //渲染函数
	&CapHisKeyHandler, //按键处理
	//不是设置菜单不需要用别的事情
	"查看历史测容数据",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数没有事情要做
	&ResetHisMenuToUpper,
	NULL
	};
