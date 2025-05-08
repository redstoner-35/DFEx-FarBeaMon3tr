#include "GUI.h"
#include "Key.h"
#include "LogSystem.h"
#include <string.h>

static bool IsPWUpdated;
static bool IsPassWordError;
static char PassWordStor[9]={0};
static char CurrentEnterIdx;
static char DecGUICoutner;
bool IsEnableAdvancedMode=false;
static int AttackLockDelay=0; //�����������
static int RetryCount; //���Դ���

//���������ʼ��
void PasswordEnterInit(void)
	{
	memset(PassWordStor,'0',8);
	PassWordStor[8]=0x00;
	CurrentEnterIdx=0;	
	IsPassWordError=false;
	IsPWUpdated=false;
	DecGUICoutner=AttackLockDelay%8;
	}
	
//��ֹ���Ƶĵ���ʱ
void AttackTimeCounter(void)
	{
	if(AttackLockDelay==1)
		{
		AttackLockDelay--;
		LogData.IsEnablePunish=false;
		ForceWriteRuntimelog(); //ǿ��д����־��������˳�
		RetryCount=0; //ʱ�䵽����λ���Դ���
		}
	else if(AttackLockDelay>0)
		{
		//ÿ�����һ��GUI
		if(DecGUICoutner>0)DecGUICoutner--;
		else 
			{
			DecGUICoutner=8;
			IsPWUpdated=false;
			}
		//�������¼���ʱ��
		AttackLockDelay--;
		}
	}	
	
//�ϵ繥������
void AttackDetectInit(void)
	{
	DecGUICoutner=8;
	RetryCount=10;
	if(!LogHeader.IsRunlogHasContent)return; //��������Ч����
	if(LogData.IsEnablePunish)AttackLockDelay=4800; //�ϴ��˳�ϵͳ��ϵ磬������ʱ
	else AttackLockDelay=0; //������
	if(LogData.IsEnablePunish)RetryCount=10;
	}
	
void PassWordMenuRender(const PasswordInputDef *CFG)
	{
	char PassDisBuf[9];
	char i,buf;
	//�����Ѹ���
	if(IsPWUpdated&&KeyState.KeyEvent==KeyEvent_None)return;
	RenderMenuBG();
	//���ڴ����������ϵͳ������
	if(AttackLockDelay>0)	
		{
		LCD_ShowChinese(21,25,"��⵽���Ʊ��ƹ���",YELLOW,LGRAY,0);
		LCD_ShowChinese(22,39,"����",WHITE,LGRAY,0);
		LCD_ShowIntNum(49,39,(AttackLockDelay/8)>0?AttackLockDelay/8:1,3,RED,LGRAY,12);
		LCD_ShowChinese(72,39,"����ٳ���",WHITE,LGRAY,0);
		LCD_ShowChinese(32,61,"����",WHITE,LGRAY,0);
		LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		LCD_ShowChinese(86,61,"���˳�",WHITE,LGRAY,0);
		//��������
		IsPWUpdated=true;
		if(KeyState.KeyEvent==KeyEvent_ESC&&CFG->ThingsToDoWhenExit!=NULL)CFG->ThingsToDoWhenExit();	
		KeyState.KeyEvent=KeyEvent_None;	
		return;
		}
	//������������
	LCD_ShowChinese(27,27,"���������Ա����",WHITE,LGRAY,0);
	PassDisBuf[8]=0x00;
	for(i=0;i<8;i++)
		{
		if(CurrentEnterIdx==i)PassDisBuf[i]=PassWordStor[i];
		else PassDisBuf[i]='*';
		}
	LCD_ShowString(49,48,PassDisBuf,CYAN,LGRAY,12,0);
	LCD_DrawRectangle(27,43,129,61,BLACK);
	if(IsPassWordError)
		{
		LCD_ShowChinese(8,65,"�������ʣ��",RED,LGRAY,0);
	  LCD_ShowIntNum(98,65,RetryCount,2,RED,LGRAY,12);
	  LCD_ShowChinese(115,65,"�γ���",RED,LGRAY,0);
		}
	//��������
	if(KeyState.KeyEvent==KeyEvent_Enter)
		{
		if(CurrentEnterIdx==7)
			{
			//�Ӵ���������ڻ�ԭ����
			for(i=0;i<4;i++)
				{
				buf=CFG->PassWord[i]^0xAA;
				PassDisBuf[i*2]='0'+((buf>>4)&0x0F);
				PassDisBuf[1+(i*2)]='0'+(buf&0x0F);
				}	
			PassDisBuf[8]=0;
			//���������ݽ��бȶ�
			if(!strncmp(PassWordStor,PassDisBuf,9))
				{
				RetryCount=10; //��λ���Դ���
				if(CFG->ThingsToDoWhenEnter!=NULL)CFG->ThingsToDoWhenEnter(); 
				KeyState.KeyEvent=KeyEvent_None; //��������¼�
				return;
				}
			else //�������
				{
				RetryCount--;
				if(!RetryCount)
					{
					AttackLockDelay=4800;
					LogData.IsEnablePunish=true;
					ForceWriteRuntimelog(); //ǿ��д����־�����������
					if(CFG->ThingsToDoWhenExit!=NULL)CFG->ThingsToDoWhenExit();
					return;
					}
        PasswordEnterInit();
				IsPassWordError=true;
				}
			}
		//��û���갴��enter��������һλ
		else
			{			
			CurrentEnterIdx++;
			IsPassWordError=false; //���������ʾ
			}
		}		
	//�����˳�
	if(KeyState.KeyEvent==KeyEvent_ESC&&CFG->ThingsToDoWhenExit!=NULL)CFG->ThingsToDoWhenExit();
	//�Ӽ���ֵ
	if(KeyState.KeyEvent==KeyEvent_Up)
		{
		if(PassWordStor[CurrentEnterIdx]<'9')PassWordStor[CurrentEnterIdx]++;
		else PassWordStor[CurrentEnterIdx]='0';
		}
	if(KeyState.KeyEvent==KeyEvent_Down)
		{
		if(PassWordStor[CurrentEnterIdx]>'0')PassWordStor[CurrentEnterIdx]--;
		else PassWordStor[CurrentEnterIdx]='9';		
		}
	//������Ⱦѡ��
	if(KeyState.KeyEvent==KeyEvent_None)IsPWUpdated=true;
	else
		{
		KeyState.KeyEvent=KeyEvent_None;
		IsPWUpdated=false;
		}
	}
