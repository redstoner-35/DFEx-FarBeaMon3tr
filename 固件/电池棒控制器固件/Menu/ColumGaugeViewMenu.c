#include "GUI.h"
#include "LogSystem.h"
#include "Key.h"

//�ڲ�����
static char ShowCMenuState=0;
bool IsUpdateCDUI=false;

static void DisplayCapacity(float IN,u16 y)
	{
	if(IN<10)LCD_ShowFloatNum1(87,y,IN,3,WHITE,LGRAY,12); //9.999��ʾ
	else if(IN<100)LCD_ShowFloatNum1(87,y,IN,2,WHITE,LGRAY,12); //99.99��ʾ	
	else if(IN<1000)LCD_ShowFloatNum1(87,y,IN,1,WHITE,LGRAY,12); //999.9��ʾ
	else if(IN<10000)LCD_ShowIntNum(87,y,iroundf(IN),5,WHITE,LGRAY,12); //9999��ʾ
	else	
	  {
		IN/=1000;
	  if(IN<10)LCD_ShowFloatNum1(87,y,IN,2,WHITE,LGRAY,12); //9.99��ʾ
		else if(IN<100)LCD_ShowFloatNum1(87,y,IN,1,WHITE,LGRAY,12); //99.9��ʾ	
		else if(IN<1000)LCD_ShowIntNum(87,y,iroundf(IN),3,WHITE,LGRAY,12); //999��ʾ
		}
	if(IN<10000)LCD_ShowString(138,y,"Wh",WHITE,LGRAY,12,0);
	else LCD_ShowString(129,y,"KWh",WHITE,LGRAY,12,0);
	}
	
static void DisplayAh(float IN,u16 y)
	{
	float buf;
  //ʹ��mAH��ʾ
	if(IN<10000) //С��10Ahʹ��mAH��ʾ
		{
		LCD_ShowIntNum(87,y,iroundf(IN),4,WHITE,LGRAY,12);
		LCD_ShowString(129,y,"mAh",WHITE,LGRAY,12,0);
		}
	else if(IN<10000*1000)//ʹ�ø�����ʾ
		{
	  buf/=(float)1000;			
		if(buf<100)LCD_ShowFloatNum1(87,y,buf,2,WHITE,LGRAY,12);  //99.99��ʾ
		else if(buf<100)LCD_ShowFloatNum1(87,y,buf,1,WHITE,LGRAY,12);  //999.9��ʾ
		else LCD_ShowIntNum(87,y,iroundf(buf),5,WHITE,LGRAY,12); //9999��ʾ
		LCD_ShowString(138,y,"Ah",WHITE,LGRAY,12,0);
		}
	else //ʹ��KAH��ʾ
		{
		buf/=(float)1000*1000;	
    if(buf<10)LCD_ShowFloatNum1(87,y,buf,3,WHITE,LGRAY,12);  //9.999��ʾ
		else if(buf<100)LCD_ShowFloatNum1(87,y,buf,2,WHITE,LGRAY,12);  //99.99��ʾ
		else if(buf<1000)LCD_ShowFloatNum1(87,y,buf,1,WHITE,LGRAY,12);  //999.9��ʾ
		else LCD_ShowIntNum(87,y,iroundf(buf),5,WHITE,LGRAY,12); //9999��ʾ
		LCD_ShowString(129,y,"KAh",WHITE,LGRAY,12,0);
		}
	}

static void ShowColumbDataUpper(void)
	{
	//���ʱ��
	LCD_ShowChinese(3,21,"���ʱ��",WHITE,LGRAY,0);
	ShowTimeCode(21,LogData.ChargeTime);
	//��������
	LCD_ShowChinese(3,35,"��������",WHITE,LGRAY,0);
	DisplayCapacity(LogData.TotalChargeWh,35);
  //��������
	LCD_ShowChinese(3,49,"��������",WHITE,LGRAY,0);
	DisplayAh(LogData.TotalChargeAh*1000,49);
	//��ض�������
  LCD_ShowChinese(3,64,"��ߵ�ص���",WHITE,LGRAY,0);		
	LCD_ShowFloatNum1(87,64,LogData.MaximumBattCurrent,2,WHITE,LGRAY,12);
	LCD_ShowChar(147,64,'A',WHITE,LGRAY,12,0);			
	}
	
