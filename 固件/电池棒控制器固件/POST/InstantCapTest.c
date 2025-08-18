#include "GUI.h"
#include "Config.h"
#include "ADC.h"
#include "delay.h"
#include "INA226.h"
#include "IP2366_REG.h"
#include <math.h>

void EnteredInstantCapTest(void)
	{
	ShowPostInfo(97,"一次性测容初始化\0","40",Msg_Statu);		
	//没有启动一次性测容
	if(CfgData.InstantCTest!=InstantCTest_Armed)return;
	//存储模式激活会导致结果不准确，不允许启动	
	if(StorageMode!=StorageMode_OFF)return;
	//电池电压大于每节2.75V不允许启动
	if(ADCO.Vbatt>(2.75*BattCellCount))return; 
	//启动测容
	if(CfgData.EnableAdvAccess)IsEnableAdvancedMode=true;  //如果是高级模式则使能该bit，否则退出测容会卡到普通菜单去
	CfgData.InstantCTest=InstantCTest_EnteredOK; //标记成功进入
	SwitchingMenu(&CapTestMenu); //直接进入测容菜单
	ShowPostInfo(97,"一次性测容已激活\0","41",Msg_INFO);	
	delay_ms(300);
	ShowPostInfo(97,"将开始测容流程\0","41",Msg_INFO);	
	delay_ms(300);
	}
//读取ADC结果并预先填写电池和C口信息
void PushDefaultResultToVBat(void)
	{
	extern float VBat,IBat;
	extern bool IsEnableHPGauge;
	bool result;
	INADoutSreDef VBUSData;
	BatteryStateDef BATTState;
	IP2366VBUSStateDef VBUSState;
	//读取ADC结果赋初值
	ShowPostInfo(98,"测量系统初始化\0","43",Msg_Statu);
	VBat=ADCO.Vbatt;
	IBat=ADCO.Ibatt;
	//IP2366初始化处理
	result=IP2366_GetChargerState(&BATTState);	
	result&=IP2366_GetVBUSState(&VBUSState);	
	if(!result)
		{
		ShowPostInfo(98,"测量系统初始化失败\0","FA",Msg_Fault);
		SelfTestErrorHandler();
		}
	//开始进行INA226的测量	
	if(IsEnableHPGauge)
		{
		//尝试读取INA226进行高精度测量
		if(!INA226_GetBusInformation(&VBUSData))
			{
			//高精度测量失效，回到传统模式
			IsEnableHPGauge=false;
			ShowPostInfo(98,"C口功率计测量失败\0","44",Msg_Warning);
			delay_Second(1);
			ShowPostInfo(98,"将使用PMIC数据\0","44",Msg_Warning);
			delay_Second(1);
			}
		//高精度测量正常完成，直接使用INA226读回来的数据	
		else
			{		
			//对电流数据进行替换
			VBUSState.VBUSCurrent=fabsf(VBUSData.BusCurrent); 
			if(BATTState==Batt_discharging)VBUSState.VBUSCurrent*=-1;	//电池处于放电状态，电流*-1表示正在放电		
			//对电压数据进行替换
			VBUSState.VBUSVolt=(VBUSState.VBUSCurrent*0.0027)+VBUSData.BusVolt; //补偿路径开关管的Rdson（实物中使用BSZ018N04，5V下加上Rdson和Rpcv大概是2.7mR）计算得出Typec位置的电压
			}
		}	
	if(VBUSState.VBUSVolt<VBat)VBat+=0.033; //当前Type-C没有连接，系统靠电池供电需要补偿的VBat保险电阻的压降（33mA@1Ω=0.033V）	
	}
