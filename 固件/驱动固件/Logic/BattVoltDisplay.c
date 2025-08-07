#include "ADCCfg.h"
#include "LEDMgmt.h"
#include "delay.h"
#include "ModeControl.h"
#include "OutputChannel.h"
#include "SideKey.h"
#include "BattDisplay.h"
#include "SelfTest.h"
#include "LocateLED.h"
#include "FastOp.h"
#include "SetupMenu.h"

//内部flag
bit IsBatteryAlert; //电池电压低于警告值	
bit IsBatteryFault; //电池电压低于保护值		

//内部变量
static unsigned char BattShowTimer; //电池电量显示计时
static unsigned char OneLMShowBattStateTimer=0; //1LM模式下显示电池状态的计时器
static xdata AverageCalcDef BattVolt;	
static xdata unsigned char LowVoltStrobeTIM;
static xdata int VbattSample; //取样的电池电压

//外部全局变量
BattStatusDef BattState; //电池电量标记位
xdata int CellVoltage; //等效单节电池电压
xdata unsigned char CommonSysFSMTIM;  //电压显示计时器
xdata BattVshowFSMDef VshowFSMState; //电池电压显示所需的计时器和状态机转移
static bit IsReportingTemperature=0; //报告温度

//内部使用的先导显示表
static code LEDStateDef VShowIndexCode[]=
	{
	LED_Red,
	LED_Amber,
	LED_Green,  //正常过渡是红黄绿
	LED_Amber,
	LED_Red  //高精度模式是反过来，绿红黄
	};

//准备电压显示状态机的模块
static void VShowFSMPrepare(void)	
	{
	VshowFSMState=BattVdis_PrepareDis;	
	if(CurrentMode->ModeIdx!=Mode_OFF)
		{
		if(LEDMode!=LED_OFF)CommonSysFSMTIM=8; //指示灯点亮状态查询电量，熄灭LED等一会
		LEDMode=LED_OFF;
		}	
	}

//启动系统温度显示
void TriggerTShowDisplay(void)
	{
	if(!Data.IsNTCOK||VshowFSMState!=BattVdis_Waiting)return; //非等待显示状态禁止操作
	VShowFSMPrepare();
	//进行温度取样
	IsReportingTemperature=1;
	if(IsNegative8(Data.Systemp))VbattSample=(int)Data.Systemp*-10;
	else VbattSample=(int)Data.Systemp*10;
	}

//启动电池电压显示
void TriggerVshowDisplay(void)	
	{
	if(VshowFSMState!=BattVdis_Waiting)return; //非等待显示状态禁止操作
	VShowFSMPrepare();
	//进行电压取样(缩放为LSB=0.01V)
	VbattSample=(int)(Data.RawBattVolt*100); 		
	}		

