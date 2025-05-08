#include "GUI.h"
#include "CapTest.h"
#include "Config.h"
#include "ADC.h"
#include "IP2366_REG.h"
#include <math.h>
#include <string.h>
#include "Key.h"

//测容系统状态机的enum
typedef enum
	{
	CapTest_Initial, //初状态
	CapTest_WaitTypeCInsert, //等待Type-C插入
	CapTest_Running,
	CapTest_Finish, //运行中和正常结束
	CapTest_ConfirmFull, //确认是否充满
	CapTest_OverCharge, //检测到过充
	CapTest_EndERROR, //测容未成功完成
	//强制终止测容	
	CapTest_ConfirmForceStopTest, //让用户确认是否强制终止
	//测容出错
	CapTest_ErrorAlreadyCharging,
	CapTest_ErrorBattToHigh,
	CapTest_ErrorDischarging,
	CapTest_ErrorChipHang,
	}CapTestFSMDef;
	
//外部变量
extern IP2366VBUSStateDef VBUS;
extern BatteryStateDef BATT;
extern bool Is2368Telem;	
extern bool IsTelemOK;
extern const unsigned char TellUserToInsertTypeC[3158];
	
//内部变量
static CapTestFSMDef CFSMState;
static float VBattSumbuf;
static float IBattsumbuf;
static char AverageCounter;	
static short ConfirmTimeCounter=0;
static bool IsUpDateGUI;
static char WaitBackToContinue=0;
	
//重置测容系统
void ResetCapTestSystem(void)
	{
	IsUpDateGUI=false;
	CFSMState=CapTest_Initial;
	VBattSumbuf=0;
	IBattsumbuf=0;
	WaitBackToContinue=0;
	AverageCounter=8;
	//重置当前的测容结果
	CurrentTestResult.Data.ChargeTime=0;
	CurrentTestResult.Data.IsDataValid=false;
	CurrentTestResult.Data.MaxChargeCurrent=0;
	CurrentTestResult.Data.MaxChargeRatio=0;
	CurrentTestResult.Data.MaxChargeTemp=-100;
	CurrentTestResult.Data.TotalmAH=0;
	CurrentTestResult.Data.MaxVbatt=0;
	CurrentTestResult.Data.StartVbatt=0;
	CurrentTestResult.Data.TotalWh=0;
	}
	
//测容系统累加处理
void CTestAverageACC(void)
	{
	extern bool OCState;
	//判断测容完毕计时器累减	
	if(ConfirmTimeCounter>0)ConfirmTimeCounter--;	
	//测容系统没有激活，禁止遥测
	if(CFSMState!=CapTest_WaitTypeCInsert&&CFSMState!=CapTest_Running&&CFSMState!=CapTest_ConfirmForceStopTest)return;
	Is2368Telem=true;
	if(WaitBackToContinue>0)WaitBackToContinue--;
	//监测当前2366是否在充电
	if(!IsTelemOK)return; //数据获取失败
	if(BATT!=Batt_PreChage&&BATT!=Batt_CCCharge&&BATT!=Batt_CVCharge)return;
	if(OCState)return; //系统过充，暂停统计
  //开始统计
	if(AverageCounter>0)
		{
		VBattSumbuf+=ADCO.Vbatt;
		IBattsumbuf+=ADCO.Ibatt;
		AverageCounter--;
		}
	else
		{
		//计算电压和电流平均结果
		VBattSumbuf/=(float)8;
		IBattsumbuf/=(float)8;
		//进行电压采样
		if(CurrentTestResult.Data.StartVbatt==0&&IBattsumbuf>0.15)CurrentTestResult.Data.StartVbatt=VBattSumbuf; //成功开始充电，抓取一次结果
		//进行时间累加
		CurrentTestResult.Data.ChargeTime++;
		//采集最高电压和温度
		if(CurrentTestResult.Data.MaxVbatt<VBattSumbuf)CurrentTestResult.Data.MaxVbatt=VBattSumbuf;
		if(CurrentTestResult.Data.MaxChargeCurrent<IBattsumbuf)CurrentTestResult.Data.MaxChargeCurrent=IBattsumbuf;
		if(!ADCO.IsNTCOK)CurrentTestResult.Data.MaxChargeTemp=-100;
		else if(CurrentTestResult.Data.MaxChargeTemp<ADCO.Systemp)CurrentTestResult.Data.MaxChargeTemp=ADCO.Systemp;
		//计算mAH和Wh
		CurrentTestResult.Data.TotalmAH+=(fabsf(IBattsumbuf)*1000)/(float)3600; //当前电流*1000 /3600秒得到mAH
		CurrentTestResult.Data.TotalWh+=fabsf(VBattSumbuf*IBattsumbuf)/(float)3600; //当前电压*电流/3600秒得到Wh
		//清除平均缓存准备再次采集
		VBattSumbuf=0;
		IBattsumbuf=0;
		AverageCounter=8;
		//采集完毕，更新GUI
		IsUpDateGUI=false;
		}
	}

