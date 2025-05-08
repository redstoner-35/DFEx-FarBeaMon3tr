#include "Config.h"
#include "Key.h"
#include "GUI.h"

static bool IsRenderICTestMode=false;

//������������ģʽ
void DisplaySuccEnteredCapStart(void)
	{
	if(IsRenderICTestMode)return;
	RenderMenuBG();
	LCD_ShowChinese(14,21,"�Ѽ���һ���Բ���ģʽ",GREEN,LGRAY,0);
	LCD_ShowChinese(7,36,"�´�����ʱ������������",WHITE,LGRAY,0);
	LCD_ShowChinese(7,49,"��ϵͳ�����Զ���ʼ����",WHITE,LGRAY,0);
	//ָʾ����ʲô��
	LCD_ShowChinese(32,64,"����",WHITE,LGRAY,0);
	LCD_ShowString(59,64,"ESC",YELLOW,LGRAY,12,0);
	LCD_ShowChinese(86,64,"���˳�",WHITE,LGRAY,0);		
	//��Ⱦ���
	IsRenderICTestMode=true;
	}
	
void LeaveICTMenu(void)
	{
	if(KeyState.KeyEvent==KeyEvent_ESC)
		{
		if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
		else SwitchingMenu(&SetMainMenu); //�����˳�״̬,����ESC��ص����˵�
		}	
	KeyState.KeyEvent=KeyEvent_None;
	}	
	
void EnableICTestMode(void)
	{
	CfgData.InstantCTest=InstantCTest_Armed;
	WriteConfiguration(&CfgUnion,false);
	IsRenderICTestMode=false;
	}

const MenuConfigDef ActOneShotCTestMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&DisplaySuccEnteredCapStart, 
	&LeaveICTMenu, //��������
	//�������ò˵�����Ҫ�ñ������
	"����һ���Բ���",
	NULL,
	NULL, 
	NULL,
	//������˳����캯��
	&EnableICTestMode, //����ʱ���úò���
	NULL 
	};
