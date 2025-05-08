
#include "Config.h"
#include "Key.h"
#include "GUI.h"

static bool IsRenderSafeAlert=false;

//������������ģʽ
void DisplayAlertMsg(void)
	{
	if(IsRenderSafeAlert)return;
	RenderMenuBG();
	LCD_ShowChinese(8,20,"����ĳ��ϵͳ���ý���",YELLOW,LGRAY,0);
	LCD_ShowChinese(8,34,"����������ը����",YELLOW,LGRAY,0);
	LCD_ShowChinese(21,48,"�����������˺���",YELLOW,LGRAY,0);	
	LCD_ShowChinese(28,64,"��ȷ��Ҫ������",RED,LGRAY,0);	
	//��Ⱦ���
	IsRenderSafeAlert=true;
	}
	
void LeaveSafeAlmMenu(void)
	{
	if(KeyState.KeyEvent==KeyEvent_BothEnt)SwitchingMenu(&ChgSysSetMenu);
	else if(KeyState.KeyEvent!=KeyEvent_None)SwitchingMenu(&SetMainMenu); //������������ص����˵�
	KeyState.KeyEvent=KeyEvent_None;
	}	
	
void EnterSafeAlmMode(void)
	{
	IsRenderSafeAlert=false;
	}

const MenuConfigDef SafeAlmMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&DisplayAlertMsg, 
	&LeaveSafeAlmMenu, //��������
	//�������ò˵�����Ҫ�ñ������
	"��ȫ����",
	NULL,
	NULL, 
	NULL,
	//������˳����캯��
	&EnterSafeAlmMode, //����ʱ���úò���
	NULL 
	};
