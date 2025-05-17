#include "IP2366_REG.h"
#include "lcd.h"
#include "GUI.h"
#include "Config.h"
#include "delay.h"
#include "ADC.h"
#include "BalanceMgmt.h"

//ȫ�ֱ���
static const char NonStdFwID[]={"YCNNH"};
static const char HSCPFwID[]={"YCEDS"};
bool IsBootFromVBUS;	
bool IsEnable17AMode=true;
bool IsEnableHSCPMode=false;
static char WaitAfterTypeCRemoved;	

void IP2366_PreInit(void)
	{
	char i=5;
	IP2366InputDef ICFG;
	IP2366OutConfigDef OCFG;
	ShowPostInfo(25,"���IC������","09",Msg_Statu);
	//���ýṹ��
	ICFG.ChargeCurrent=9700;
	ICFG.ChargePower=Power_100W;
	ICFG.FullVoltage=4200;
	ICFG.PreChargeCurrent=200;
	ICFG.IsEnableCharger=false;
		
	OCFG.IsEnableDPDMOut=false;
  OCFG.IsEnableOutput=false;
  OCFG.IsEnablePDOut=false;
  OCFG.IsEnableSCPOut=false;		
	//����Ƿ����
	do	
		{
		if(IP2366_DetectIfPresent())break;
		delay_ms(200);
		ShowPostInfo(25,"����������IC","0A",Msg_Warning);
		i--;
		}
	while(i>0);	
	//�����������Ȼʧ�ܣ���ʾ������
	if(i==0)	
		{
		ShowPostInfo(25,"���ICͨ��ʧ��","E3",Msg_Fault);
		SelfTestErrorHandler();
		}
	IP2366_SetInputState(&ICFG);
	//���Type-C״̬
  if(!IP2366_GetIfInputConnected())IP2366_SetOutputState(&OCFG); //�������		
	}
	
//ϵͳ���ȱ�������	
bool IsSystemOverheating=false;	
static bool IP2366DCDCState=false;
	
void SysOverHeatProt(void)
	{
	bool ChgEN,DisEN,IsStepDown;
	extern bool IsEnableAdapterEmu;
	TypeCRoleDef Role;
	//NTC�쳣
	if(!ADCO.IsNTCOK)return;
	//��������
	if(ADCO.Systemp>(float)CfgData.OverHeatLockTemp)IsSystemOverheating=true;
	else if(ADCO.Systemp<(float)CfgData.OverHeatLockTemp-10)IsSystemOverheating=false;
	//ͬ��DCDC״̬
  if(IP2366DCDCState==IsSystemOverheating)return;
	//����DCDCʹ��״̬
	if(BalanceForceEnableTIM>0)	
		{
		//�ֶ����������У��رճ�ŵ�
		ChgEN=false;
		DisEN=false;
		Role=TypeC_Disconnect;
		}
	else if(IsSystemOverheating)
		{
		//ϵͳ���ȣ��رճ�ŵ�
		ChgEN=false;
		DisEN=false;
		Role=TypeC_Disconnect;
		//�жϹ��ȱ����Ƿ���������
		if(!CfgData.EnableThermalStepdown)IsStepDown=false;
		else if(CfgData.InputConfig.ChargePower==Power_140W)IsStepDown=true;
		else if(CfgData.InputConfig.ChargePower==Power_100W)IsStepDown=true;	
		else IsStepDown=false;
		//�������ȱ���ǿ�аѹ��ʵ���ȥ
		if(IsStepDown)
			{
			//���е���
			if(CfgData.InputConfig.ChargePower==Power_140W)CfgData.InputConfig.ChargePower=Power_100W;
			else CfgData.InputConfig.ChargePower=Power_65W;
			WriteConfiguration(&CfgUnion,false);
			if(!IP2366_UpdataChargePower(CfgData.InputConfig.ChargePower))return;
			}
		}
	else if(IsEnableAdapterEmu)
		{
		//����������ģ�⣬�����ŵ�رճ��
		ChgEN=false;
		DisEN=true;
		Role=TypeC_UFP;
		}
	else
		{
		ChgEN=true;
		DisEN=CfgData.OutputConfig.IsEnableOutput;
		Role=CfgData.OutputConfig.IsEnableOutput?TypeC_DRP:TypeC_DFP;
		}		
	if(!IP2366_EnableDCDC(ChgEN,DisEN))return;
	if(!IP2366_SetTypeCRole(Role))return;
	IP2366DCDCState=IsSystemOverheating;
	}	
	
