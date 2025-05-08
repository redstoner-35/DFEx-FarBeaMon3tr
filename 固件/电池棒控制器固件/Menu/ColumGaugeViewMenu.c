#include "GUI.h"
#include "LogSystem.h"
#include "Key.h"

//内部变量
static char ShowCMenuState=0;
bool IsUpdateCDUI=false;

static void DisplayCapacity(float IN,u16 y)
	{
	if(IN<10)LCD_ShowFloatNum1(87,y,IN,3,WHITE,LGRAY,12); //9.999显示
	else if(IN<100)LCD_ShowFloatNum1(87,y,IN,2,WHITE,LGRAY,12); //99.99显示	
	else if(IN<1000)LCD_ShowFloatNum1(87,y,IN,1,WHITE,LGRAY,12); //999.9显示
	else if(IN<10000)LCD_ShowIntNum(87,y,iroundf(IN),5,WHITE,LGRAY,12); //9999显示
	else	
	  {
		IN/=1000;
	  if(IN<10)LCD_ShowFloatNum1(87,y,IN,2,WHITE,LGRAY,12); //9.99显示
		else if(IN<100)LCD_ShowFloatNum1(87,y,IN,1,WHITE,LGRAY,12); //99.9显示	
		else if(IN<1000)LCD_ShowIntNum(87,y,iroundf(IN),3,WHITE,LGRAY,12); //999显示
		}
	if(IN<10000)LCD_ShowString(138,y,"Wh",WHITE,LGRAY,12,0);
	else LCD_ShowString(129,y,"KWh",WHITE,LGRAY,12,0);
	}
	
static void DisplayAh(float IN,u16 y)
	{
	float buf;
  //使用mAH显示
	if(IN<10000) //小于10Ah使用mAH显示
		{
		LCD_ShowIntNum(87,y,iroundf(IN),4,WHITE,LGRAY,12);
		LCD_ShowString(129,y,"mAh",WHITE,LGRAY,12,0);
		}
	else if(IN<10000*1000)//使用浮点显示
		{
	  buf/=(float)1000;			
		if(buf<100)LCD_ShowFloatNum1(87,y,buf,2,WHITE,LGRAY,12);  //99.99显示
		else if(buf<100)LCD_ShowFloatNum1(87,y,buf,1,WHITE,LGRAY,12);  //999.9显示
		else LCD_ShowIntNum(87,y,iroundf(buf),5,WHITE,LGRAY,12); //9999显示
		LCD_ShowString(138,y,"Ah",WHITE,LGRAY,12,0);
		}
	else //使用KAH显示
		{
		buf/=(float)1000*1000;	
    if(buf<10)LCD_ShowFloatNum1(87,y,buf,3,WHITE,LGRAY,12);  //9.999显示
		else if(buf<100)LCD_ShowFloatNum1(87,y,buf,2,WHITE,LGRAY,12);  //99.99显示
		else if(buf<1000)LCD_ShowFloatNum1(87,y,buf,1,WHITE,LGRAY,12);  //999.9显示
		else LCD_ShowIntNum(87,y,iroundf(buf),5,WHITE,LGRAY,12); //9999显示
		LCD_ShowString(129,y,"KAh",WHITE,LGRAY,12,0);
		}
	}

static void ShowColumbDataUpper(void)
	{
	//充电时长
	LCD_ShowChinese(3,21,"充电时长",WHITE,LGRAY,0);
	ShowTimeCode(21,LogData.ChargeTime);
	//充入能力
	LCD_ShowChinese(3,35,"充入能量",WHITE,LGRAY,0);
	DisplayCapacity(LogData.TotalChargeWh,35);
  //充入容量
	LCD_ShowChinese(3,49,"充入容量",WHITE,LGRAY,0);
	DisplayAh(LogData.TotalChargeAh*1000,49);
	//电池端最大电流
  LCD_ShowChinese(3,64,"最高电池电流",WHITE,LGRAY,0);		
	LCD_ShowFloatNum1(87,64,LogData.MaximumBattCurrent,2,WHITE,LGRAY,12);
	LCD_ShowChar(147,64,'A',WHITE,LGRAY,12,0);			
	}
	
