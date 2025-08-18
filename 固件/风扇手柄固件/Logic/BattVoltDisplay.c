#include "ADCCfg.h"
#include "LEDMgmt.h"
#include "delay.h"
#include "SideKey.h"
#include "BattDisplay.h"
#include "FastOp.h"
#include "OutputChannel.h"
#include "ModeSel.h"

//内部flag
bit IsBatteryAlert; //电池电压低于警告值	
bit IsBatteryFault; //电池电压低于保护值		

//内部变量
static xdata unsigned char BattShowTimer=0; //电池电量显示计时
static xdata unsigned char OneLMShowBattStateTimer=0; //1LM模式下显示电池状态的计时器
static xdata AverageCalcDef BattVolt;	
static xdata int VbattSample; //取样的电池电压
static xdata BattStatusDef BattState; //电池电量标记位

//外部全局变量
xdata int CellVoltage; //等效单节电池电压
xdata unsigned char CommonSysFSMTIM;  //电压显示计时器
xdata BattVshowFSMDef VshowFSMState; //电池电压显示所需的计时器和状态机转移

//内部使用的先导显示表
static code LEDStateDef VShowIndexCode[]=
	{
	LED_Green,
	LED_Amber,
	LED_Red  //绿黄红过度
	};

//启动电池电压显示
void TriggerVshowDisplay(void)	
	{
	if(VshowFSMState!=BattVdis_Waiting)return; //非等待显示状态禁止操作
	VshowFSMState=BattVdis_PrepareDis;	
	if(GetIfFanOutputEnabled())
		{
		if(LEDMode!=LED_OFF)CommonSysFSMTIM=8; //指示灯点亮状态查询电量，熄灭LED等一会
		LEDMode=LED_OFF;
		}	
	//进行电压取样(缩放为LSB=0.01V)
	VbattSample=(int)(Data.BatteryVoltage*100); 		
	}		

//控制LED侧按产生闪烁指示电池电压的处理
static void VshowGenerateSideStrobe(LEDStateDef Color,BattVshowFSMDef NextStep)
	{
	//传入的是负数，符号位=1，通过快闪一次表示是0
	if(IsNegative8(CommonSysFSMTIM))
		{
		MakeFastStrobe(Color);
		CommonSysFSMTIM=0; 
		}
	//正常指示
	LEDMode=(CommonSysFSMTIM%4)&0x7E?Color:LED_OFF; //制造红色闪烁指示对应位的电压
	//显示结束
	if(!CommonSysFSMTIM) 
		{
		LEDMode=LED_OFF;
		CommonSysFSMTIM=10;
		VshowFSMState=NextStep; //等待一会
		}
	}
//电压显示状态机根据对应的电压位数计算出闪烁定时器的配置值
static void VshowFSMGenTIMValue(int Vsample,BattVshowFSMDef NextStep)
	{
	if(!CommonSysFSMTIM)	//时间到允许配置
		{	
		if(!Vsample)CommonSysFSMTIM=0x80; //0x80=瞬间闪一下
		else CommonSysFSMTIM=(4*Vsample)-1; //配置显示的时长
		VshowFSMState=NextStep; //执行下一步显示
		}
	}
	
//根据电池状态机设置LED指示电池电量
static void SetPowerLEDBasedOnVbatt(void)	
	{
	switch(BattState)
		{
		 case Battery_Plenty:LEDMode=LED_Green;break; //电池电量充足绿色常亮
		 case Battery_Mid:LEDMode=LED_Amber;break; //电池电量中等黄色常亮
		 case Battery_Low:LEDMode=LED_Red;break;//电池电量不足
		 case Battery_VeryLow:LEDMode=LED_RedBlink;break; //电池电量严重不足红色慢闪
		}
	}
//电池采样显示电压
LEDStateDef VshowEnter_ShowIndex(void)
	{
	char Index;
	if(CommonSysFSMTIM>9)
		{
		Index=((CommonSysFSMTIM-8)>>1)-1;
		return VShowIndexCode[Index];
		}
	return LED_OFF; //红黄绿闪烁之后(如果是高精度显示模式则为绿红黄)等待
	}

