#include "GUI.h"
#include "Key.h"
#include "IP2366_REG.h"
#include "PCA9536.h"
#include "Config.h"
#include <string.h>

static bool ChipStateUpdated=false;
static char ChipInfoSelect=0;


//��ʾ�߾��ȹ��ʼ�״̬
static void ShowHPGaugeState(void)
	{
	extern bool IsEnableHPGauge;
	bool Result;
	LCD_ShowChinese(4,21,"�߾��ȹ��ʼ�",WHITE,LGRAY,0);
	if(!CfgData.EnableHPGauge)LCD_ShowChinese(131,21,"����",WHITE,LGRAY,0);
	else if(!IsEnableHPGauge)LCD_ShowChinese(131,21,"����",RED,LGRAY,0);
	else LCD_ShowChinese(131,21,"����",GREEN,LGRAY,0);	
	//��ʾ���������״̬
	Result=PCA9536_SetIOState(PCA9536_IOPIN_3,false);
	LCD_ShowChinese(4,35,"��������ģ��",WHITE,LGRAY,0);
	if(!Result)LCD_ShowChinese(131,35,"����",RED,LGRAY,0);
	else LCD_ShowChinese(131,35,"����",GREEN,LGRAY,0);		
	
	}

//��ʾоƬ�������
static void ShowChipChargeState(void)
	{
	bool Result;
	float Vstop;
  int Istop;		
	ChipStatDef State;
	//��ʾ�������	
	LCD_ShowChinese(4,21,"ͣ���ֹ����",WHITE,LGRAY,0);
	LCD_ShowChinese(4,35,"��ѹ����ѹ",WHITE,LGRAY,0);
	Result=IP2366_getCurrentChargeParam(&Istop,&Vstop);
	if(!Result)
		{
		LCD_ShowChinese(131,21,"δ֪",WHITE,LGRAY,0);
		LCD_ShowChinese(131,35,"δ֪",WHITE,LGRAY,0);
		}
	else
		{
		LCD_ShowIntNum(95,21,Istop,3,WHITE,LGRAY,12);
		LCD_ShowString(137,21,"mA",WHITE,LGRAY,12,0);
		LCD_ShowFloatNum1(95,35,Vstop,2,WHITE,LGRAY,12);
		LCD_ShowString(137,35," V",WHITE,LGRAY,12,0);
		}
	//��ʾType-C״̬
	Result=IP2366_ReadChipState(&State);
	LCD_ShowHybridString(4,49,"Type-C�˿�״̬",WHITE,LGRAY,0);
	if(!Result)LCD_ShowChinese(131,49,"δ֪",WHITE,LGRAY,0);
	else switch(State.VBusState)
		{
		case VBUS_NoPower:LCD_ShowChinese(127,49,"�µ�",WHITE,LGRAY,0);break;
		case VBUS_Normal:LCD_ShowChinese(127,49,"����",GREEN,LGRAY,0);break;
		case VBUS_OverVolt:LCD_ShowChinese(127,49,"��ѹ",RED,LGRAY,0);break;
		}	
	//��ʾ�ٳ��ѹ
	Result=IP2366_GetVRecharge(&Vstop);
	LCD_ShowHybridString(4,64,"�ٳ���ѹ",WHITE,LGRAY,0);
	if(!Result)LCD_ShowChinese(131,64,"δ֪",WHITE,LGRAY,0);	
	else if(Vstop==-1)LCD_ShowChinese(131,64,"����",WHITE,LGRAY,0);	
	else
		{
		LCD_ShowFloatNum1(95,64,Vstop,2,WHITE,LGRAY,12);
		LCD_ShowString(137,64," V",WHITE,LGRAY,12,0);
		}
	}