static void ShowColumbDataLower(void)
	{
	u16 Color;
	float Temp;
	//充电时长
	LCD_ShowChinese(3,21,"放电时长",WHITE,LGRAY,0);
	ShowTimeCode(21,LogData.DischargeTime);
	//充入能力
	LCD_ShowChinese(3,35,"放出能量",WHITE,LGRAY,0);
	DisplayCapacity(LogData.TotalDischargeWh,35);
  //充入容量
	LCD_ShowChinese(3,49,"放出容量",WHITE,LGRAY,0);
	DisplayAh(LogData.TotalDischargeAh*1000,49);
	//显示温度
	LCD_ShowChinese(3,64,"系统最高温度",WHITE,LGRAY,0);
	if(LogData.SysMaxTemp==-100)LCD_ShowChinese(103,64,"不可用",RED,LGRAY,0);
	else
		{
		Temp=LogData.SysMaxTemp;
		if(Temp<0)Color=DARKBLUE;	
		else if(Temp<10)Color=BLUE;
		else if(Temp<CfgData.OverHeatLockTemp-20)Color=GREEN;
		else if(Temp<CfgData.OverHeatLockTemp-8)Color=YELLOW;
		else Color=RED;
		//负数温度
		if(Temp<0)
			{
			Temp*=-1; //转正数
			LCD_ShowChar(98,64,'-',Color,LGRAY,12,0);
			if(Temp<10)LCD_ShowFloatNum1(107,64,Temp,2,Color,LGRAY,12); //9.99显示
			else LCD_ShowFloatNum1(107,64,Temp,1,Color,LGRAY,12); //99.9显示
			}
		//正数温度
		else if(Temp<10)LCD_ShowFloatNum1(98,64,Temp,3,Color,LGRAY,12); //9.999显示
		else if(Temp<100)LCD_ShowFloatNum1(98,64,Temp,2,Color,LGRAY,12); //99.99显示
		else LCD_ShowFloatNum1(98,64,Temp,1,Color,LGRAY,12); //999.9显示
		//显示摄氏度符号
		LCD_ShowChinese12x12(143,64,"℃",WHITE,LGRAY,12,0); //显示汉字
		}
	}
//显示未均衡容量
static void ShowbalanceStatic(void)
	{
	LCD_ShowChinese(3,21,"均衡时长",WHITE,LGRAY,0);
	ShowTimeCode(21,LogData.BalanceTime);
	
	LCD_ShowChinese(3,35,"未均衡容量",WHITE,LGRAY,0);
	DisplayAh(LogData.UnbalanceBatteryAh*1000,35);
	}
	
//容量显示的按键处理
void ColHisKeyHandler(void)
	{
	//上下翻页
	if(KeyState.KeyEvent==KeyEvent_Down&&ShowCMenuState>0)ShowCMenuState--;
	if(KeyState.KeyEvent==KeyEvent_Up&&ShowCMenuState<2)ShowCMenuState++;
	//退出
	if(KeyState.KeyEvent==KeyEvent_ESC)
		{
		if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
		else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
		}
	if(KeyState.KeyEvent!=KeyEvent_None)
		{
		IsUpdateCDUI=true;
		KeyState.KeyEvent=KeyEvent_None;
		}
	}

//显示GUI
void ShowColHisGUI(void)
	{
	if(!IsUpdateCDUI)return;
	RenderMenuBG();
	switch(ShowCMenuState)
		{
		case 0:ShowColumbDataUpper();break;
		case 1:ShowColumbDataLower();break;
		case 2:ShowbalanceStatic();break;
		}
	IsUpdateCDUI=false;
	}
	
void ResetColHisMenuToUpper(void)
	{
	//每次进入时重置数据
	IsUpdateCDUI=true;
	ShowCMenuState=0;
	}	
	
const MenuConfigDef ColHisMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&ShowColHisGUI, //渲染函数
	&ColHisKeyHandler, //按键处理
	//不是设置菜单不需要用别的事情
	"查看库仑计历史数据",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数没有事情要做
	&ResetColHisMenuToUpper,
	NULL
	};	