//电池详细电压显示的状态机处理
static void BatVshowFSM(void)
	{
	//电量显示状态机
	switch(VshowFSMState)
		{
		case BattVdis_PrepareDis: //准备显示
			if(CommonSysFSMTIM)break;
	    CommonSysFSMTIM=15; //延迟1.75秒
			VshowFSMState=BattVdis_DelayBeforeDisplay; //显示头部
		  break;
		//延迟并显示开头
		case BattVdis_DelayBeforeDisplay: 
			//头部显示结束后开始正式显示电压
			LEDMode=VshowEnter_ShowIndex();
		  if(CommonSysFSMTIM)break;
			//配置计时器显示第一组电压
			VshowFSMGenTIMValue(VbattSample/100,BattVdis_Show10V);
		  break;
    //显示十位
		case BattVdis_Show10V:
			VshowGenerateSideStrobe(LED_Red,BattVdis_Gap10to1V); //调用处理函数生成红色侧部闪烁
		  break;
		//十位和个位之间的间隔
		case BattVdis_Gap10to1V:
			VbattSample%=100;
			VshowFSMGenTIMValue(VbattSample/10,BattVdis_Show1V); //配置计时器开始显示下一组	
			break;	
		//显示个位
		case BattVdis_Show1V:
		  VshowGenerateSideStrobe(LED_Amber,BattVdis_Gap1to0_1V); //调用处理函数生成黄色侧部闪烁
		  break;
		//个位和十分位之间的间隔		
		case BattVdis_Gap1to0_1V:	
			VshowFSMGenTIMValue(VbattSample%10,BattVdis_Show0_1V);
			break;
		//显示小数点后一位(0.1V)
		case BattVdis_Show0_1V:
		  VshowGenerateSideStrobe(LED_Green,BattVdis_WaitShowChargeLvl); //调用处理函数生成绿色侧部闪烁
			break;
		
		//等待一段时间后显示当前电量
		case BattVdis_WaitShowChargeLvl:
			if(CommonSysFSMTIM)break;
			//关机下电量指示灯不常驻点亮，所以需要额外给个延时让LED点亮
			if(!GetIfFanOutputEnabled()||CurrentMode->ModeIdx==Mode_OFF)BattShowTimer=18; 
			VshowFSMState=BattVdis_ShowChargeLvl; //等待电量显示状态结束
      break;
	  //等待总体电量显示结束
		case BattVdis_ShowChargeLvl:
			VbattSample=0;                              //电压显示每次结束后，clear掉电压缓存数据
		  if(BattShowTimer)SetPowerLEDBasedOnVbatt();//显示电量
			else if(!getSideKeyNClickAndHoldEvent())VshowFSMState=BattVdis_Waiting; //用户仍然按下按键，等待用户松开,松开后回到等待阶段
      break;
		}
	}
//电池电量状态机
static void BatteryStateFSM(void)
	{
	//状态机处理	
	switch(BattState) 
		 {
		 //电池电量充足
		 case Battery_Plenty: 
				if(CellVoltage<3700)BattState=Battery_Mid; //电池电压小于3.7V，回到电量中等状态
			  break;
		 //电池电量较为充足
		 case Battery_Mid:
			  if(CellVoltage>4000)BattState=Battery_Plenty; //电池电压大于阈值，回到充足状态
				if(CellVoltage<3300)BattState=Battery_Low; //电池电压低于3.3则切换到电量低的状态
				break;
		 //电池电量不足
		 case Battery_Low:
		    if(CellVoltage>3600)BattState=Battery_Mid; //电池电压高于3.6，切换到电量中等的状态
			  if(CellVoltage<2950)BattState=Battery_VeryLow; //电池电压低于3.0，报告严重不足
		    break;
		 //电池电量严重不足
		 case Battery_VeryLow:
			  if(CellVoltage>3300)BattState=Battery_Low; //电池电压回升到3.3，跳转到电量不足阶段
		    break;
		 }
	}

//复位电池电压检测缓存
static void ResetBattAvg(void)	
	{
	BattVolt.Min=32766;
	BattVolt.Max=-32766; //复位最大最小捕获器
	BattVolt.Count=0;
  BattVolt.AvgBuf=0; //清除平均计数器和缓存
	}
	
