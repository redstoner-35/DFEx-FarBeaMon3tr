#include "GUI.h"
#include "Key.h"

//�ⲿ����
extern const unsigned char MySign[1368];
void SetDebugPortState(bool IsEnable);

//�ڲ�����
static bool AboutIsRendered=false;
static bool ShowEgg=false;
static u8 R,G,B;

typedef enum
	{
	Shift_R2G,
	Shift_G2B,
	Shift_B2RG,
	Shift_RG2RB,
	Shift_W2R,
	Shift_GB2GR,
	Shift_RB2GB,
	Shift_GR2W
	}RGBShiftDef;

RGBShiftDef RGBState;	
	
//׼����Ⱦ
void PrepareAboutRender(void)
	{
	AboutIsRendered=false;
	ShowEgg=false;
	RGBState=Shift_R2G;
	R=255;
	G=0;
	B=0;
	}

void AboutEasterEgg(void)
	{
	LCD_ShowChinese(10,33,"��ע��������",WHITE,LGRAY,0);
	LCD_ShowString(91,33,"@MP86957",YELLOW,LGRAY,12,0);	
	LCD_ShowChinese(56,49,"лл����",WHITE,LGRAY,0);
	}	
	
static u16 ColorTextGen(void)
	{
	u16 FColor;
	u8 CCode;		
	switch(RGBState)
			{
			//�쵽��ɫ
			case Shift_R2G:
			  if(R>0)
					{
					R--;
					G++;
					}
			  else RGBState=Shift_G2B;
				break;
			//�̵���ɫ
			case Shift_G2B:
				if(G>0)
					{
					G--;
					B++;
					}
				else RGBState=Shift_B2RG;
				break;
			//������ɫ
			case Shift_B2RG:
				if(B>0)
					{
					B--;
					R++;
					G++;
					}
				else
					{
					if(R<255)R=255;
					if(G<255)G=255;
					RGBState=Shift_R2G;
					}
				break;
			case Shift_RG2RB:
        if(G>0)
					{
					G--;
					B++;
					}
				else RGBState=Shift_RB2GB;
				break;
			case Shift_RB2GB:
				if(R>0)
					{
					R--;
					G++;
					}
				else RGBState=Shift_GB2GR;
				break;
      case Shift_GB2GR:
        if(B>0)
					{
					B--;
					G++;
					}		
        else RGBState=Shift_GR2W;
				break;
      case Shift_GR2W:
        if(B<255)B++;
        else RGBState=Shift_W2R;	
        break;
      case Shift_W2R:
        if(G>0)G--;
        else if(B>0)B--;
			  else if(R<255)R++;
        else RGBState=Shift_R2G;
				break;
			}
		CCode=R>>3;
		FColor=((u16)CCode)<<11;	
		CCode=G>>3;
		FColor|=(((u16)CCode)<<6)&0x7C0;
		CCode=B>>3;
    FColor|=(u16)CCode&0x001F;
	//��ɫ�������
	return FColor;
	}	
	
void AboutMenuRender(void)
	{
	if(AboutIsRendered)
		{
		if(ShowEgg)LCD_ShowChinese(56,49,"лл����",ColorTextGen(),LGRAY,0);
		else LCD_ShowString(55,40,"Redstoner35",ColorTextGen(),LGRAY,12,0);	
		return;
		}
	//�����״���Ⱦ
	RenderMenuBG();		
	if(ShowEgg)
		{
		AboutEasterEgg();
		AboutIsRendered=true;
		return;
		}
	LCD_ShowHybridString(4,21,"DFEx-GT96�ǻ۵����ϵͳ",WHITE,LGRAY,0);
	LCD_ShowChinese(4,40,"��Ӳ����",WHITE,LGRAY,0);
	LCD_ShowString(55,40,"Redstoner35",WHITE,LGRAY,12,0);		
	LCD_ShowChinese(133,40,"���",WHITE,LGRAY,0);	
	LCD_ShowChinese(7,61,"�̼��汾",WHITE,LGRAY,0);
	LCD_ShowChar(57,61,':',WHITE,LGRAY,12,0);
	LCD_ShowString(64,61,"V1.2 Build1",CYAN,LGRAY,12,0);	
	AboutIsRendered=true;
	}

void AboutMenuKeyProc(void)
	{
	if(KeyState.KeyEvent==KeyEvent_BothEnt)
		{
	  ShowEgg=ShowEgg?false:true;
		if(ShowEgg)SetDebugPortState(true);
		AboutIsRendered=false;
		}
	if(KeyState.KeyEvent==KeyEvent_ESC)
		{
		SetDebugPortState(false); //�˳�֮ǰ�ر�debug��
		if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
		else SwitchingMenu(&SetMainMenu); //�����˳�״̬,����ESC��ص����˵�
		}
	KeyState.KeyEvent=KeyEvent_None;
	}	
	
	const MenuConfigDef AboutMenu=
	{
	MenuType_Custom,
	//����������
	NULL,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//�Զ������
	&AboutMenuRender, 
	&AboutMenuKeyProc, //��������
	//�������ò˵�����Ҫ�ñ������
	"����",
	NULL,
	NULL, 
	NULL,
	//������˳����캯��
	&PrepareAboutRender, //����ʱ��Ⱦ
	NULL
	};
