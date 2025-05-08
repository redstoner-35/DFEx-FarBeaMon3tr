#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"
#include "CapTest.h"
#include "Key.h"
#include "LogSystem.h"

bool IsResetRendered=false;
static bool IsColResetStart=false;
static bool ColResetResult=false;

typedef enum
	{
	Reset_SysConfig,
	Reset_CapTest,
	Reset_ColumbGauge
	}ResetDoneTypeDef;
	
//ѡ��λ�ĸ�
ResetDoneTypeDef RSTType;	

void DisplayResetOK(void)
	{
	if(IsResetRendered)return;
	RenderMenuBG(); //��ʾ����
	if(RSTType==Reset_ColumbGauge&&!IsColResetStart)	
		{
		ColResetResult=true;
		LCD_ShowChinese(28,23,"�������ÿ��ؼ�....",WHITE,LGRAY,0);//��������
		if(LogData.DischargeTime)ColResetResult=ResetRunTimeLogArea();; 
		IsColResetStart=true;
		return;
		}
	if(RSTType==Reset_ColumbGauge&&!ColResetResult&&IsColResetStart)
		{
		LCD_ShowChinese(28,23,"���ؼ�����ʧ��",RED,LGRAY,0);
		IsResetRendered=true;
		return;
		}
	switch(RSTType)
		{
		case Reset_SysConfig:LCD_ShowChinese(21,23,"�ѽ�ϵͳ���ûָ�Ϊ",WHITE,LGRAY,0);break;
		case Reset_CapTest:LCD_ShowChinese(21,23,"�ѽ��������ݻָ�Ϊ",WHITE,LGRAY,0);break;
		case Reset_ColumbGauge:LCD_ShowChinese(28,23,"�ѽ����ؼƻָ�Ϊ",WHITE,LGRAY,0);break;
		}
	LCD_ShowChinese(47,37,"����Ĭ��ֵ",WHITE,LGRAY,0);
	LCD_ShowChinese(32,61,"����",WHITE,LGRAY,0);
  LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
	LCD_ShowChinese(86,61,"���˳�",WHITE,LGRAY,0);
	//�ѻָ���������
	IsResetRendered=true;
	}

//�ص����ó������õ����˵�
void BackToResetMenu(void)
	{
	//û�а���
	if(KeyState.KeyEvent!=KeyEvent_ESC)return;	
	if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
	else SwitchingMenu(&RSTMainMenu); //�����˳�״̬,����ESC��ص����˵�	
	KeyState.KeyEvent=KeyEvent_None;
	IsResetRendered=false;
	}
//��λ���ؼ�
void ResetColumData(void)
	{
	RSTType=Reset_ColumbGauge;
	IsResetRendered=false;
	IsColResetStart=false;
	}

const MenuConfigDef ResetColMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
  &DisplayResetOK,
	&BackToResetMenu,	
	//�������ò˵�����Ҫ�ñ������
	"���ÿ��ؼ�����\0",
	NULL,
	NULL, 
	NULL,
	//������˳����캯��
	&ResetColumData,
	NULL
	};	
	
//��λ����ϵͳ
void SelectResetCTest(void)
	{
	RSTType=Reset_CapTest;
	ClearHistoryData();
	IsResetRendered=false;
	WriteCapData(&CTestData.ROMImage.Data,false);
	}
	
const MenuConfigDef ResetCTestMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
  &DisplayResetOK,
	&BackToResetMenu,	
	//�������ò˵�����Ҫ�ñ������
	"���ò���ϵͳ����\0",
	NULL,
	NULL, 
	NULL,
	//������˳����캯��
	&SelectResetCTest,
	NULL
	};

//��λϵͳ����	
void SelectResetConfig(void)
	{
	RSTType=Reset_SysConfig;
	RestoreDefaultConfig();
	IsResetRendered=false;
	WriteConfiguration(&CfgUnion,false);
	}	

const MenuConfigDef ResetSysConfigtMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
  &DisplayResetOK,
	&BackToResetMenu,	
	//�������ò˵�����Ҫ�ñ������
	"����ϵͳ����\0",
	NULL,
	NULL, 
	NULL,
	//������˳����캯��
	&SelectResetConfig,
	NULL
	};