//��ʾоƬ������Ϣ
static void ShowChipBasicInfo(void)
	{
	extern bool IsEnable17AMode;
	bool Result;
	char ChipIDBuf[6];
	int ICCMax;
	ChipStatDef State;
	RecvPDODef PDOState;
	//��ȡоƬ�ַ���
	memset(ChipIDBuf,0,sizeof(ChipIDBuf));
	Result=IP2366_GetFirmwareTimeStamp(ChipIDBuf);
	LCD_ShowChinese(4,21,"�̼�ʱ���",WHITE,LGRAY,0);
	if(!Result)LCD_ShowString(130,21,"N/A",RED,LGRAY,12,0);
	else if(!IsEnableAdvancedMode)LCD_ShowString(118,21,"*****",WHITE,LGRAY,12,0);
  else LCD_ShowString(118,21,ChipIDBuf,IsEnable17AMode?CYAN:WHITE,LGRAY,12,0);
	//��ȡVBUS״̬
	LCD_ShowChinese(4,35,"��ֵ����",WHITE,LGRAY,0);
	Result=IP2366_GetCurrentPeakCurrent(&ICCMax);
	if(!Result)LCD_ShowChinese(131,35,"δ֪",WHITE,LGRAY,0);
	else
		{
		if(!IsEnableAdvancedMode)LCD_ShowString(95,35,"?????",WHITE,LGRAY,12,0);
		else LCD_ShowIntNum(95,35,ICCMax,5,WHITE,LGRAY,12);
		LCD_ShowString(137,35,"mA",WHITE,LGRAY,12,0);
		}
	//��ȡPDO״̬
	Result=IP2366_GetRecvPDO(&PDOState);
	LCD_ShowChinese(4,49,"��ǰ",WHITE,LGRAY,0);
	LCD_ShowString(30,49,"PDO",WHITE,LGRAY,12,0);
	LCD_ShowChinese(57,49,"�㲥����",WHITE,LGRAY,0);
  if(!Result)LCD_ShowChinese(131,49,"δ֪",WHITE,LGRAY,0);
	else switch(PDOState)
		{
		case RecvPDO_None:LCD_ShowString(130,49,"N/A",WHITE,LGRAY,12,0);break;
		case RecvPDO_5V:LCD_ShowString(130,49," 5V",CYAN,LGRAY,12,0);break;
		case RecvPDO_9V:LCD_ShowString(130,49," 9V",BLUE,LGRAY,12,0);break;
		case RecvPDO_12V:LCD_ShowString(130,49,"12V",GREEN,LGRAY,12,0);break;
		case RecvPDO_15V:LCD_ShowString(130,49,"15V",CYAN,LGRAY,12,0);break;
		case RecvPDO_20V:LCD_ShowString(130,49,"20V",CYAN,LGRAY,12,0);break;
		}
	//VSys״̬
	Result=IP2366_ReadChipState(&State);
	LCD_ShowChinese(4,64,"ϵͳ״̬",WHITE,LGRAY,0);
	if(!Result)LCD_ShowChinese(131,64,"δ֪",WHITE,LGRAY,0);
	else switch(State.VSysState)
		{
		case VSys_State_Normal:LCD_ShowChinese(131,64,"����",GREEN,LGRAY,0);break;
		case VSys_State_OCP:LCD_ShowChinese(131,64,"����",RED,LGRAY,0);break;
		case VSys_State_Short:LCD_ShowChinese(131,64,"��·",RED,LGRAY,0);break;
		}
	}

//��ʾоƬ��Ϣ
void ShowChipInfo(void)
	{
	//�Ѿ���Ⱦ����
	if(ChipStateUpdated)return;
  RenderMenuBG();
	//����index��Ⱦ��Ӧ�Ľ��
  switch(ChipInfoSelect)		
		{
		case 0:ShowChipBasicInfo();break;
		case 1:ShowChipChargeState();break;
		case 2:ShowHPGaugeState();break;
		default:
			ChipInfoSelect=0;
		  ChipStateUpdated=false;
		  return;  //�������ķǷ�״̬���˳�
		}
	//��Ⱦ��ϣ�ָʾ״̬
	ChipStateUpdated=true;
	}
	
void ShowChipKeyHandler(void)
	{
	//���¼���ҳ
	if(KeyState.KeyEvent==KeyEvent_Up&&ChipInfoSelect<2)
		{
		ChipInfoSelect++;
		ChipStateUpdated=false;
		}
	if(KeyState.KeyEvent==KeyEvent_Down&&ChipInfoSelect>0)	
		{
		ChipInfoSelect--;
		ChipStateUpdated=false;
		}		
	//����ESC�˳�
	if(KeyState.KeyEvent==KeyEvent_ESC)
		{
		if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
		else SwitchingMenu(&SetMainMenu); //�����˳�״̬,����ESC��ص����˵�
		}
	if(KeyState.KeyEvent==KeyEvent_Enter)ChipStateUpdated=false; //ˢ��״̬
	KeyState.KeyEvent=KeyEvent_None;
	}

//����ʱ���ò˵�flag�ĺ���
void ResetChipMenuState(void)
	{
	ChipStateUpdated=false;
	ChipInfoSelect=0;
	}

const MenuConfigDef ChipStatMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&ShowChipInfo, //��Ⱦ����
	&ShowChipKeyHandler, //��������
	//�������ò˵�����Ҫ�ñ������
	"�鿴оƬ״̬",
	NULL,
	NULL, 
	NULL,
	//������˳����캯��û������Ҫ��
	&ResetChipMenuState,
	NULL
	};	
