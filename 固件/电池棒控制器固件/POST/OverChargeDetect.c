#include "ADC.h"
#include "Config.h"
#include "IP2366_REG.h"
#include <math.h>

//全局变量
bool OCState=false;
static float IBattBuf=0;
static char OCDeAssertCounter=80;

//向下减少充电电压
static void DecreseVFull(void)
	{
	CfgData.InputConfig.FullVoltage-=20; //充电电压减少20mV
	if(CfgData.InputConfig.FullVoltage<4100)CfgData.InputConfig.FullVoltage=4100; //不允许一直往下调到低于4.1V
	WriteConfiguration(&CfgUnion,false);
	IP2366_UpdateFullVoltage(CfgData.InputConfig.FullVoltage); //更新充电电压
	}

//过充检测模块
void OverChargeDetectModule(void)
	{
	float IDiff;
	bool IsAssert,IsDeAssert;
	//计算电流差值	
	IDiff=fabsf(IBattBuf-ADCO.Ibatt);
	if(ADCO.Ibatt<0||IBattBuf<0)IDiff=0; //电流差为0
	//判断是否满足Asser条件
  if(OCState)IsAssert=false;	 //已经触发了
	else if(ADCO.Ibatt<-0.05)IsAssert=false; //放电阶段强制Deassert
	else if(ADCO.Vbatt>(4.215*BattCellCount))IsAssert=true;	 //电池电压超了，强制触发
	else if(IDiff>0.35&&ADCO.Ibatt<0.2)IsAssert=true; //满足电流瞬间跌到0且当前电流接近0才触发	
	else IsAssert=false;
	//触发操作处理
	if(IsAssert&&!OCState)
		{
		OCDeAssertCounter=80;
		OCState=true; //标记系统过冲
		if(ADCO.Vbatt>(4.215*BattCellCount))DecreseVFull(); //电池电压过高，减少充电电压
		}
	//判断是否满足DeAssert条件
	if(IsAssert||!OCState)IsDeAssert=false; //不满足退出条件
	else if(ADCO.Vbatt>(4.2075*BattCellCount))IsDeAssert=false;	 //电池电压超了
	else if(ADCO.Ibatt>0.3||ADCO.Ibatt<-0.05)IsDeAssert=true; //电池电流跑起来了或者处于过充状态
	else IsDeAssert=false;
	//退出操作处理
	if(IsDeAssert&&OCState)
		{
		if(OCDeAssertCounter>0)OCDeAssertCounter--; //时间没到继续清除状态
		else OCState=false; //时间到，清除状态	
		}
	else if(OCState&&OCDeAssertCounter<80)OCDeAssertCounter++; //如果当前处于过充保护状态但是退出条件不满足则复位计时器
	}
