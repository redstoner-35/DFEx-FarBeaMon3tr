#include "GUI.h"
#include "Key.h"
#include "IP2366_REG.h"
#include "PCA9536.h"
#include "LogSystem.h"
#include "24Cxx.h"
#include "Config.h"
#include <string.h>

static bool ChipStateUpdated=false;
static char ChipInfoSelect=0;
static int UsedSpace=0;

//显示高精度功率计状态
static void ShowHPGaugeState(void)
	{
	extern bool IsEnableHPGauge;
	bool Result,PinState;
	int PPS1ICC,PPS2ICC;
	LCD_ShowChinese(4,21,"高精度功率计",WHITE,LGRAY,0);
	if(!CfgData.EnableHPGauge)LCD_ShowChinese(131,21,"禁用",WHITE,LGRAY,0);
	else if(!IsEnableHPGauge)LCD_ShowChinese(131,21,"故障",RED,LGRAY,0);
	else LCD_ShowChinese(131,21,"正常",GREEN,LGRAY,0);	
	//显示均衡控制器状态
	Result=PCA9536_SetIOState(PCA9536_IOPIN_3,false);
	Result&=PCA9536_GetOutputState(PCA9536_IOPIN_0,&PinState);  //获取输入状态
	LCD_ShowChinese(4,35,"主动均衡模块",WHITE,LGRAY,0);
	if(!Result)LCD_ShowChinese(131,35,"故障",RED,LGRAY,0);
	else if(!PinState)LCD_ShowChinese(131,35,"正常",WHITE,LGRAY,0);		
	else LCD_ShowChinese(131,35,"启用",GREEN,LGRAY,0);
	//显示PPS1和PPS2电流
	Result=IP2366_GetPPSCurrent(&PPS1ICC,&PPS2ICC);
	LCD_ShowHybridString(4,49,"PPS1广播电流",WHITE,LGRAY,0);
	LCD_ShowHybridString(4,64,"PPS2广播电流",WHITE,LGRAY,0);	
	if(!Result)
		{
		LCD_ShowChinese(131,49,"未知",WHITE,LGRAY,0);
		LCD_ShowChinese(131,64,"未知",WHITE,LGRAY,0);
		}
	else
		{
		LCD_ShowIntNum(95,49,PPS1ICC,4,WHITE,LGRAY,12);
		LCD_ShowString(137,49,"mA",WHITE,LGRAY,12,0);
		LCD_ShowIntNum(95,64,PPS2ICC,4,WHITE,LGRAY,12);
		LCD_ShowString(137,64,"mA",WHITE,LGRAY,12,0);		
		}		
	}

//显示配置文件状态
static bool ShowConfigState(void)
	{
	extern bool UsingBackupConfig;
		
	if(!UsedSpace)
		{
		LCD_ShowChinese(10,22,"正在获取外存储器状态",WHITE,LGRAY,0);
		LCD_ShowChinese(46,40,"请稍等……",WHITE,LGRAY,0);
		UsedSpace=CalcCurrentAvailableLogCount();
		return false;
		}
	LCD_ShowChinese(4,21,"当前欲编程日志入口",WHITE,LGRAY,0);
	LCD_ShowChinese(4,35,"当前配置文件",WHITE,LGRAY,0);
	LCD_ShowChinese(4,49,"主用配置文件状态",WHITE,LGRAY,0);
	LCD_ShowChinese(4,64,"备用配置文件状态",WHITE,LGRAY,0);
	
  if(UsedSpace==-1)LCD_ShowChinese(131,21,"未知",WHITE,LGRAY,0);			
	else LCD_ShowIntNum(124,21,RunLogEntry.ProgrammedEntry,4,WHITE,LGRAY,12);	
		
	if(UsedSpace==-1)LCD_ShowChinese(131,35,"未知",RED,LGRAY,0);		
	else if(UsingBackupConfig)LCD_ShowChinese(131,35,"备用",YELLOW,LGRAY,0);	
  else LCD_ShowChinese(131,35,"主用",GREEN,LGRAY,0);		
	
	if(UsedSpace==-1)LCD_ShowChinese(131,49,"未知",RED,LGRAY,0);	
	else if(!CheckIfConfigOK(false))LCD_ShowChinese(131,49,"损坏",RED,LGRAY,0);	
	else LCD_ShowChinese(131,49,"正常",GREEN,LGRAY,0);	

	if(UsedSpace==-1)LCD_ShowChinese(131,64,"未知",RED,LGRAY,0);	
	else if(!CheckIfConfigOK(true))LCD_ShowChinese(131,64,"损坏",RED,LGRAY,0);	
	else LCD_ShowChinese(131,64,"正常",GREEN,LGRAY,0);	
		
	//返回状态
	return true;
	}	