//����ϵͳ�������³�ʼ��2366
void IP2366_ReInitBasedOnConfig(void)
	{
	IP2366InputDef ICFG;
	IP2366OutConfigDef OCFG;
	extern bool IsEnableAdapterEmu;
	extern bool OCState;
	//���õ͵籣����PDO�������������
	OCFG.IsEnableDPDMOut=CfgData.OutputConfig.IsEnableDPDMOut;
	OCFG.IsEnablePDOut=CfgData.OutputConfig.IsEnablePDOut;
	OCFG.IsEnableSCPOut=CfgData.OutputConfig.IsEnableSCPOut;
	if(IsSystemOverheating)OCFG.IsEnableOutput=false;
	else if(IsEnableAdapterEmu)OCFG.IsEnableOutput=true;
	else OCFG.IsEnableOutput=CfgData.OutputConfig.IsEnableOutput;
			
	IP2366_SetVLowVolt(CfgData.Vlow);
	if(!IsBootFromVBUS&&!OCState)IP2366_SetOutputState(&OCFG);	//������ڹ���׶λ����ǵ�������Type-C���ǾͲ��������������Ȼ��Ƭ��ֱ�Ӷϵ���
	IP2366_SetPDOBroadCast(&CfgData.PDOCFG);
	//�����������
	ICFG.ChargePower=CfgData.InputConfig.ChargePower;	
	ICFG.FullVoltage=CfgData.InputConfig.FullVoltage;
	ICFG.PreChargeCurrent=CfgData.InputConfig.PreChargeCurrent; //���������ճ���д
	if(IsEnableAdapterEmu)ICFG.IsEnableCharger=false; //�رճ����
	else ICFG.IsEnableCharger=IsSystemOverheating?false:true; //�����������ȴ�����ر�
	ICFG.ChargeCurrent=CfgData.OutputConfig.IsEnableOutput?9700:CfgData.InputConfig.ChargeCurrent; //��ʼ��ʱ��������������д9.7A					
	IP2366_SetInputState(&ICFG);
	//����Type-Cģʽ
	if(!IsBootFromVBUS&&!OCState) //������ڹ���׶λ����ǵ�������Type-C���ǾͲ��ܷ���TCRST���Ȼ��Ƭ��ֱ�Ӷϵ���
		{
		if(!IsEnableAdapterEmu)
			{
			IP2366_SetTypeCRole(TypeC_Disconnect);
			delay_ms(200);
			IP2366_SetTypeCRole(CfgData.OutputConfig.IsEnableOutput?TypeC_DRP:TypeC_DFP);
			}
		else IP2366_SetTypeCRole(TypeC_UFP); //����������ģ������UFPģʽ
		delay_ms(100);
		}
	IP2366_SetOTPSign();
	IP2366_ClearOCFlag(); //�Ƴ�OC Flag
	}	

//�������ö�̬���������2366������
void IP2366_SetIBatLIMBaseOnSysCfg(void)
	{
	int Current;
	//���㶯̬����
	if(IP2366_GetIfInputConnected())Current=CfgData.InputConfig.ChargeCurrent; //���ģʽ�����ӣ�����Ϊ�������
	else Current=CfgData.InputConfig.ChargeCurrent>9700?CfgData.InputConfig.ChargeCurrent:9700; //Ϊ��ȷ���ŵ��������У������δ����ʱ����Ϊ9.7A����(������������������Ǿ�����Ϊ9700)
	//����ICCMAX�Ĵ���	
	IP2366_SetICCMax(Current); 
	}
	
//���Իָ�IP2366
void IP2366StallRestore(void);
extern short IPStallTime;	
	
void DetectIfIP2366Reset(void)
	{
	bool Reset;
	
	//��ȡ��ǰоƬͨ��״̬
	if(!IP2366_DetectIfChipReset(&Reset))IP2366StallRestore(); //��ʼ��ʱ
	//оƬ������λ�����üĴ���
	else if(Reset)IP2366_ReInitBasedOnConfig(); //���³�ʼ��
	else IPStallTime=0; //δ������λ
	//�������������Ӧ��
	if(!CfgData.OutputConfig.IsEnableOutput)return; //������رղ���Ҫ��̬��������
  IP2366_SetIBatLIMBaseOnSysCfg(); //��̬����
	}

