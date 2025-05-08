#include "GUI.h"
#include "Config.h"
#include "ADC.h"

void EnteredInstantCapTest(void)
	{
	extern bool IsBootFromVBUS;
	//û������һ���Բ���
	if(CfgData.InstantCTest!=InstantCTest_Armed)return;
	//��ص�ѹ����13V����������
	if(ADCO.Vbatt>13.0)return; 
	//�����������ʱ�����������˵��û�г��׷ŵ����
	if(!IsBootFromVBUS)return;
	//��������
	if(CfgData.EnableAdvAccess)IsEnableAdvancedMode=true;  //����Ǹ߼�ģʽ��ʹ�ܸ�bit�������˳����ݻῨ����ͨ�˵�ȥ
	CfgData.InstantCTest=InstantCTest_EnteredOK; //��ǳɹ�����
	SwitchingMenu(&CapTestMenu); //ֱ�ӽ�����ݲ˵�
	}
//��ȡADC�����Ԥ����д�����Ϣ
void PushDefaultResultToVBat(void)
	{
	extern float VBat,IBat;
	//��ȡ���
	VBat=ADCO.Vbatt;
	IBat=ADCO.Ibatt;
	}