//按键处理
void CTestKeyHandler(void)
	{
	switch(CFSMState)
		{
		case CapTest_Initial:break; //没有任何操作
		case CapTest_WaitTypeCInsert:
			if(KeyState.KeyEvent==KeyEvent_ESC)CFSMState=CapTest_EndERROR; //在等待Type-C阶段强制退出
		  break;
		//运行中进行确认
	  case CapTest_ConfirmFull:
		case CapTest_Running: 
		case CapTest_OverCharge:
			if(KeyState.KeyEvent!=KeyEvent_ESC)break; //按下退出
		  WaitBackToContinue=80; //延迟80秒后无操作则继续
			CFSMState=CapTest_ConfirmForceStopTest; 
		  break;
		case CapTest_EndERROR:
		case CapTest_ErrorBattToHigh:
		case CapTest_ErrorAlreadyCharging:
		case CapTest_ErrorChipHang:
		case CapTest_ErrorDischarging:
		case CapTest_Finish:
			if(KeyState.KeyEvent!=KeyEvent_ESC)break;
			if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
			else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
		  break;	  
		//确认退出
		case CapTest_ConfirmForceStopTest:
			if(KeyState.KeyEvent==KeyEvent_BothEnt)CFSMState=CapTest_EndERROR; //强制退出测试
		  else if(!WaitBackToContinue||KeyState.KeyEvent!=KeyEvent_None)CFSMState=CapTest_Running; //继续测试
		}
	//清除按键事件
	KeyState.KeyEvent=KeyEvent_None;
	}	

