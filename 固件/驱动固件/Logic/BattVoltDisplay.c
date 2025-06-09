#include "ADCCfg.h"
#include "LEDMgmt.h"
#include "delay.h"
#include "ModeControl.h"
#include "SideKey.h"
#include "BattDisplay.h"
#include "SelfTest.h"
#include "LocateLED.h"

//内部flag
bit IsBatteryAlert; //电池电压低于警告值	
bit IsBatteryFault; //电池电压低于保护值		

//内部变量
static char BattShowTimer; //电池电量显示计时
static char OneLMShowBattStateTimer=0; //1LM模式下显示电池状态的计时器
static xdata AverageCalcDef BattVolt;	
static xdata signed char VshowTIM;  //电压显示计时器
static char LowVoltStrobeTIM;
static xdata int VbattSample; //取样的电池电压

//外部全局变量
BattStatusDef BattState; //电池电量标记位
xdata float Battery; //等效单节电池电压
xdata BattVshowFSMDef VshowFSMState; //电池电压显示所需的计时器和状态机转移

//内部使用的先导显示表
static code LEDStateDef VShowIndexCode[]=
	{
	LED_Red,
	LED_Amber,
	LED_Green,  //正常过渡是红黄绿
	LED_Amber,
	LED_Red  //高精度模式是反过来，绿红黄
	};

//启动电池电压显示
void TriggerVshowDisplay(void)	
	{
	if(VshowFSMState!=BattVdis_Waiting)return; //非等待显示状态禁止操作
	VshowFSMState=BattVdis_PrepareDis;
	if(CurrentMode->ModeIdx!=Mode_OFF)
		{
		if(LEDMode!=LED_OFF)VshowTIM=8; //指示灯点亮状态查询电量，熄灭LED等一会
		LEDMode=LED_OFF;
		}
	}		

//生成低电量提示报警
bit LowPowerStrobe(void)
	{
	//电量正常,或者是1LM模式，不启动计时
	if(CurrentMode->ModeIdx==Mode_1Lumen||BattState!=Battery_VeryLow)LowVoltStrobeTIM=0;
	//电量异常开始计时
	else if(!LowVoltStrobeTIM)LowVoltStrobeTIM=1; //启动计时器
	else if(LowVoltStrobeTIM>((LowVoltStrobeGap*8)-4))return 1; //触发闪烁标记电流为0
	//其余情况返回0
	return 0;
	}
	
//控制LED侧按产生闪烁指示电池电压的处理
static void VshowGenerateSideStrobe(LEDStateDef Color,BattVshowFSMDef NextStep)
	{
	//传入负数，通过快闪一次表示是0
	if(VshowTIM&0x80)
		{
		MakeFastStrobe(Color);
		VshowTIM=0; 
		}
	//正常指示
	LEDMode=(VshowTIM%4)>1?Color:LED_OFF; //制造红色闪烁指示对应位的电压
	//显示结束
	if(!VshowTIM) 
		{
		LEDMode=LED_OFF;
		VshowTIM=10;
		VshowFSMState=NextStep; //等待一会
		}
	}
