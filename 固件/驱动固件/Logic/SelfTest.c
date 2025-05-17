#include "LEDMgmt.h"
#include "delay.h"
#include "ADCCfg.h"
#include "BattDisplay.h"
#include "ModeControl.h"
#include "SelfTest.h"
#include "OutputChannel.h"

//内部变量
static xdata int ErrDisplayIndex; //错误显示计时
static xdata char ShortDetectTIM=0; //短路监测计时器
static xdata char ShortBlankTIM; //短路blank定时器
static code FaultCodeDef NonCriticalFault[]={ //非致命的错误代码
	Fault_DCDCOpen,
  Fault_DCDCShort, //开路和短路可能是误报，允许消除
  Fault_InputOVP
	};
	
//外部全局参考
bit IsInputLimited;  //输入限流触发
xdata FaultCodeDef ErrCode; //错误代码	

//查询错误是否致命
bit IsErrorFatal(void)	
	{
	char i;
	for(i=0;i<sizeof(NonCriticalFault);i++)
		if(NonCriticalFault[i]==ErrCode)return 0;
	//寻找了目前已有的错误码发现是致命问题
	return 1;
	}

//报告错误
void ReportError(FaultCodeDef Code)
	{
	ErrCode=Code;
	if(CurrentMode->ModeIdx==Mode_Fault)return;
	SwitchToGear(Mode_Fault);  //指示故障发生
	}

//消除错误
void ClearError(void)
	{
	ErrCode=Fault_None;
	SwitchToGear(Mode_OFF);
	}

//错误ID显示计时函数	
void DisplayErrorTIMHandler(void)	
	{
	//没有错误发生，复位计时器
	if(ErrCode==Fault_None)ErrDisplayIndex=0;
	else //发生错误，开始计时
		{
		ErrDisplayIndex++;
    if(ErrDisplayIndex>=(5+(6*(int)ErrCode)+10))ErrDisplayIndex=0; //上限到了，开始翻转
		}
	}

//出现错误时显示DCDC的错误ID
void DisplayErrorIDHandler(void)
	{
	int buf;
	//先导提示红黄绿交替闪
  if(ErrDisplayIndex<5)
		{
		if(ErrDisplayIndex<3)LEDMode=(LEDStateDef)(ErrDisplayIndex+1);	
		else LEDMode=LED_OFF;
		}
	//闪烁指定次数显示Err ID
	else if(ErrDisplayIndex<(5+(6*(int)ErrCode)))
		{
		buf=(ErrDisplayIndex-5)/3; 
		if(!(buf%2))LEDMode=LED_Red;
		else LEDMode=LED_OFF;  //按照错误ID闪烁指定次数
		}
  else LEDMode=LED_OFF; //LED熄灭
	}
//内部函数，故障计数器
static char ErrTIMCounter(char buf,char Count)
	{
	//累加计数器
	return buf<8?buf+Count:8;
	}

//输出故障检测
void OutputFaultDetect(void)
	{
	char buf,OErrID;
	//输入MPPT限流监测
	if(Data.FBInjectVolt<0.2&&Data.RawBattVolt<12.0&&Data.OutputVoltage>16)IsInputLimited=1; //电池总电压低于12V，FB注入运放输出拉到负轨且输出大于16V，说明输入限流触发
	else IsInputLimited=0;
	//输出故障监测
	if(!GetIfOutputEnabled())ShortBlankTIM=0; //DCDC关闭
	else if(ShortBlankTIM<FaultBlankingInterval)ShortBlankTIM++; //时间未到不允许监测
	else  //开始检测
		{		
		buf=ShortDetectTIM&0x0F; //取出定时器值					
		//输入过压保护
		if(Data.BatteryVoltage>4.4)ReportError(Fault_InputOVP); 
		//短路检测	
		if(Data.OutputVoltage<14.6&&Data.FBInjectVolt>4.8) //输出短路
			{
			buf=ErrTIMCounter(buf,2); //计时器累计
			OErrID=0;
			}
		//输出开路检测
		else if(Data.FBInjectVolt<0.5&&Data.OutputVoltage>22.5) 
			{
			buf=ErrTIMCounter(buf,1); //计时器累计
			OErrID=1;
			}
		else buf=buf>0?buf-1:0; //没有发生错误，清除计数器
		//进行定时器数值的回写
		ShortDetectTIM=buf|(OErrID<<4);
		//状态检测
		if(buf<8)return; //没有故障,跳过执行
		switch((ShortDetectTIM>>4)&0x0F)	
			{
			case 1:ReportError(Fault_DCDCOpen);break;
			default:ReportError(Fault_DCDCShort);
			}
		}
	}