//在启动时显示电池电压
void DisplayVBattAtStart(bit IsPOR)
	{
	unsigned char i=10;
	//初始化平均值缓存,复位标志位
	ResetBattAvg();
  //复位电池电压状态和电池显示状态机
  VshowFSMState=BattVdis_Waiting;		
	do
		{
		SystemTelemHandler();
		CellVoltage=(int)(Data.BatteryVoltage*1000); //获取并更新电池电压
		BatteryStateFSM(); //反复循环执行状态机更新到最终的电池状态
		}
	while(--i);
	//启动电池电量显示(仅系统使能的情况下)
	if(!IsPOR)return;
	BattShowTimer=18;
	}
	
//触发电池电量提示
void TriggerBattStatDisplay(void)
	{
  //电量显示进行中不允许操作
	if(BattShowTimer)return;
	//设置定时器，启动显示
	BattShowTimer=14;
	}

//电池电量显示延时的处理
void BattDisplayTIM(void)
	{
	long buf;
	//电量平均模块计算
	if(BattVolt.Count<VBattAvgCount)		
		{
		buf=(long)(Data.BatteryVoltage*1000);
		BattVolt.Count++;
		BattVolt.AvgBuf+=buf;
		if(BattVolt.Min>buf)BattVolt.Min=buf;
		if(BattVolt.Max<buf)BattVolt.Max=buf; //极值读取
		}
	else //平均次数到，更新电压
		{
		BattVolt.AvgBuf-=(long)BattVolt.Min+(long)BattVolt.Max; //去掉最高最低
		BattVolt.AvgBuf/=(long)(BattVolt.Count-2); //求平均值
		CellVoltage=(int)BattVolt.AvgBuf;	//得到最终的电池电压(单位mV)
		ResetBattAvg(); //复位缓存
		}
	//电池电压显示的计时器处理	
	if(CommonSysFSMTIM)CommonSysFSMTIM--;
	//电池显示定时器
	if(BattShowTimer)BattShowTimer--;
	}

//等待电池电压就绪（安全保护）
void WaitBatteryVoltageOK(void)
	{
	unsigned char Wait=200;
	do
		{		
		//延迟10mS采样电池电压
		delay_ms(10);
		SystemTelemHandler();
		//如果电池电压正常则退出
		if(Data.BatteryVoltage>2.40)return;
		}
	while(--Wait);
	//电池电压不正常，禁止固件启动并亮红灯
	LEDMode=LED_Red;
	while(1)LEDControlHandler();
	}	

//电池参数测量和指示灯控制
void BatteryTelemHandler(void)
	{
	//根据电池电压控制flag实现低电压降档和关机保护
  if(CellVoltage>2820)		
		{
		if(IsBatteryFault)
			{
			//故障bit置起，令警告bit始终=0，并且检测直到电池电压回升到足以解除的等级后clear掉故障flag
			if(CellVoltage>3000)IsBatteryFault=0;
			IsBatteryAlert=0;
			}
		else IsBatteryAlert=CellVoltage>CurrentMode->LowVoltThres?0:1; //警报bit根据各个挡位的阈值进行判断
		}
	else
		{
		IsBatteryAlert=0; //故障bit置起后强制清除警报bit
		IsBatteryFault=1; //故障bit=1
		}
	//电池电量指示状态机
	BatteryStateFSM();
	//LED控制
	if(IsOneTimeStrobe())return; //为了避免干扰只工作一次的频闪指示，不执行控制 
	else if(VshowFSMState!=BattVdis_Waiting)BatVshowFSM();//电池电压显示启动，执行状态机
	else if((GetIfFanOutputEnabled()&&CurrentMode->ModeIdx!=Mode_OFF)||BattShowTimer)
		{
		//用户查询电量或者风扇手柄开机，指示电量
		SetPowerLEDBasedOnVbatt(); 
		}
  else LEDMode=LED_OFF; //风扇手柄处于关闭状态，且没有按键按下的动静，故LED设置为关闭
	}
	