//生成低电量提示报警
bit LowPowerStrobe(void)
	{
	//月光挡位下计时也停止	
	if(CurrentMode->ModeIdx==Mode_Moon)LowVoltStrobeTIM=0;
	//电量正常,或者是1LM模式，不启动计时
	else if(CurrentMode->ModeIdx==Mode_1Lumen||BattState!=Battery_VeryLow)LowVoltStrobeTIM=0;
	//电量异常开始计时
	else if(!LowVoltStrobeTIM)LowVoltStrobeTIM=1; //启动计时器
	else if(LowVoltStrobeTIM>((LowVoltStrobeGap*8)-4))return 1; //触发闪烁标记电流为0
	//其余情况返回0
	return 0;
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

//电池采样显示电压
LEDStateDef VshowEnter_ShowIndex(void)
	{
	char Index;
	if(CommonSysFSMTIM>9)
		{
		Index=((CommonSysFSMTIM-8)>>1)-1;
		if(IsReportingTemperature&&!(Data.Systemp&0x80))Index+=2;//温度播报时温度为正数，使用常规显示模式
		if(!IsReportingTemperature&&VbattSample>999)Index+=2; //电压播报时传入电压大于10V,使用常规显示模式
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
			//电池电压为大于10V的数，进行四舍五入处理保留小数点后一位的结果
		  if(VbattSample>999)
			   {
				 /********************************************************
				 这里四舍五入的原理是电池电压会被采样为整数，1LSB=0.01V。例如
				 电池电压为12.59V采样之后就会变成1259。那么此时我们需要对小数
				 点后两位进行四舍五入判断，得到一位小数的结果。由于整数结果的
				 个位实际上等于浮点的电池电压中的小数点后两位，因此我们只需要
				 通过和10求余数就可以取出小数点后结果的2位，然后如果结果大于4
				 则进行进位，令小数点后一位+1就实现了四舍五入了。对整个采样结
				 果除以10之后就会自动去掉小数点后两位的值保留1位小数。
				 *********************************************************/					 
				 if((VbattSample%10)>4)VbattSample+=10;
				 VbattSample/=10;
				 }
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
			//温度播报结束之后直接进入等待阶段
			if(IsReportingTemperature)
				{
				CommonSysFSMTIM=10;  
				VshowFSMState=BattVdis_WaitShowTempState; 
				}
			else VshowFSMGenTIMValue(VbattSample%10,BattVdis_Show0_1V);
			break;
		//显示小数点后一位(0.1V)
		case BattVdis_Show0_1V:
		  VshowGenerateSideStrobe(LED_Green,BattVdis_WaitShowChargeLvl); //调用处理函数生成绿色侧部闪烁
			break;
		//等待一段时间后显示当前温度水平
		case BattVdis_WaitShowTempState: 
			if(CommonSysFSMTIM)break;
			VshowFSMState=BattVdis_ShowTempState;
		  CommonSysFSMTIM=31;
			break;
	 
		//等待当前温度水平显示结束
		case BattVdis_ShowTempState:
			if(CommonSysFSMTIM<25&&CommonSysFSMTIM&0xF8)
				{
				if(Data.Systemp<50)LEDMode=LED_Green;
				else if(Data.Systemp<62)LEDMode=LED_Amber;
				else LEDMode=LED_Red;
				}
			//显示结束，LED熄灭一段时间
			else LEDMode=LED_OFF;
			//等待温度状态显示时间到，到了之后跳转到等待用户松开按键的处理
			if(!CommonSysFSMTIM)VshowFSMState=BattVdis_ShowChargeLvl;
			break;		  
		//等待一段时间后显示当前电量
		case BattVdis_WaitShowChargeLvl:
			if(CommonSysFSMTIM)break;
			//1LM模式以及关机下电量指示灯不常驻点亮，所以需要额外给个延时让LED点亮
			if(CurrentMode->ModeIdx==Mode_1Lumen||CurrentMode->ModeIdx==Mode_OFF)BattShowTimer=18; 
			VshowFSMState=BattVdis_ShowChargeLvl; //等待电量显示状态结束
      break;
	  //等待总体电量显示结束
		case BattVdis_ShowChargeLvl:
			IsReportingTemperature=0;  									//clear掉温度显示标志位
			VbattSample=0;                              //电压显示每次结束后，clear掉电压缓存数据
		  if(BattShowTimer)SetPowerLEDBasedOnVbatt();//显示电量
			else if(!getSideKeyNClickAndHoldEvent())VshowFSMState=BattVdis_Waiting; //用户仍然按下按键，等待用户松开,松开后回到等待阶段
      break;
		}
	}
//电池电量状态机
static void BatteryStateFSM(void)
	{
	int thres;
	//计算阈值
  if(!CurrentMode->ModeIdx==Mode_Turbo)thres=3650;
  else thres=3550;
	if(CurrentBuf>CalcIREFValue(19000))thres-=100;  //输出电流大于19000mA时减少100mA阈值避免错误黄灯
	//状态机处理	
	switch(BattState) 
		 {
		 //电池电量充足
		 case Battery_Plenty: 
				if(CellVoltage<thres)BattState=Battery_Mid; //电池电压小于指定阈值，回到电量中等状态
			  break;
		 //电池电量较为充足
		 case Battery_Mid:
			  if(CellVoltage>(thres+250))BattState=Battery_Plenty; //电池电压大于阈值，回到充足状态
				if(CellVoltage<(thres-200))BattState=Battery_Low; //电池电压低于3.3则切换到电量低的状态
				break;
		 //电池电量不足
		 case Battery_Low:
		    if(CellVoltage>(thres+50))BattState=Battery_Mid; //电池电压高于3.6，切换到电量中等的状态
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
	//启动电池电量显示(仅无错误的情况下)
	if(!IsPOR||CurrentMode->ModeIdx!=Mode_OFF)return;
	BattShowTimer=18;
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
	//低电压提示闪烁计时器
	if(LowVoltStrobeTIM==LowVoltStrobeGap*8)LowVoltStrobeTIM=1;//时间到清除数值重新计时
	else if(LowVoltStrobeTIM)LowVoltStrobeTIM++;
	//1LM模式下交替显示的计时器
	if(OneLMShowBattStateTimer)OneLMShowBattStateTimer--;	
	//电池电压显示的计时器处理	
	if(CommonSysFSMTIM)CommonSysFSMTIM--;
	//电池显示定时器
	if(BattShowTimer)BattShowTimer--;
	}

//电池参数测量和指示灯控制
void BatteryTelemHandler(void)
	{
	int AlertThr;
	extern bit IsDisplayLocked;
	//根据电池电压控制flag实现低电压降档和关机保护
	if(CurrentMode->ModeIdx==Mode_Ramp)AlertThr=SysCfg.RampBattThres; //无极调光模式下，使用结构体内的动态阈值
	else AlertThr=CurrentMode->LowVoltThres; //从当前目标挡位读取模式值  
  if(CellVoltage>2650)		
		{
		IsBatteryAlert=CellVoltage>AlertThr?0:1; //警报bit根据各个挡位的阈值进行判断
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
	if(ErrCode!=Fault_None&&!IsDisplayLocked)DisplayErrorIDHandler(); //有故障发生且并非应急允许开机的故障码，显示错误
	else if(VshowFSMState!=BattVdis_Waiting)BatVshowFSM();//电池电压显示启动，执行状态机
	else if(LocLEDState&0x02)LEDMode=LocateLED_ShowType(); //系统进入LED编辑状态，显示编辑数据
	else if(SetupFSMState!=SetupMenu_InACT)LEDMode=SetupMenuFSM(); //系统进入设置菜单编辑器，显示编辑数据
	else if(CurrentMode->ModeIdx!=Mode_OFF||BattShowTimer)ShowBatteryState(); //用户查询电量或者手电开机，指示电量
  else LEDMode=LED_OFF; //手电处于关闭状态，且没有按键按下的动静，故LED设置为关闭
	}
	