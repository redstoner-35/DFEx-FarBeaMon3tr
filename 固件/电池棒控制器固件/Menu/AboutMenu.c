#include "GUI.h"
#include "Key.h"

//外部声明
extern const unsigned char MySign[1368];
void SetDebugPortState(bool IsEnable);

//内部变量
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
	
//准备渲染
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
	LCD_ShowChinese(10,33,"关注哔哩哔哩",WHITE,LGRAY,0);
	LCD_ShowString(91,33,"@MP86957",YELLOW,LGRAY,12,0);	
	LCD_ShowChinese(56,49,"谢谢喵！",WHITE,LGRAY,0);
	}	
	
static u16 ColorTextGen(void)
	{
	u16 FColor;
	u8 CCode;		
	switch(RGBState)
			{
			//红到绿色
			case Shift_R2G:
			  if(R>0)
					{
					R--;
					G++;
					}
			  else RGBState=Shift_G2B;
				break;
			//绿到蓝色
			case Shift_G2B:
				if(G>0)
					{
					G--;
					B++;
					}
				else RGBState=Shift_B2RG;
				break;
			//蓝到红色
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
	//颜色运算完毕
	return FColor;
	}	
	
void AboutMenuRender(void)
	{
	if(AboutIsRendered)
		{
		if(ShowEgg)LCD_ShowChinese(56,49,"谢谢喵！",ColorTextGen(),LGRAY,0);
		else LCD_ShowString(55,40,"Redstoner35",ColorTextGen(),LGRAY,12,0);	
		return;
		}
	//启动首次渲染
	RenderMenuBG();		
	if(ShowEgg)
		{
		AboutEasterEgg();
		AboutIsRendered=true;
		return;
		}
	LCD_ShowHybridString(4,21,"DFEx-GT96智慧电池子系统",WHITE,LGRAY,0);
	LCD_ShowChinese(4,40,"软硬件由",WHITE,LGRAY,0);
	LCD_ShowString(55,40,"Redstoner35",WHITE,LGRAY,12,0);		
	LCD_ShowChinese(133,40,"设计",WHITE,LGRAY,0);	
	LCD_ShowChinese(7,61,"固件版本",WHITE,LGRAY,0);
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
		SetDebugPortState(false); //退出之前关闭debug口
		if(!IsEnableAdvancedMode)SwitchingMenu(&EasySetMainMenu);
		else SwitchingMenu(&SetMainMenu); //处于退出状态,按下ESC后回到主菜单
		}
	KeyState.KeyEvent=KeyEvent_None;
	}	
	
	const MenuConfigDef AboutMenu=
	{
	MenuType_Custom,
	//布尔类的入口
	NULL,
	//枚举编辑的入口
	NULL,
  NULL,
  NULL,		
	//自定义入口
	&AboutMenuRender, 
	&AboutMenuKeyProc, //按键处理
	//不是设置菜单不需要用别的事情
	"关于",
	NULL,
	NULL, 
	NULL,
	//进入和退出构造函数
	&PrepareAboutRender, //进入时渲染
	NULL
	};