//显示存储器状态
static bool ShowStorageState(void)
	{
	int Capacity=MaxByteRange+1;
	char Result;	
	char UIDBUF[2];
	float fbuf;
	
  if(!UsedSpace)
		{
		LCD_ShowChinese(10,22,"正在获取外存储器状态",WHITE,LGRAY,0);
		LCD_ShowChinese(46,40,"请稍等……",WHITE,LGRAY,0);
		UsedSpace=CalcCurrentAvailableLogCount();
		return false;
		}
  
	if(UsedSpace==-1)Result=1; //进行读取时存储器异常，显示failed
	else Result=M24C512_ReadUID(UIDBUF,2);
	LCD_ShowChinese(4,21,"系统外存储器状态",WHITE,LGRAY,0);	
	LCD_ShowChinese(4,35,"存储器总容量",WHITE,LGRAY,0);
	LCD_ShowChinese(4,49,"存储器已用容量",WHITE,LGRAY,0);
	LCD_ShowChinese(4,64,"总日志条数",WHITE,LGRAY,0);
	if(Result)LCD_ShowChinese(131,21,"异常",RED,LGRAY,0);		
	else LCD_ShowChinese(131,21,"正常",GREEN,LGRAY,0);		
		
	//显示总容量
	if(Result)LCD_ShowChinese(131,35,"未知",WHITE,LGRAY,0);	
	else if(Capacity<10000)
		{
		LCD_ShowIntNum(107,35,Capacity,4,WHITE,LGRAY,12);
	  LCD_ShowString(142,35," B",WHITE,LGRAY,12,0);
		}
	else
		{		
		fbuf=(float)Capacity/(float)1024;
		LCD_ShowFloatNum1(100,35,fbuf,fbuf<99?2:1,WHITE,LGRAY,12);
		LCD_ShowString(142,35,"KB",WHITE,LGRAY,12,0);
		}
	//显示已用容量
	Capacity=UsedSpace;
	if(!CTestData.ROMImage.Data.Data.IsDataValid)Capacity-=sizeof(ChargeTestStorDef); //测容数据无效时减去测容数据的结果
	if(Capacity>(MaxByteRange+1))Capacity=MaxByteRange+1; //限幅，禁止容量数据大于EEPROM已有存储数据
	if(Result)LCD_ShowChinese(131,49,"未知",WHITE,LGRAY,0);	
	else if(Capacity<10000)
		{
		LCD_ShowIntNum(107,49,Capacity,4,WHITE,LGRAY,12);
	  LCD_ShowString(142,49," B",WHITE,LGRAY,12,0);
		}
	else
		{		
		fbuf=(float)Capacity/(float)1024;
		LCD_ShowFloatNum1(100,49,fbuf,fbuf<99?2:1,WHITE,LGRAY,12);
		LCD_ShowString(142,49,"KB",WHITE,LGRAY,12,0);
		}			
	if(Result)LCD_ShowChinese(131,64,"未知",WHITE,LGRAY,0);		
	else if(RunLogEntry.Data.DataSec.TotalLogCount<10000)
		{
		LCD_ShowIntNum(107,64,RunLogEntry.Data.DataSec.TotalLogCount,4,WHITE,LGRAY,12);
		LCD_ShowChinese(142,64,"条",WHITE,LGRAY,12);
		}
	else
	 {
	 LCD_ShowFloatNum1(89,64,(float)RunLogEntry.Data.DataSec.TotalLogCount/(float)1000,2,WHITE,LGRAY,12);
	 LCD_ShowChinese(130,64,"千条",WHITE,LGRAY,12);
	 }
	//返回状态
	return true;
	}	
	
