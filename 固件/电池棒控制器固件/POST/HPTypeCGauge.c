#include "Config.h"
#include "INA226.h"
#include "lcd.h"
#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"
#include "delay.h"
#include <math.h>
#include <string.h>

//内部字符串
const char *A226ErrorStr[]=
{
"SMBUS_NACK",
"CalReg_OVF",
"Write_Calibration",
"Write_Config",	
"Write_AlertCfg",
"NotGenuineDevice"
};

//全局变量，是否开启高精度测量模块
bool IsEnableHPGauge=false;

//内部const
const char A226ERRORIDMSG[]={"错误ID:0x0"};

//报告功率计故障
static void ReportGaugeFailure(void)
	{
	ShowPostInfo(96,"功率计自检异常\0","3B",Msg_Warning);
	delay_Second(1);
	IsEnableHPGauge=false;
	}

//初始化高精度功率计
void HPPowerGuage_Start(void)
	{
	INAinitStrdef INAConf;
	INA226InitStatDef Result;
	INADoutSreDef TestResult;
	IP2366VBUSStateDef IP2366Result;
	bool SelfTestResult;
	float fbuf;
	int retry;
	char WakeMsg[sizeof(A226ERRORIDMSG)];
	//不启动
	if(!CfgData.EnableHPGauge)return;
	//准备配置INA226
	ShowPostInfo(95,"配置高精度功率计\0","38",Msg_Statu);
	INAConf.ConvMode=INA226_Cont_Both; //同时转换电压和电流，持续运行
	INAConf.IBUSConvTime=INA226_Conv_588US;
	INAConf.VBUSConvTime=INA226_Conv_588US;
	INAConf.AvgCount=INA226_AvgCount_128;    //设置平均次数=128，单次转换时间0.588mS，总更新时间=0.588*75.264mS，小于系统125mS的轮询间隔
	INAConf.IsAlertPinInverted=false;
	INAConf.IsEnableAlertLatch=false;
	INAConf.AlertConfig=A226_AlertDisable; //关闭所有警报，不使用警报相关功能
	INAConf.ShuntValue=CurrentIP2366FW->ShuntValue; //分流电阻阻值按照IP2366固件版本对应的电流值
	//进行配置
	Result=INA226_INIT(&INAConf);	
	if(Result!=A226_Init_OK)
		{
		ShowPostInfo(95,"功率计初始化失败\0","39",Msg_Warning);
		delay_Second(1);
		memcpy(WakeMsg,A226ERRORIDMSG,sizeof(WakeMsg));
		WakeMsg[9]='0'+(char)Result;
		ShowPostInfo(95,WakeMsg,"39",Msg_Warning);
		delay_Second(1);
		ShowPostInfo(95,(char *)A226ErrorStr[(char)Result-1],"39",Msg_Warning);	
		delay_Second(1);
		}			
	else IsEnableHPGauge=true;
	delay_ms(100);
	//进行一次测量尝试
	ShowPostInfo(96,"功率计自检...\0","3A",Msg_Statu);		
	SelfTestResult=INA226_SetAlertRegister(0);	
	//循环等待直到CVRF置起，表示可以读取结果
	retry=0;
	if(SelfTestResult)do
		{
		//CNVR置起，标记已经成功初始化
		if(INA226_QueueIfGaugeCanReady())break;
		//继续等待
		delay_ms(10);
		retry++;
		}
	while(retry<40);
	//启动超时，报错
  if(retry==40)
		{
		ReportGaugeFailure();
		return;
		}		
	//读取2366寄存器结果对获取到的电压电流进行比对
	SelfTestResult&=INA226_GetBusInformation(&TestResult);
	IP2366Result.VBUSCurrent=0;
	IP2366Result.VBUSVolt=0;
	retry=0;
	do
		{
		//获取IP2366的VBUS结果
		if(IP2366_GetVBUSState(&IP2366Result))break; 
		//获取失败，等待25mS再试
		delay_ms(25);
		retry++;
		}
	while(retry<200);
	//功率计自检超时，报错并退出
	if(retry==200)
		{
		ReportGaugeFailure();
		return;
		}
	fbuf=fabs(IP2366Result.VBUSVolt-TestResult.BusVolt);
	if(fbuf>0.3)SelfTestResult=false;       //和IP2366内的电压测量结果进行比对，如果电压误差大于0.3V，则说明电压采样系统异常，报错
	fbuf=fabs(IP2366Result.VBUSCurrent);
	fbuf-=fabs(TestResult.BusCurrent);
	if(fabs(fbuf)>0.5)SelfTestResult=false; //和IP2366内的电流测量结果进行比对，如果电流误差大于0.5A，则说明电流误差异常，报错
	//判断是否自检失败
	if(!SelfTestResult)ReportGaugeFailure();
  else ShowPostInfo(97,"功率计自检完毕\0","3C",Msg_Statu);
	}