//GUI处理
void CTestGUIHandler(void)
	{
	unsigned long time;
	float Power;
	u16 Color;
	int Temp;
	extern bool IsDispChargingINFO;
	//不执行渲染
	if(IsUpDateGUI&&KeyState.KeyEvent==KeyEvent_None)return;
	RenderMenuBG(); //显示背景
	switch(CFSMState)
		{
		//初始化
		case CapTest_Initial:
				LCD_ShowChinese(27,33,"测容系统正在启动",CYAN,LGRAY,0);
		    LCD_ShowChinese(46,47,"请稍后……",CYAN,LGRAY,0);
		    break;
	  //指示连接充电器
		case CapTest_WaitTypeCInsert:
			  LCD_ShowPicture(40,21,75,21,TellUserToInsertTypeC);
			  LCD_ShowChinese(27,47,"请将电池架连接到",WHITE,LGRAY,0);
		    LCD_ShowString(36,61,"Type-C",WHITE,LGRAY,12,0);
		    LCD_ShowChinese(81,61,"充电器",WHITE,LGRAY,0);
		    break;
	  //系统正在等待充满判断 
		case CapTest_ConfirmFull:
		    LCD_ShowChinese(20,19,"正在确认充满状态…",WHITE,LGRAY,0);
				LCD_ShowChinese(46,47,"请稍后……",WHITE,LGRAY,0);
		    break;
		case CapTest_OverCharge:
		    LCD_ShowChinese(20,19,"系统检测到过充事件",YELLOW,LGRAY,0);
				LCD_ShowChinese(20,47,"请等待事件解除……",WHITE,LGRAY,0);
		    break;			 
		//测容进行中
		case CapTest_Running:
			  LCD_ShowChinese(20,19,"充电测容进行中",CYAN,LGRAY,0);
		    //营造出省略号反复变化的效果指示充电进行中
	      time=CurrentTestResult.Data.ChargeTime&0x03;
        switch(time)
					{
					case 0:LCD_ShowChinese12x12(111,19,"…\0",CYAN,LGRAY,12,0);;break;
					case 1:LCD_ShowChinese(111,19,"……\0",CYAN,LGRAY,0);break;
					default : break;
					}
		    //显示充电时长
		    LCD_ShowChinese(3,35,"充电时长",WHITE,LGRAY,0);
		    if(CurrentTestResult.Data.ChargeTime>86400) //充电时长超过一天
					{
					time=CurrentTestResult.Data.ChargeTime/86400; //计算出天数
					LCD_ShowIntNum(60,35,time,2,YELLOW,LGRAY,12);
					LCD_ShowChinese12x12(79,35,"天\0",WHITE,LGRAY,12,0);
					time=(CurrentTestResult.Data.ChargeTime%86400)/3600; //计算出小时数
					LCD_ShowIntNum(93,35,time,2,YELLOW,LGRAY,12);
					LCD_ShowChinese12x12(111,35,"时\0",WHITE,LGRAY,12,0);
					time=(CurrentTestResult.Data.ChargeTime%3600)/60; //计算出分钟数
					LCD_ShowIntNum(124,35,time,2,YELLOW,LGRAY,12);
					LCD_ShowChinese12x12(143,35,"分\0",WHITE,LGRAY,12,0);		  
					}
				else //使用时分秒
					{
					time=CurrentTestResult.Data.ChargeTime/3600; //计算出小时数
					LCD_ShowIntNum(60,35,time,2,YELLOW,LGRAY,12);
					LCD_ShowChinese12x12(79,35,"时\0",WHITE,LGRAY,12,0);
					time=(CurrentTestResult.Data.ChargeTime%3600)/60; //计算出分钟数
					LCD_ShowIntNum(93,35,time,2,YELLOW,LGRAY,12);
					LCD_ShowChinese12x12(111,35,"分\0",WHITE,LGRAY,12,0);
					time=CurrentTestResult.Data.ChargeTime%60;
					LCD_ShowIntNum(124,35,time,2,YELLOW,LGRAY,12);
					LCD_ShowChinese12x12(143,35,"秒\0",WHITE,LGRAY,12,0);		  
					}
		    //显示充电功率和Wh数
		    LCD_ShowChinese(3,50,"能量",WHITE,LGRAY,0);
		    if(CurrentTestResult.Data.TotalWh<10)LCD_ShowFloatNum1(33,50,CurrentTestResult.Data.TotalWh,1,CYAN,LGRAY,12);
				else LCD_ShowIntNum(33,50,iroundf(CurrentTestResult.Data.TotalWh),3,CYAN,LGRAY,12);
		    LCD_ShowString(60,50,"Wh",CYAN,LGRAY,12,0);
				if(!IsDispChargingINFO)
					{
					LCD_ShowChinese(87,50,"功率",WHITE,LGRAY,0);
					Power=fabsf(ADCO.Vbatt*ADCO.Ibatt);
					if(Power<10)LCD_ShowFloatNum1(115,50,Power,1,GREEN,LGRAY,12);
					else LCD_ShowIntNum(115,50,iroundf(Power),3,GREEN,LGRAY,12);
					LCD_ShowChar(147,50,'W',GREEN,LGRAY,12,0);
					}
				else
					{
					LCD_ShowChinese(87,50,"电压",WHITE,LGRAY,0);
					LCD_ShowFloatNum1(115,50,ADCO.Vbatt,1,GREEN,LGRAY,12);
					LCD_ShowChar(147,50,'V',GREEN,LGRAY,12,0);
					}
				//显示Ah数
				LCD_ShowChinese(3,64,"容量",WHITE,LGRAY,0);
				if(CurrentTestResult.Data.TotalmAH<100)
					{
					LCD_ShowIntNum(33,64,iroundf(CurrentTestResult.Data.TotalmAH),2,RED,LGRAY,12);
					LCD_ShowString(51,64,"mAh",RED,LGRAY,12,0);
					}
				else if(CurrentTestResult.Data.TotalmAH<10000) //使用0.1Ah显示
					{
					LCD_ShowFloatNum1(33,64,CurrentTestResult.Data.TotalmAH/(float)1000,1,RED,LGRAY,12);
					LCD_ShowString(60,64,"Ah",RED,LGRAY,12,0);
					}					
				else //容量大于10Ah，使用整数Ah显示
					{
					LCD_ShowIntNum(33,64,iroundf(CurrentTestResult.Data.TotalmAH/1000),3,RED,LGRAY,12);
					LCD_ShowString(60,64,"Ah",RED,LGRAY,12,0);
					}
			 //温度显示
			LCD_ShowChinese(87,64,"温度",WHITE,LGRAY,0);
			if(!ADCO.IsNTCOK)LCD_ShowString(115,64,"---",WHITE,LGRAY,12,0);
			else
				{
				Temp=iroundf(ADCO.Systemp);
				if(Temp<0)Color=DARKBLUE;	
				else if(Temp<10)Color=BLUE;
				else if(Temp<CfgData.OverHeatLockTemp-20)Color=GREEN;
				else if(Temp<CfgData.OverHeatLockTemp-8)Color=YELLOW;
				else Color=RED;
				//负数温度，显示为负标识符
				if(Temp<0)
					{
					Temp*=-1;
					LCD_ShowChar(117,61,'-',Color,LGRAY,12,0);
					LCD_ShowIntNum(124,64,Temp,2,Color,LGRAY,12);
					}		
				//个位数温度，使用浮点显示
				else if(Temp<10)LCD_ShowFloatNum1(115,64,ADCO.Systemp,1,Color,LGRAY,12);		
				//其余温度，整数显示
				else LCD_ShowIntNum(115,64,Temp,2,Color,LGRAY,12);
				}
			//显示℃符号
			LCD_ShowChinese12x12(143,64,"℃\0",Color,LGRAY,12,0);	
		  break;
		//运行结束
		case CapTest_Finish:
			LCD_ShowChinese(33,22,"容量测试已完成",GREEN,LGRAY,0);
		  LCD_ShowChinese(24,41,"共充入",GREEN,LGRAY,0);
		  Power=CurrentTestResult.Data.TotalmAH/1000;
		  if(Power<10)LCD_ShowFloatNum1(67,41,Power,3,WHITE,LGRAY,12);
		  else if(Power<100)LCD_ShowFloatNum1(67,41,Power,2,WHITE,LGRAY,12);
		  else if(Power<1000)LCD_ShowFloatNum1(67,41,Power,1,WHITE,LGRAY,12);
		  else LCD_ShowIntNum(67,41,iroundf(Power),4,WHITE,LGRAY,12);
		  LCD_ShowString(112,41,"Ah",WHITE,LGRAY,12,0);
		  LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
		  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
		  break;
	  //确认退出
		case CapTest_ConfirmForceStopTest:
			LCD_ShowChinese(4,22,"强制结束本次测容并退出",RED,LGRAY,0);
		  LCD_ShowChar(147,22,'?',RED,LGRAY,12,0);
			LCD_ShowChinese(7,46,"同时按下",WHITE,LGRAY,0);
		  LCD_ShowString(59,46,"ENT",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese12x12(86,46,"和",WHITE,LGRAY,12,0);
		  LCD_ShowString(99,46,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(127,46,"退出",WHITE,LGRAY,0);
		  LCD_ShowChinese(7,62,"其余任意操作以继续",WHITE,LGRAY,0);
		  break;
		//异常退出
		case CapTest_EndERROR:
			LCD_ShowChinese(28,22,"容量测试异常结束",RED,LGRAY,0);
			LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
		  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
		  break;
	  //正在充放电
	  case CapTest_ErrorDischarging:
		case CapTest_ErrorAlreadyCharging:
			LCD_ShowChinese(28,22,"容量测试无法继续",RED,LGRAY,0);
		  LCD_ShowChinese(10,41,"系统",RED,LGRAY,0);
		  if(CFSMState==CapTest_ErrorAlreadyCharging)LCD_ShowChinese12x12(36,41,"充",RED,LGRAY,12,0);
			else LCD_ShowChinese12x12(36,41,"放",RED,LGRAY,12,0);
		  LCD_ShowChinese(50,41,"电中",RED,LGRAY,0);
 		  LCD_ShowChar(74,41,',',RED,LGRAY,12,0);  
			LCD_ShowChinese(83,41,"请移除线材",RED,LGRAY,0);
			LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
		  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
		  break;
		case CapTest_ErrorChipHang:
			LCD_ShowChinese(28,22,"容量测试无法继续",RED,LGRAY,0);
		  LCD_ShowChinese(21,41,"充放电管理芯片异常",RED,LGRAY,0);
			LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
		  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
		  break;
		case CapTest_ErrorBattToHigh:
			LCD_ShowChinese(28,22,"容量测试无法继续",RED,LGRAY,0);
		  LCD_ShowChinese(4,41,"请将电池放电至",RED,LGRAY,0);
		  LCD_ShowString(94,41,"12.3V",RED,LGRAY,12,0);
		  LCD_ShowChinese(133,41,"以内",RED,LGRAY,0);
			LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
		  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		  LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
		  break;
		}
	//渲染完毕
	IsUpDateGUI=true;
	}

//测容系统状态机处理
void CTestFSMHandler(void)
	{
	extern bool OCState;
	//状态机	
	switch(CFSMState)
		{
		//初状态
		case CapTest_Initial:
      //瞬时测容启动			
			if(CfgData.InstantCTest==InstantCTest_EnteredOK)
				{
				if(BATT==Batt_discharging)CFSMState=CapTest_ErrorDischarging;
				else if(BATT==Batt_ChgError)CFSMState=CapTest_ErrorChipHang;
  			else //直接进入到测试		
					 {	
					 CFSMState=CapTest_WaitTypeCInsert; //等待Type-C插入
					 CfgData.InstantCTest=InstantCTest_NotTriggered; //本次启动已触发
					 WriteConfiguration(&CfgUnion,false); //写入到not triggered
					 }
				break;
				}
	    if(BATT==Batt_discharging)CFSMState=CapTest_ErrorDischarging;
	    else if(BATT==Batt_ChgError)CFSMState=CapTest_ErrorChipHang;
		  else if(BATT!=Batt_StandBy||VBUS.IsTypeCConnected)CFSMState=CapTest_ErrorAlreadyCharging;
		  else if(ADCO.Vbatt>12.3)CFSMState=CapTest_ErrorBattToHigh;
		  else CFSMState=CapTest_WaitTypeCInsert; //等待Type-C插入
			IsUpDateGUI=false; //发送指令重绘GUI
		  break;
		//等待Type-C插入
		case CapTest_WaitTypeCInsert:
			if(!VBUS.IsTypeCConnected)break; //TypeC未接入
		  switch(BATT)
				{
				case Batt_discharging:CFSMState=CapTest_ErrorDischarging;break; //进入放电提示测容启动失败
				case Batt_PreChage:
				case Batt_CCCharge:
				case Batt_CVCharge:
				case Batt_ChgDone: //开始测容
					 CurrentTestResult.Data.StartVbatt=ADCO.Vbatt;
					 VBattSumbuf=0;
					 IBattsumbuf=0;
					 AverageCounter=8; //复位结果
				   CFSMState=CapTest_Running;
					 break;
				case Batt_ChgError:CFSMState=CapTest_ErrorChipHang;break; //芯片异常
				default:break;
				}
			if(CFSMState==CapTest_Running)IsUpDateGUI=false; //发送指令重绘GUI
		  break;
		//过充事件
		case CapTest_OverCharge:		
       //条件跳转			
			 if(!VBUS.IsTypeCConnected)CFSMState=CapTest_EndERROR;
			 else if(BATT==Batt_discharging)CFSMState=CapTest_ErrorDischarging;
			 else if(BATT==Batt_ChgError)CFSMState=CapTest_ErrorChipHang;
			 else if(!OCState)CFSMState=CapTest_Running; //过充事件解除，继续充电
		 //重绘GUI检测
			 if(CFSMState!=CapTest_OverCharge)IsUpDateGUI=false; //发送指令重绘GUI
		   break;
		//测容等待中
		case CapTest_ConfirmFull:
			 //条件跳转
			 if(!VBUS.IsTypeCConnected)CFSMState=CapTest_EndERROR;
			 else if(BATT==Batt_discharging)CFSMState=CapTest_ErrorDischarging;
			 else if(BATT==Batt_ChgError)CFSMState=CapTest_ErrorChipHang;
		   else if(OCState)CFSMState=CapTest_OverCharge; //过充事件bit置起，标记过充发生
		   else if(BATT!=Batt_ChgDone)CFSMState=CapTest_Running; //芯片回到非充满状态
	     //重绘GUI检测
			 if(CFSMState!=CapTest_ConfirmFull)IsUpDateGUI=false; //发送指令重绘GUI
		   //成功完成
			 if(ConfirmTimeCounter>0)break;
		   IsUpDateGUI=false; //发送指令重绘GUI
			 CFSMState=CapTest_Finish;
			 CurrentTestResult.Data.IsDataValid=true;
			 CurrentTestResult.Data.MaxChargeRatio=(CurrentTestResult.Data.MaxChargeCurrent*1000)/CurrentTestResult.Data.TotalmAH; //最大C数等于最大电流/总容量
			 memcpy(CTestData.ROMImage.Data.ByteBuf,CurrentTestResult.ByteBuf,sizeof(ChargeTestUnionDef)); //更新当前测容数据
			 WriteCapData(&CurrentTestResult,false);
		   break;
		//测容运行中
		case CapTest_Running:	
			if(!VBUS.IsTypeCConnected)CFSMState=CapTest_EndERROR;
		  else if(BATT==Batt_discharging)CFSMState=CapTest_ErrorDischarging;
	    else if(BATT==Batt_ChgError)CFSMState=CapTest_ErrorChipHang;
		  else if(OCState)CFSMState=CapTest_OverCharge; //过充事件bit置起，标记过充发生
			else if(BATT==Batt_ChgDone) //成功完成测容，保存数据
				{			
        ConfirmTimeCounter=960;
				CFSMState=CapTest_ConfirmFull; //等待满电
				}
			if(CFSMState!=CapTest_Running)IsUpDateGUI=false; //发送指令重绘GUI
			break;
	  //其余情况不响应
		default: break;
		}
	}
	
	
const MenuConfigDef CapTestMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
  &CTestGUIHandler,
	&CTestKeyHandler,	
	//不是设置菜单不需要用别的事情
	"一键充电测容\0",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&ResetCapTestSystem,
	&ResetCapTestSystem
	};