//显示芯片充电配置
static void ShowChipChargeState(void)
	{
	bool Result;
	float Vstop;
  int Istop;		
	ChipStatDef State;
	//显示充电配置	
	LCD_ShowChinese(4,21,"停充截止电流",WHITE,LGRAY,0);
	LCD_ShowChinese(4,35,"恒压充电电压",WHITE,LGRAY,0);
	Result=IP2366_getCurrentChargeParam(&Istop,&Vstop);
	if(!Result)
		{
		LCD_ShowChinese(131,21,"未知",WHITE,LGRAY,0);
		LCD_ShowChinese(131,35,"未知",WHITE,LGRAY,0);
		}
	else
		{
		LCD_ShowIntNum(95,21,Istop,3,WHITE,LGRAY,12);
		LCD_ShowString(137,21,"mA",WHITE,LGRAY,12,0);
		LCD_ShowFloatNum1(95,35,Vstop,2,WHITE,LGRAY,12);
		LCD_ShowString(137,35," V",WHITE,LGRAY,12,0);
		}
	//显示Type-C状态
	Result=IP2366_ReadChipState(&State);
	LCD_ShowHybridString(4,49,"Type-C端口状态",WHITE,LGRAY,0);
	if(!Result)LCD_ShowChinese(131,49,"未知",WHITE,LGRAY,0);
	else switch(State.VBusState)
		{
		case VBUS_NoPower:LCD_ShowChinese(127,49,"下电",WHITE,LGRAY,0);break;
		case VBUS_Normal:LCD_ShowChinese(127,49,"正常",GREEN,LGRAY,0);break;
		case VBUS_OverVolt:LCD_ShowChinese(127,49,"过压",RED,LGRAY,0);break;
		}	
	//显示再充电压
	Result=IP2366_GetVRecharge(&Vstop);
	LCD_ShowHybridString(4,64,"再充电电压",WHITE,LGRAY,0);
	if(!Result)LCD_ShowChinese(131,64,"未知",WHITE,LGRAY,0);	
	else if(Vstop==-1)LCD_ShowChinese(131,64,"禁用",WHITE,LGRAY,0);	
	else
		{
		LCD_ShowFloatNum1(95,64,Vstop,2,WHITE,LGRAY,12);
		LCD_ShowString(137,64," V",WHITE,LGRAY,12,0);
		}
	}