//电压显示状态机根据对应的电压位数计算出闪烁定时器的配置值
static void VshowFSMGenTIMValue(int Vsample,BattVshowFSMDef NextStep)
	{
	if(!VshowTIM)	//时间到允许配置
		{	
		if(!Vsample)VshowTIM=-1; //0=瞬间闪一下
		else VshowTIM=(4*Vsample)-1; //配置显示的时长
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

//在手电工作时根据系统状态显示电池状态
static void ShowBatteryState(void)	
	{
	bit IsShowBatteryState;
	//非1LM挡位，正常显示
	if(CurrentMode->ModeIdx!=Mode_1Lumen)IsShowBatteryState=1;
	//1LM挡位下如果电池电量严重过低，显示
	else if(BattState==Battery_VeryLow)IsShowBatteryState=1;
	//1LM挡位下基于计时器正常显示电量
	else
		{
		if(!OneLMShowBattStateTimer)OneLMShowBattStateTimer=82;
		IsShowBatteryState=OneLMShowBattStateTimer>2?0:1;
		}		
	//根据结果选择是否调用函数显示电量	
	if(IsShowBatteryState)SetPowerLEDBasedOnVbatt();
	else LEDMode=LED_OFF;  //非显示状态需要保持LED熄灭
	}

//电池详细电压显示的状态机处理
static void BatVshowFSM(void)
	{
	char Index;
	//电量显示状态机
	switch(VshowFSMState)
		{
		case BattVdis_PrepareDis: //准备显示
			if(VshowTIM)break;
	    VshowTIM=15; //延迟1.75秒
			VshowFSMState=BattVdis_DelayBeforeDisplay; //显示头部
		  //进行电压取样(缩放为LSB=0.01V)
			VbattSample=(int)(Data.RawBattVolt*100); 
		  break;
		//延迟并显示开头
		case BattVdis_DelayBeforeDisplay:
			if(VshowTIM>9)
				{
				Index=((VshowTIM-8)>>1)-1;
				if(VbattSample>999)Index+=2; //传入电压大于10V，使用常规显示模式
				LEDMode=VShowIndexCode[Index];
				}
		  else LEDMode=LED_OFF; //红黄绿闪烁之后(如果是高精度显示模式则为绿红黄)等待
		  //头部显示结束后开始正式显示电压
		  if(VshowTIM>0)break;
			//电池电压超过显示范围，进行限幅
		  if(VbattSample>999)VbattSample/=10;
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
			if(VshowTIM>0)break;
			if(CurrentMode->ModeIdx==Mode_1Lumen)BattShowTimer=12; //1LM模式下电量指示灯不常驻点亮，所以需要额外给个延时让LED点亮
		  else BattShowTimer=CurrentMode->ModeIdx!=Mode_OFF?0:12; //启动总体电量显示
			VshowFSMState=BattVdis_ShowChargeLvl; //等待电量显示状态结束
      break;
	  //等待总体电量显示结束
		case BattVdis_ShowChargeLvl:
		  if(BattShowTimer)SetPowerLEDBasedOnVbatt(); //显示电量
			else if(!getSideKeyNClickAndHoldEvent())VshowFSMState=BattVdis_Waiting; //用户仍然按下按键，等待用户松开,松开后回到等待阶段
      break;
		}
	}
//电池电量状态机
static void BatteryStateFSM(void)
	{
	float Thres;
	//进行极亮阈值计算
	if(CurrentMode->ModeIdx!=Mode_Turbo)Thres=3.7;
	else Thres=3.5;
	//状态机处理	
	switch(BattState) 
		 {
		 //电池电量充足
		 case Battery_Plenty: 
				if(Battery<Thres)BattState=Battery_Mid; //电池电压小于3.7，回到电量较低状态
			  break;
		 //电池电量较为充足
		 case Battery_Mid:
			  if(Battery>(Thres+0.2))BattState=Battery_Plenty; //电池电压大于3.8，回到充足状态
				if(Battery<3.0)BattState=Battery_Low; //电池电压低于3.2则切换到电量低的状态
				break;
		 //电池电量不足
		 case Battery_Low:
		    if(Battery>3.2)BattState=Battery_Mid; //电池电压高于3.5，切换到电量中等的状态
			  if(Battery<2.8)BattState=Battery_VeryLow; //电池电压低于2.8，报告严重不足
		    break;
		 //电池电量严重不足
		 case Battery_VeryLow:
			  if(Battery>3.0)BattState=Battery_Low; //电池电压回升到3.0，跳转到电量不足阶段
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
	char i=10;
	//初始化平均值缓存,复位标志位
	ResetBattAvg();
  //复位电池电压状态和电池显示状态机
  VshowFSMState=BattVdis_Waiting;		
	do
		{
		SystemTelemHandler();
		Battery=Data.BatteryVoltage; //获取并更新电池电压
		BatteryStateFSM(); //反复循环执行状态机更新到最终的电池状态
		}
	while(--i);
	//启动电池电量显示(仅无错误的情况下)
	if(!IsPOR||CurrentMode->ModeIdx!=Mode_OFF)return;
	BattShowTimer=12;
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
		Battery=(float)BattVolt.AvgBuf/(float)1000; //得到最终的电池电压
		ResetBattAvg(); //复位缓存
		}
	//低电压提示闪烁计时器
	if(LowVoltStrobeTIM==LowVoltStrobeGap*8)LowVoltStrobeTIM=1;//时间到清除数值重新计时
	else if(LowVoltStrobeTIM)LowVoltStrobeTIM++;
	//1LM模式下交替显示的计时器
	if(OneLMShowBattStateTimer)OneLMShowBattStateTimer--;	
	//电池电压显示的计时器处理	
	if(VshowTIM>0)VshowTIM--;
	//电池显示定时器
	if(BattShowTimer)BattShowTimer--;
	}

//电池参数测量和指示灯控制
void BatteryTelemHandler(void)
	{
	int AlertThr,VBatt;
	//根据电池电压控制flag实现低电压降档和关机保护
	if(CurrentMode->ModeIdx==Mode_Ramp)AlertThr=SysCfg.RampBattThres; //无极调光模式下，使用结构体内的动态阈值
	else AlertThr=CurrentMode->LowVoltThres; //从当前目标挡位读取模式值  
	VBatt=(int)(Battery*1000); //得到电池电压(mV)
  if(VBatt>2650)		
		{
		IsBatteryAlert=VBatt>AlertThr?0:1; //警报bit根据各个挡位的阈值进行判断
		IsBatteryFault=0; //电池电压没有低于危险值，fault=0
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
	if(ErrCode!=Fault_None)DisplayErrorIDHandler(); //有故障发生，显示错误
	else if(VshowFSMState!=BattVdis_Waiting)BatVshowFSM();//电池电压显示启动，执行状态机
	else if(LocLEDState==LocateLED_Sel)LEDMode=LocateLED_ShowType(); //进入LED编辑
	else if(CurrentMode->ModeIdx!=Mode_OFF||BattShowTimer)ShowBatteryState(); //用户查询电量或者手电开机，指示电量
  else LEDMode=LED_OFF; //手电处于关闭状态，且没有按键按下的动静，故LED设置为关闭
	}
	