static void ShowColumbDataLower(void)
	{
	u16 Color;
	float Temp;
	//���ʱ��
	LCD_ShowChinese(3,21,"�ŵ�ʱ��",WHITE,LGRAY,0);
	ShowTimeCode(21,LogData.DischargeTime);
	//��������
	LCD_ShowChinese(3,35,"�ų�����",WHITE,LGRAY,0);
	DisplayCapacity(LogData.TotalDischargeWh,35);
  //��������
	LCD_ShowChinese(3,49,"�ų�����",WHITE,LGRAY,0);
	DisplayAh(LogData.TotalDischargeAh*1000,49);
	//��ʾ�¶�
	LCD_ShowChinese(3,64,"ϵͳ����¶�",WHITE,LGRAY,0);
	if(LogData.SysMaxTemp==-100)LCD_ShowChinese(103,64,"������",RED,LGRAY,0);
	else
		{
		Temp=LogData.SysMaxTemp;
		if(Temp<0)Color=DARKBLUE;	
		else if(Temp<10)Color=BLUE;
		else if(Temp<CfgData.OverHeatLockTemp-20)Color=GREEN;
		else if(Temp<CfgData.OverHeatLockTemp-8)Color=YELLOW;
		else Color=RED;
		//�����¶�
		if(Temp<0)
			{
			Temp*=-1; //ת����
			LCD_ShowChar(98,64,'-',Color,LGRAY,12,0);
			if(Temp<10)LCD_ShowFloatNum1(107,64,Temp,2,Color,LGRAY,12); //9.99��ʾ
			else LCD_ShowFloatNum1(107,64,Temp,1,Color,LGRAY,12); //99.9��ʾ
			}
		//�����¶�
		else if(Temp<10)LCD_ShowFloatNum1(98,64,Temp,3,Color,LGRAY,12); //9.999��ʾ
		else if(Temp<100)LCD_ShowFloatNum1(98,64,Temp,2,Color,LGRAY,12); //99.99��ʾ
		else LCD_ShowFloatNum1(98,64,Temp,1,Color,LGRAY,12); //999.9��ʾ
		//��ʾ���϶ȷ���
		LCD_ShowChinese12x12(143,64,"��",WHITE,LGRAY,12,0); //��ʾ����
		}
	}
//��ʾδ��������
static void ShowbalanceStatic(void)
	{
	LCD_ShowChinese(3,21,"����ʱ��",WHITE,LGRAY,0);
	ShowTimeCode(21,LogData.BalanceTime);
	
	LCD_ShowChinese(3,35,"δ��������",WHITE,LGRAY,0);
	DisplayAh(LogData.UnbalanceBatteryAh*1000,35);
	}
	
//������ʾ�İ�������
void ColHisKeyHandler(void)
	{
	//���·�ҳ
	if(KeyState.KeyEvent==KeyEvent_Down&&ShowCMenuState>0)ShowCMenuState--;
	if(KeyState.KeyEvent==KeyEvent_Up&&ShowCMenuState<2)ShowCMenuState++;
	//�˳�
	if(KeyState.KeyEvent==KeyEvent_ESC)
		{
		if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
		else SwitchingMenu(&SetMainMenu); //�����˳�״̬,����ESC��ص����˵�
		}
	if(KeyState.KeyEvent!=KeyEvent_None)
		{
		IsUpdateCDUI=true;
		KeyState.KeyEvent=KeyEvent_None;
		}
	}

//��ʾGUI
void ShowColHisGUI(void)
	{
	if(!IsUpdateCDUI)return;
	RenderMenuBG();
	switch(ShowCMenuState)
		{
		case 0:ShowColumbDataUpper();break;
		case 1:ShowColumbDataLower();break;
		case 2:ShowbalanceStatic();break;
		}
	IsUpdateCDUI=false;
	}
	
void ResetColHisMenuToUpper(void)
	{
	//ÿ�ν���ʱ��������
	IsUpdateCDUI=true;
	ShowCMenuState=0;
	}	
	
const MenuConfigDef ColHisMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&ShowColHisGUI, //��Ⱦ����
	&ColHisKeyHandler, //��������
	//�������ò˵�����Ҫ�ñ������
	"�鿴���ؼ���ʷ����",
	NULL,
	NULL, 
	NULL,
	//������˳����캯��û������Ҫ��
	&ResetColHisMenuToUpper,
	NULL
	};	