//������ڵ�أ����ڰγ�TypeC֮���һ��ʱ�����³�ʼ��
void IP2366_ReConfigOutWhenTypeCOFF(void)	
	{
	if(!IsBootFromVBUS)return;
	if(WaitAfterTypeCRemoved>0)WaitAfterTypeCRemoved--;
	else
		{
		IsBootFromVBUS=false;
		IP2366_SetOutputState(&CfgData.OutputConfig);
		IP2366_SetTypeCRole(CfgData.OutputConfig.IsEnableOutput?TypeC_DRP:TypeC_DFP);
		IP2366_SetOTPSign();
		}
	}

//���ʼ��
void IP2366_PostInit(void)
	{
	bool IsCmdSendOK,IsPoweredByVBUS;		
	extern bool EnableDetailOutput;
	IP2366InputDef ICFG;	
	int retry=5,i;
	char VendorString[5];
	ShowPostInfo(78,"���IC������\0","10",Msg_Statu);
	//���ϵͳ�Ƿ���VBUS����
	for(i=0;i<15;i++)
		{
		ADC_GetResult();
		delay_ms(2);
		if(ADCO.Vbatt>(BattCellCount*2.6))break;
		}
	if(i<15)IsPoweredByVBUS=false;
	else  //��⵽����쳣����������	
    {		
		IsPoweredByVBUS=true;
		ShowPostInfo(78,"��ص�ѹ�쳣\0","EB",Msg_Warning);
		delay_Second(1);
		ShowPostInfo(78,"����Ƿ���ȷ��װ?","EB",Msg_Warning);
		delay_Second(1);
		}
	//���̼��汾
	ShowPostInfo(79,"��ȡоƬ�汾\0","11",Msg_Statu);
  if(!IP2366_GetFirmwareTimeStamp(VendorString))		
		{
		ShowPostInfo(79,"��ȡоƬ�汾ʧ��\0","DE",Msg_Fault);
		SelfTestErrorHandler();
		}
	IsEnable17AMode=true;
	for(i=0;i<5;i++)if(NonStdFwID[i]!=VendorString[i])
		{
		//ʱ�����ƥ�䣬��ֹҰ��ģʽ
		IsEnable17AMode=false;	
		break;
		}
	if(IsEnable17AMode)
		{
		ShowPostInfo(79,"����ģʽ������\0","6F",Msg_Statu);
		if(CfgData.IStop==IStop_100mA||CfgData.IStop==IStop_150mA)
			{
			ShowPostInfo(78,"ͣ������Ƿ�\0","D3",Msg_Warning);
			delay_Second(1);
			ShowPostInfo(78,"���Զ�����\0","D3",Msg_Warning);
			delay_Second(1);
			CfgData.IStop=IStop_200mA;
			if(!WriteConfiguration(&CfgUnion,true))
				{
				ShowPostInfo(30,"�洢��д���쳣\0","E6",Msg_Fault);
				SelfTestErrorHandler();
				}
			}
		if(EnableDetailOutput)delay_ms(300);
		}	
	if((CfgData.MaxVPD==PDMaxIN_28V||CfgData.InputConfig.ChargeCurrent>9700)&&!IsEnable17AMode)
		{
		ShowPostInfo(78,"оƬΪ����̼�\0","D0",Msg_Warning);
		delay_Second(1);
		ShowPostInfo(78,"����ģʽ�ѽ���\0","D0",Msg_Warning);
		delay_Second(1);
		//����ֵ��������ȥ
		CfgData.MaxVPD=PDMaxIN_20V;
		CfgData.InputConfig.ChargeCurrent=9700;
		if(!WriteConfiguration(&CfgUnion,true))
			{
			ShowPostInfo(80,"�洢��д���쳣\0","E6",Msg_Fault);
			SelfTestErrorHandler();
			}		
		}
	//����ѹSCP֧��
	IsEnableHSCPMode=true;
	for(i=0;i<5;i++)if(VendorString[i]!=HSCPFwID[i])
		{
		//ʱ�����ƥ�䣬�رո߹���SCP֧��
		IsEnableHSCPMode=false;
		break;
		}		
	if(!IsEnableHSCPMode&&CfgData.OutputConfig.IsEnableHSCPOut)
		{
		ShowPostInfo(78,"оƬΪ����̼�\0","D2",Msg_Warning);
		delay_Second(1);
		ShowPostInfo(78,"�߹���SCP�ѽ���\0","D2",Msg_Warning);
		delay_Second(1);
		//�رո߹���SCP����
		CfgData.OutputConfig.IsEnableHSCPOut=false;
		if(!WriteConfiguration(&CfgUnion,true))
			{
			ShowPostInfo(80,"�洢��д���쳣\0","E6",Msg_Fault);
			SelfTestErrorHandler();
			}		
		}
	//�����ٳ�����
	ShowPostInfo(80,"�����ٳ�����\0","12",Msg_Statu);	
	if(!IP2366_SetReChargeParam(CfgData.VRecharge,CfgData.IStop))
		{
		ShowPostInfo(80,"�ٳ��������ʧ��\0","EC",Msg_Fault);
		SelfTestErrorHandler();
		}
	//���õ�ѹ��������
	ShowPostInfo(82,"���õ�ѹ������ѹ\0","13",Msg_Statu);	
	if(!IP2366_SetVLowVolt(CfgData.Vlow))	
		{
		ShowPostInfo(82,"��ѹ��������ʧ��\0","E8",Msg_Fault);
		SelfTestErrorHandler();
		}
	//��ʼд�Ĵ���
	if(!IsPoweredByVBUS)
		{
		ShowPostInfo(85,"���÷ŵ�ϵͳ\0","14",Msg_Statu);	
		if(!IP2366_SetOutputState(&CfgData.OutputConfig))
			{
			ShowPostInfo(85,"�ŵ�ϵͳ����ʧ��\0","E6",Msg_Fault);
			SelfTestErrorHandler();
			}
		}
	//��������
	ShowPostInfo(88,"���ó��ϵͳ\0","15",Msg_Statu);
	ICFG.ChargePower=CfgData.InputConfig.ChargePower;	
	ICFG.FullVoltage=CfgData.InputConfig.FullVoltage;
	ICFG.PreChargeCurrent=CfgData.InputConfig.PreChargeCurrent; //���������ճ���д
	ICFG.IsEnableCharger=true; //�����ʼ�տ���
	ICFG.ChargeCurrent=CfgData.OutputConfig.IsEnableOutput?9700:CfgData.InputConfig.ChargeCurrent; //��ʼ��ʱ��������������д9.7A	
  //����PDO����
	if(!IP2366_SetInputState(&ICFG))
		{
		ShowPostInfo(88,"���ϵͳ����ʧ��\0","E7",Msg_Fault);
		SelfTestErrorHandler();
		}	
	//����PDO����
	ShowPostInfo(92,"����PDO�㲥\0","16",Msg_Statu);
	if(!IP2366_SetPDOBroadCast(&CfgData.PDOCFG))
		{
		ShowPostInfo(92,"PDO�㲥����ʧ��\0","EA",Msg_Fault);
		SelfTestErrorHandler();		
		}
	//�����������ֵ�ָ��
	ShowPostInfo(94,"����TypeC��������\0","17",Msg_Statu);
  if(!IsPoweredByVBUS)do
		{
		IsCmdSendOK=IP2366_SetTypeCRole(TypeC_Disconnect);
		delay_ms(200);
		IsCmdSendOK&=IP2366_SetTypeCRole(CfgData.OutputConfig.IsEnableOutput?TypeC_DRP:TypeC_DFP);
		delay_ms(200);
		IsCmdSendOK&=IP2366_SetOTPSign();
		if(IsCmdSendOK)break;
		retry--;
		ShowPostInfo(94,"��������������\0","16",Msg_Warning);
		}
	while(retry>0);
	//����ִֻ������OTP
	else do
		{
		IsCmdSendOK=IP2366_SetOTPSign();
		if(IsCmdSendOK)break;
		retry--;
		delay_ms(10);
		ShowPostInfo(94,"��������������\0","18",Msg_Warning);
		}
	while(retry>0);
	//����ʧ��
	if(!retry)
		{
		ShowPostInfo(94,"���������ʧ��\0","D3",Msg_Fault);
		SelfTestErrorHandler();
		}
	IP2366_ClearOCFlag(); //�Ƴ�OC Flag
	//������ϣ���ʱ����Ǵ�VBUS�����ģ�����ʾ���
	if(!IsPoweredByVBUS)return;
	IsBootFromVBUS=true;
	WaitAfterTypeCRemoved=50;
	}
