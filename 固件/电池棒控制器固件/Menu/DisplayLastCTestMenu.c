#include "GUI.h"
#include "CapTest.h"
#include <math.h>
#include "Key.h"
#include "Config.h"

//�ڲ�����
static bool ShowMenuState;
static bool IsUpdateCUI=false;

//��ʾGUI�°벿��
void ShowLowerPart(void)
	{
	u16 Color;
	float Temp;
	//��ʾ�¶�
	LCD_ShowChinese(3,21,"����ʱ����¶�",WHITE,LGRAY,0);
	if(LastCData.MaxChargeTemp==-100)LCD_ShowChinese(103,21,"������",RED,LGRAY,0);
	else
		{
		Temp=LastCData.MaxChargeTemp;
		if(Temp<0)Color=DARKBLUE;	
		else if(Temp<10)Color=BLUE;
		else if(Temp<CfgData.OverHeatLockTemp-20)Color=GREEN;
		else if(Temp<CfgData.OverHeatLockTemp-8)Color=YELLOW;
		else Color=RED;
		//�����¶�
		if(Temp<0)
			{
			Temp*=-1; //ת����
			LCD_ShowChar(98,21,'-',Color,LGRAY,12,0);
			if(Temp<10)LCD_ShowFloatNum1(107,21,Temp,2,Color,LGRAY,12); //9.99��ʾ
			else LCD_ShowFloatNum1(107,21,Temp,1,Color,LGRAY,12); //99.9��ʾ
			}
		//�����¶�
		else if(Temp<10)LCD_ShowFloatNum1(98,21,Temp,3,Color,LGRAY,12); //9.999��ʾ
		else if(Temp<100)LCD_ShowFloatNum1(98,21,Temp,2,Color,LGRAY,12); //99.99��ʾ
		else LCD_ShowFloatNum1(98,21,Temp,1,Color,LGRAY,12); //999.9��ʾ
		//��ʾ���϶ȷ���
		LCD_ShowChinese12x12(143,21,"��",WHITE,LGRAY,12,0); //��ʾ����
		}
	//��ʾ�����
	Temp=LastCData.MaxChargeRatio;
	if(Temp<1.0)Color=GREEN;
  else if(Temp<2.0)Color=YELLOW;
  else Color=RED;  //���ݱ�����ʾ��ɫ
	LCD_ShowChinese(3,35,"����籶��",WHITE,LGRAY,0);
	if(LastCData.MaxChargeRatio<10)LCD_ShowFloatNum1(98,35,LastCData.MaxChargeRatio,3,Color,LGRAY,12); //9.999��ʾ
	else if(LastCData.MaxChargeRatio<100)LCD_ShowFloatNum1(98,35,LastCData.MaxChargeRatio,2,Color,LGRAY,12); //99.99��ʾ
	else LCD_ShowFloatNum1(98,35,LastCData.MaxChargeRatio,1,Color,LGRAY,12); //99.99��ʾ
	LCD_ShowChar(147,35,'C',WHITE,LGRAY,12,0);
	//��ʾ���ݿ�ʼ��ѹ
	LCD_ShowChinese(3,49,"���ݿ�ʼ��ѹ",WHITE,LGRAY,0);
	LCD_ShowFloatNum1(98,49,LastCData.StartVbatt,2,WHITE,LGRAY,12);
	LCD_ShowChar(147,49,'V',WHITE,LGRAY,12,0);
	//�����ߵ�ѹ
	Temp=LastCData.MaxVbatt/4;
	if(Temp<4.21)Color=GREEN;
  else if(Temp<4.25)Color=YELLOW;
  else Color=RED;  //���ݽ�ֹ��ѹ״̬��ʾ״̬
	LCD_ShowChinese(3,64,"�����ߵ�ѹ",WHITE,LGRAY,0);
	LCD_ShowFloatNum1(98,64,LastCData.MaxVbatt,2,Color,LGRAY,12);
	LCD_ShowChar(147,64,'V',WHITE,LGRAY,12,0);	
	}

