#include "Config.h"
#include "INA226.h"
#include "lcd.h"
#include "GUI.h"
#include "Config.h"
#include "delay.h"

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

void HPPowerGuage_Start(void)
	{
	INAinitStrdef INAConf;
	extern bool IsEnable17AMode;
	INA226InitStatDef Result;
	INADoutSreDef TestResult;
	bool SelfTestResult;
	int retry=0;
	char WakeMsg[]={"错误ID:0x0"};
	//不启动
	if(!CfgData.EnableHPGauge)return;
	//准备配置INA226
	ShowPostInfo(95,"配置高精度功率计\0","73",Msg_Statu);
	INAConf.ConvMode=INA226_Cont_Both; //同时转换电压和电流，持续运行
	INAConf.IBUSConvTime=INA226_Conv_588US;
	INAConf.VBUSConvTime=INA226_Conv_588US;
	INAConf.AvgCount=INA226_AvgCount_128;    //设置平均次数=128，单次转换时间0.588mS，总更新时间=0.588*75.264mS，小于系统125mS的轮询间隔
	INAConf.IsAlertPinInverted=false;
	INAConf.IsEnableAlertLatch=false;
	INAConf.AlertConfig=A226_AlertDisable; //关闭所有警报，不使用警报相关功能
	INAConf.ShuntValue=IsEnable17AMode?2.50:5.00; //分流电阻阻值按照默认值配置
	//进行配置
	Result=INA226_INIT(&INAConf);	
	if(Result!=A226_Init_OK)
		{
		ShowPostInfo(95,"功率计初始化失败\0","7A",Msg_Warning);
		delay_Second(1);
		WakeMsg[9]='0'+(char)Result;
		ShowPostInfo(95,WakeMsg,"78",Msg_Warning);
		delay_Second(1);
		ShowPostInfo(95,A226ErrorStr[(char)Result-1],"78",Msg_Warning);	
		delay_Second(1);
		}			
	else IsEnableHPGauge=true;
	delay_ms(100);
	//进行一次测量尝试
	ShowPostInfo(97,"功率计自检...\0","74",Msg_Statu);		
	SelfTestResult=INA226_SetAlertRegister(0);	
	//循环等待直到CVRF置起，表示可以读取结果
	if(SelfTestResult)do
		{
		//CNVR置起，标记已经成功初始化
		if(INA226_QueueIfGaugeCanReady())break;
		//继续等待
		delay_ms(10);
		retry++;
		}
	while(retry<40);
	//设置警报寄存器失败，直接报错
	else retry=40;
	//判断是否自检失败
	if(retry==40||!INA226_GetBusInformation(&TestResult))
		{
		ShowPostInfo(95,"功率计自检异常\0","7A",Msg_Warning);
		delay_Second(1);
		IsEnableHPGauge=false;
		}
	}