//显示芯片基本信息
static void ShowChipBasicInfo(void)
	{
	extern bool IsEnable17AMode;
	bool Result;
	char ChipIDBuf[6];
	int ICCMax;
	ChipStatDef State;
	RecvPDODef PDOState;
	//获取芯片字符串
	memset(ChipIDBuf,0,sizeof(ChipIDBuf));
	Result=IP2366_GetFirmwareTimeStamp(ChipIDBuf);
	LCD_ShowChinese(4,21,"固件时间戳",WHITE,LGRAY,0);
	if(!Result)LCD_ShowString(130,21,"N/A",RED,LGRAY,12,0);
	else if(!IsEnableAdvancedMode)LCD_ShowString(118,21,"*****",WHITE,LGRAY,12,0);
  else LCD_ShowString(118,21,ChipIDBuf,IsEnable17AMode?CYAN:WHITE,LGRAY,12,0);
	//获取VBUS状态
	LCD_ShowChinese(4,35,"峰值电流",WHITE,LGRAY,0);
	Result=IP2366_GetCurrentPeakCurrent(&ICCMax);
	if(!Result)LCD_ShowChinese(131,35,"未知",WHITE,LGRAY,0);
	else
		{
		if(!IsEnableAdvancedMode)LCD_ShowString(95,35,"?????",WHITE,LGRAY,12,0);
		else LCD_ShowIntNum(95,35,ICCMax,5,WHITE,LGRAY,12);
		LCD_ShowString(137,35,"mA",WHITE,LGRAY,12,0);
		}
	//获取PDO状态
	Result=IP2366_GetRecvPDO(&PDOState);
	LCD_ShowChinese(4,49,"当前",WHITE,LGRAY,0);
	LCD_ShowString(30,49,"PDO",WHITE,LGRAY,12,0);
	LCD_ShowChinese(57,49,"广播输入",WHITE,LGRAY,0);
  if(!Result)LCD_ShowChinese(131,49,"未知",WHITE,LGRAY,0);
	else switch(PDOState)
		{
		case RecvPDO_None:LCD_ShowString(130,49,"N/A",WHITE,LGRAY,12,0);break;
		case RecvPDO_5V:LCD_ShowString(130,49," 5V",CYAN,LGRAY,12,0);break;
		case RecvPDO_9V:LCD_ShowString(130,49," 9V",BLUE,LGRAY,12,0);break;
		case RecvPDO_12V:LCD_ShowString(130,49,"12V",GREEN,LGRAY,12,0);break;
		case RecvPDO_15V:LCD_ShowString(130,49,"15V",CYAN,LGRAY,12,0);break;
		case RecvPDO_20V:LCD_ShowString(130,49,"20V",CYAN,LGRAY,12,0);break;
		}
	//VSys状态
	Result=IP2366_ReadChipState(&State);
	LCD_ShowChinese(4,64,"系统状态",WHITE,LGRAY,0);
	if(!Result)LCD_ShowChinese(131,64,"未知",WHITE,LGRAY,0);
	else switch(State.VSysState)
		{
		case VSys_State_Normal:LCD_ShowChinese(131,64,"正常",GREEN,LGRAY,0);break;
		case VSys_State_OCP:LCD_ShowChinese(131,64,"过流",RED,LGRAY,0);break;
		case VSys_State_Short:LCD_ShowChinese(131,64,"短路",RED,LGRAY,0);break;
		}
	}

//显示芯片信息
void ShowChipInfo(void)
	{
	//已经渲染过了
	if(ChipStateUpdated)return;
  RenderMenuBG();
	//按照index渲染对应的结果
  switch(ChipInfoSelect)		
		{
		case 0:ShowChipBasicInfo();break;
		case 1:ShowChipChargeState();break;
		case 2:ShowHPGaugeState();break;
		case 3:ChipStateUpdated=ShowStorageState();break;
		case 4:ChipStateUpdated=ShowConfigState();break;
		default:
			ChipInfoSelect=0;
		  ChipStateUpdated=false;
		  return;  //卡出来的非法状态，退出
		}
	//渲染完毕，指示状态
	if(ChipInfoSelect<3)ChipStateUpdated=true;
	}
	
void ShowChipKeyHandler(void)
	{
	//上下键翻页
	if(KeyState.KeyEvent==KeyEvent_Up&&ChipInfoSelect<4)
		{
		ChipInfoSelect++;
		ChipStateUpdated=false;
		}
	if(KeyState.KeyEvent==KeyEvent_Down&&ChipInfoSelect>0)	
		{
		ChipInfoSelect--;
		ChipStateUpdated=false;
		}		
	//按下ESC退出
	if(KeyState.KeyEvent==KeyEvent_ESC)
		{
		if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
		else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
		}
	if(KeyState.KeyEvent==KeyEvent_Enter)ChipStateUpdated=false; //刷新状态
	KeyState.KeyEvent=KeyEvent_None;
	}

//进入时重置菜单flag的函数
void ResetChipMenuState(void)
	{
	UsedSpace=0;
	ChipStateUpdated=false;
	ChipInfoSelect=0;
	}

const MenuConfigDef ChipStatMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&ShowChipInfo, //渲染函数
	&ShowChipKeyHandler, //按键处理
	//不是设置菜单不需要用别的事情
	"查看芯片状态",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数没有事情要做
	&ResetChipMenuState,
	NULL
	};	
