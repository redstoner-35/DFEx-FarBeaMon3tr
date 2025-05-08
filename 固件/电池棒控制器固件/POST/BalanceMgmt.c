#include "LCD_Init.h"
#include "Config.h"
#include "ht32.h"
#include "GUI.h"
#include "PCA9536.h"
#include "IP2366_REG.h"
#include "LogSystem.h"
#include "BalanceMgmt.h"
#include "ADC.h"
#include <math.h>

//ȫ�ֱ���
int BalanceForceEnableTIM=0; //ǿ�����þ���ϵͳ�ı��������ñ���д����0��ֵ���þ�����
bool EnableExtendedBal=false;  //��δ������ĵ�س�ŵ���Ŀ�ﵽ�㹻���ʱ����������
bool BalanceState=false;
int BalanceDisableTIM=0; //��ʱ�رվ���ļ�ʱ��
int ChargeFullTIM=0; //��������ʱ��

//�ڲ�ȫ��
static bool BalTypeCConnectedState; //��������״̬

//���þ��������
void BalanceMgmt_Init(void)
	{
	bool State;
	ShowPostInfo(52,"�������������","30",Msg_Statu);
	State=PCA9536_SetIOState(PCA9536_IOPIN_0,false); //����ӦIO����Ϊ0
	State&=PCA9536_SetIOPolarity(PCA9536_IOPIN_0,PCA9536_IO_Normal); //��������
	State&=PCA9536_SetIODirection(PCA9536_IOPIN_0,PCA9536_IODIR_OUT); //���ģʽ
	//�������״̬
	if(!State)
		{
		ShowPostInfo(52,"����������쳣","3E",Msg_Fault);
		SelfTestErrorHandler();
		}
	}
	
//ǿ�ƹرվ���
void Balance_ForceDiasble(void)
	{
	//����ӦIO����Ϊ0
	PCA9536_SetIOState(PCA9536_IOPIN_0,false); 
	}

//����Ƿ����ü�ǿ�����ģ��
static void Balance_ExtendBalMgmt(void)
	{
	bool State;
	float BalValue;
	extern bool EnableManuBal;
	//���TypeC״̬
	State=IP2366_GetIfInputConnected();
	if(BalTypeCConnectedState==State)return;
	BalTypeCConnectedState=State;
	//���������Ҫ���������״̬
	if(State||!EnableManuBal)return;
	if(CfgData.BalanceMode==Balance_Diasbled)BalValue=11.50;	
	else BalValue=16.50; //���ݾ�����ģʽѡ����Ҫ�Զ������ʱ��	
	if(LogData.UnbalanceBatteryAh<BalValue)return; //ѭ��������û��
	SwitchingMenu(&AutoBALMenu);	//�����Զ�����
	}	

//�ڼ�⵽ϵͳ��ʱ�䴦�ں�ѹ���״̬�޷�ת�̵�ʱ����ʱ�رվ���
static void Balance_ChargeDertect(void)
	{
	BatteryStateDef SysState=Batt_StandBy;
	IP2366_GetChargerState(&SysState); //��ȡ״̬
	//�����״̬������ؿ��ڳ���״̬��Ϊ��ѹ״̬ʱ����ʱ������
	if(BalanceDisableTIM>0&&SysState==Batt_ChgDone)BalanceDisableTIM=0; //����سɹ�ת�����֮���������¼��������
	if(BalanceDisableTIM>0)ChargeFullTIM=0;
	else if(fabsf(ADCO.Ibatt)<0.2&&SysState==Batt_CVCharge)ChargeFullTIM++;
	else if(ChargeFullTIM>0)ChargeFullTIM--;
	//��ʱ��ʱ�䵽3���ӣ����Թرվ�����оƬ���������ж�����
	if(ChargeFullTIM>=(8*60*2))
		{
		ChargeFullTIM=0;
		BalanceDisableTIM=(8*60*3); //ǿ�ƹرվ���2���ӣ���оƬ�����ж�����
		}
	}	
	
//���й����п��ƾ��������õ�ģ��
void Balance_IOMgmt(void)
	{
	bool IsBalanceEnable; 
	extern int SleepTimer;
	BatteryStateDef SysState=Batt_StandBy;
	//������ǿ�Զ������Լ���ʱ���þ�������оƬ�������ж�
	Balance_ChargeDertect();
	Balance_ExtendBalMgmt();
	//��ȡ���״̬
	IP2366_GetChargerState(&SysState); 
	//��ص�ѹ���ͻ��߼����������ߣ����þ���
	if(ADCO.Vbatt<10.1)IsBalanceEnable=false;
	//ǿ�����þ���ļ�ʱ��������еݼ�		
	else if(BalanceForceEnableTIM>0)
		{
		BalanceForceEnableTIM--;
		IsBalanceEnable=true;
		}		
	//ǿ�ƹرվ����ʱ�����������þ���
	else if(BalanceDisableTIM>0)
		{
		BalanceDisableTIM--;
		IsBalanceEnable=false;
		}
	//��������״̬��������
	else if(SysState!=Batt_StandBy||SleepTimer>8)switch(CfgData.BalanceMode)
		{
		case Balance_Diasbled:IsBalanceEnable=false;break; //���ùر���������
		case Balance_ChgOnly: //�����ʱ����
		  switch(SysState)
				{
				case Batt_PreChage:
				case Batt_CCCharge:
				case Batt_CVCharge:
				case Batt_ChgDone:IsBalanceEnable=true;break; //�����������״̬ʱ�����þ���ϵͳ
				default:IsBalanceEnable=false; //����رվ���ϵͳ
				}
			break;
		case Balance_ChgDisOnly: //����ŵ�ʱ����
		  if(SysState==Batt_ChgWait)IsBalanceEnable=false;
		  else if(SysState!=Batt_StandBy)IsBalanceEnable=true; //����������ŵ�״̬ʱ�����þ���ϵͳ
		  else IsBalanceEnable=false;
		  break;
		case Balance_AlwaysEnabled:IsBalanceEnable=true;break; //������Զ����
		}
	//ϵͳ��������˯�ߣ��رվ���
	else IsBalanceEnable=false;
	//����IO״̬
	if(BalanceState==IsBalanceEnable)return;
	if(PCA9536_SetIOState(PCA9536_IOPIN_0,IsBalanceEnable))BalanceState=IsBalanceEnable; //���þ���״̬
	}