//��ʾ�˵����ϰ벿��
void ShowUpperPart(void)
	{
	float buf;
	//��ʾʱ��
	LCD_ShowChinese(3,21,"���ʱ��",WHITE,LGRAY,0);
	ShowTimeCode(21,LastCData.ChargeTime);
	//��ʾ�������
  LCD_ShowChinese(3,35,"��������",WHITE,LGRAY,0);		
	buf=LastCData.TotalWh;
	if(buf<10)LCD_ShowFloatNum1(87,35,buf,3,WHITE,LGRAY,12); //9.999��ʾ
	else if(buf<100)LCD_ShowFloatNum1(87,35,buf,2,WHITE,LGRAY,12); //99.99��ʾ	
	else if(buf<1000)LCD_ShowFloatNum1(87,35,buf,1,WHITE,LGRAY,12); //999.9��ʾ
	else LCD_ShowIntNum(87,35,iroundf(buf),5,WHITE,LGRAY,12); //9999��ʾ
	LCD_ShowString(138,35,"Wh",WHITE,LGRAY,12,0);
	//��ʾ����Ah��
	LCD_ShowChinese(3,49,"��������",WHITE,LGRAY,0);		
	buf=LastCData.TotalmAH;
	if(buf<10000) //С��10Ahʹ��mAH��ʾ
		{
		LCD_ShowIntNum(87,49,iroundf(buf),4,WHITE,LGRAY,12);
		LCD_ShowString(129,49,"mAh",WHITE,LGRAY,12,0);
		}
	else //ʹ�ø�����ʾ
		{
	  buf/=(float)1000;			
		if(buf<100)LCD_ShowFloatNum1(87,49,buf,2,WHITE,LGRAY,12);  //99.99��ʾ
		else if(buf<100)LCD_ShowFloatNum1(87,49,buf,1,WHITE,LGRAY,12);  //999.9��ʾ
		else LCD_ShowIntNum(87,49,iroundf(buf),5,WHITE,LGRAY,12); //9999��ʾ
		LCD_ShowString(138,49,"Ah",WHITE,LGRAY,12,0);
		}
	//��ʾ��߳�����
	LCD_ShowChinese(3,64,"��߳�����",WHITE,LGRAY,0);		
	LCD_ShowFloatNum1(87,64,LastCData.MaxChargeCurrent,2,WHITE,LGRAY,12);
	LCD_ShowChar(147,64,'A',WHITE,LGRAY,12,0);	
	}
//������ʾ�İ�������
void CapHisKeyHandler(void)
	{
	//���·�ҳ
	if(KeyState.KeyEvent==KeyEvent_Down)ShowMenuState=true;
	if(KeyState.KeyEvent==KeyEvent_Up)ShowMenuState=false;
	//�˳�
	if(KeyState.KeyEvent==KeyEvent_ESC)
		{
		if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
		else SwitchingMenu(&SetMainMenu); //�����˳�״̬,����ESC��ص����˵�
		}
	if(KeyState.KeyEvent!=KeyEvent_None)
		{
		IsUpdateCUI=true;
		KeyState.KeyEvent=KeyEvent_None;
		}
  else IsUpdateCUI=false;
	}

//��ʾGUI
void ShowCapHisGUI(void)
	{
	if(!IsUpdateCUI)return;
	RenderMenuBG();
	if(!ShowMenuState)ShowUpperPart();
	else ShowLowerPart();
	IsUpdateCUI=false;
	}
	
void ResetHisMenuToUpper(void)
	{
	//ÿ�ν���ʱ��������
	IsUpdateCUI=true;
	ShowMenuState=false;
	}	
	
const MenuConfigDef CapTestHisMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&ShowCapHisGUI, //��Ⱦ����
	&CapHisKeyHandler, //��������
	//�������ò˵�����Ҫ�ñ������
	"�鿴��ʷ��������",
	NULL,
	NULL, 
	NULL,
	//������˳����캯��û������Ҫ��
	&ResetHisMenuToUpper,
	NULL
	};
