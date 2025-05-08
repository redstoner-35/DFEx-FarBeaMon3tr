#include "GUI.h"
#include "IP2366_REG.h"
#include "Config.h"
#include "Key.h"

typedef enum
	{
	TCResetFSM_Wait,
	TCResetFSM_Break,
	TCResetFSM_WaitForSomeTime,
	TCResetFSM_MakeConnect,
	TCResetFSM_Success,
	TCResetFSM_Failed,
	}TCFSMStateDef;

//�ڲ�����
TCFSMStateDef TCFSMState=TCResetFSM_Wait; //TypeC���״̬��
static char TCFSMWait=0;
static bool IsTCResetUpdated=false;
	
//TypeC��������״̬��
void TCResetFSM(void)
	{
	TypeCRoleDef Role;
	extern bool IsEnableAdapterEmu;
	extern bool IsSystemOverheating;
	//״̬��	
	switch(TCFSMState)
		{
		case TCResetFSM_Wait:break;
	  case TCResetFSM_Break: //�������
			 if(!IP2366_SetTypeCRole(TypeC_Disconnect))
				 {
				 TCFSMState=TCResetFSM_Failed;
				 IsTCResetUpdated=false;
				 }
	     else
					{
				  TCFSMWait=50;
				  TCFSMState=TCResetFSM_WaitForSomeTime;
					}
		   break;
		case TCResetFSM_WaitForSomeTime: //��һ��
		   if(TCFSMWait>0)TCFSMWait--;
		   else TCFSMState=TCResetFSM_MakeConnect; //׼������
		   break;
		case TCResetFSM_MakeConnect:
			//����ϵͳ״̬ȷ����ɫ
		  IP2366_ClearOCFlag();
			if(IsSystemOverheating)Role=TypeC_Disconnect;	//ϵͳ���ȣ��رճ�ŵ�
			else if(IsEnableAdapterEmu)Role=TypeC_UFP;	//����������ģ�⣬�����ŵ�رճ��
			else Role=CfgData.OutputConfig.IsEnableOutput?TypeC_DRP:TypeC_DFP;
		  //����ȷ���Ľ�ɫ����Type-Cģʽ
		  if(!IP2366_SetTypeCRole(Role))
					TCFSMState=TCResetFSM_Failed;
	     else
		      TCFSMState=TCResetFSM_Success;
			 IsTCResetUpdated=false;
			 break;
		//����״̬
		case TCResetFSM_Success:
		case TCResetFSM_Failed:break;
		}
	}
	
//TypeC������Ⱦ����
void TypeCResetRender(void)
	{
	if(IsTCResetUpdated)return;
	RenderMenuBG();
	switch(TCFSMState)
		{
		case TCResetFSM_Success:
			LCD_ShowString(24,26,"Type-C",GREEN,LGRAY,12,0);
			LCD_ShowChinese(67,26,"��·������",GREEN,LGRAY,0);  
			LCD_ShowChinese(32,61,"����",WHITE,LGRAY,0);
			LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
			LCD_ShowChinese(86,61,"���˳�",WHITE,LGRAY,0);
		  break;
		case TCResetFSM_Failed:
			LCD_ShowString(19,26,"Type-C",RED,LGRAY,12,0);
			LCD_ShowChinese(63,26,"��·����ʧ��",RED,LGRAY,0);  
			LCD_ShowChinese(32,61,"����",WHITE,LGRAY,0);
			LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
			LCD_ShowChinese(86,61,"���˳�",WHITE,LGRAY,0);		
			break;
		default:
			LCD_ShowChinese(18,32,"��������",WHITE,LGRAY,0);
		  LCD_ShowString(70,32,"Type-C",WHITE,LGRAY,12,0);
		  LCD_ShowChinese(114,32,"��·",WHITE,LGRAY,0);
			LCD_ShowChinese(46,47,"���Ժ󡭡�",WHITE,LGRAY,0);
		  break;
		}
	//��Ⱦ���
	IsTCResetUpdated=true;
	}
	
void TypeCResetKeyHandler(void)
	{
	if(KeyState.KeyEvent!=KeyEvent_ESC)return;
	if(TCFSMState==TCResetFSM_Success||TCFSMState==TCResetFSM_Failed)
		{
		TCFSMState=TCResetFSM_Wait; //�ص��ȴ�״̬
		if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
		else SwitchingMenu(&SetMainMenu); //�����˳�״̬,����ESC��ص����˵�
		}
	KeyState.KeyEvent=KeyEvent_None;
	}	
	
void EnterTCResetMenu(void)
	{
	TCFSMWait=0;
	IsTCResetUpdated=false;
	TCFSMState=TCResetFSM_Break;
	}
	
const MenuConfigDef TCResetMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&TypeCResetRender, 
	&TypeCResetKeyHandler, //��������
	//�������ò˵�����Ҫ�ñ������
	"Type-C��·����",
	NULL,
	NULL, 
	NULL,
	//������˳����캯��
	&EnterTCResetMenu, //����ʱ���úò���
	NULL 
	};
