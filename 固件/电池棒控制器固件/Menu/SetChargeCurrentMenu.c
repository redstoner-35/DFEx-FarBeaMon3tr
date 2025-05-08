#include "GUI.h"
#include "Config.h"

//�ص����ò˵�
void ReturnFromIset(void)
	{
	SwitchingMenu(&ChgSysSetMenu);
	}
	
//���ò���
const intEditMenuCfg ChargeCurrentEdit=
	{
	&CfgData.InputConfig.ChargeCurrent, //����Դ
	3000,
	9700, //3000-9700mA(����оƬ)
	100, //LSB=100mA
	"mA", //����
	"����",
	"����",
  &ReturnFromIset,
	};
	
const intEditMenuCfg ChargeCurrentEditBeastMode=
	{
	&CfgData.InputConfig.ChargeCurrent, //����Դ
	3000,
	IP2366_ICCMAX, //3000оƬ���������ߵ���(�ǹ���оƬҰ��ģʽ)
	100, //LSB=100mA
	"mA", //����
	"����",
	"����",
  &ReturnFromIset,
	};	
	
	
//�����ֵ�Ƿ�Ϸ�
void CheckILimitIsOK(void)
	{
	extern bool IsEnable17AMode;
	if((!IsEnable17AMode||CfgData.MaxVPD==PDMaxIN_20V)&&CfgData.InputConfig.ChargeCurrent>9700)
		CfgData.InputConfig.ChargeCurrent=9700; //������Ӻ͹̼�������оƬ���ֻ�ܵ�9.7A��ֵ
	}	
	
//ռλ���������Զ�����Ⱦģʽ��CALL�����༭�˵�
void ISetMenuDummy(void)
	{
	extern bool IsEnable17AMode;
	if(!IsEnable17AMode||CfgData.MaxVPD==PDMaxIN_20V)IntEditHandler(&ChargeCurrentEdit);
	else IntEditHandler(&ChargeCurrentEditBeastMode);
	}
	
const MenuConfigDef IChargeSetMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&ISetMenuDummy, //��Ⱦ����
	NULL, //��������
	//�������ò˵�����Ҫ�ñ������
	"��ط�ֵ��������",
	NULL,
	NULL, 
	NULL,
	//�����ʱ���ʼ���˵��༭���˳���ʱ������ֵ
	&IntEditInitHandler,
	&CheckILimitIsOK
	};
