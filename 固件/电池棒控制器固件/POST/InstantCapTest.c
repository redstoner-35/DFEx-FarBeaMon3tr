#include "GUI.h"
#include "Config.h"
#include "ADC.h"

void EnteredInstantCapTest(void)
	{
	extern bool IsBootFromVBUS;
	//没有启动一次性测容
	if(CfgData.InstantCTest!=InstantCTest_Armed)return;
	//电池电压大于13V不允许启动
	if(ADCO.Vbatt>13.0)return; 
	//如果本次启动时保护板有输出说明没有彻底放电结束
	if(!IsBootFromVBUS)return;
	//启动测容
	if(CfgData.EnableAdvAccess)IsEnableAdvancedMode=true;  //如果是高级模式则使能该bit，否则退出测容会卡到普通菜单去
	CfgData.InstantCTest=InstantCTest_EnteredOK; //标记成功进入
	SwitchingMenu(&CapTestMenu); //直接进入测容菜单
	}
//读取ADC结果并预先填写电池信息
void PushDefaultResultToVBat(void)
	{
	extern float VBat,IBat;
	//读取结果
	VBat=ADCO.Vbatt;
	IBat=ADCO.Ibatt;
	}
