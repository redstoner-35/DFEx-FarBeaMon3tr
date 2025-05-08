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
static int AttackLockDelay=0; //攻击保护监测
static int RetryCount; //重试次数

//密码输入初始化
void PasswordEnterInit(void)
	{
	memset(PassWordStor,'0',8);
	PassWordStor[8]=0x00;
	CurrentEnterIdx=0;	
	IsPassWordError=false;
	IsPWUpdated=false;
	DecGUICoutner=AttackLockDelay%8;
	}
	
//防止爆破的倒计时
void AttackTimeCounter(void)
	{
	if(AttackLockDelay==1)
		{
		AttackLockDelay--;
		LogData.IsEnablePunish=false;
		ForceWriteRuntimelog(); //强制写入日志标记锁定退出
		RetryCount=0; //时间到，复位重试次数
		}
	else if(AttackLockDelay>0)
		{
		//每秒更新一次GUI
		if(DecGUICoutner>0)DecGUICoutner--;
		else 
			{
			DecGUICoutner=8;
			IsPWUpdated=false;
			}
		//继续向下减少时间
		AttackLockDelay--;
		}
	}	
	
//断电攻击保护
void AttackDetectInit(void)
	{
	DecGUICoutner=8;
	RetryCount=10;
	if(!LogHeader.IsRunlogHasContent)return; //不采用无效数据
	if(LogData.IsEnablePunish)AttackLockDelay=4800; //上次退出系统后断电，重置延时
	else AttackLockDelay=0; //清除结果
	if(LogData.IsEnablePunish)RetryCount=10;
	}
	
void PassWordMenuRender(const PasswordInputDef *CFG)
	{
	char PassDisBuf[9];
	char i,buf;
	//界面已更新
	if(IsPWUpdated&&KeyState.KeyEvent==KeyEvent_None)return;
	RenderMenuBG();
	//由于错误次数过多系统被锁定
	if(AttackLockDelay>0)	
		{
		LCD_ShowChinese(21,25,"检测到疑似爆破攻击",YELLOW,LGRAY,0);
		LCD_ShowChinese(22,39,"请在",WHITE,LGRAY,0);
		LCD_ShowIntNum(49,39,(AttackLockDelay/8)>0?AttackLockDelay/8:1,3,RED,LGRAY,12);
		LCD_ShowChinese(72,39,"秒后再尝试",WHITE,LGRAY,0);
		LCD_ShowChinese(32,61,"按下",WHITE,LGRAY,0);
		LCD_ShowString(59,61,"ESC",YELLOW,LGRAY,12,0);
		LCD_ShowChinese(86,61,"以退出",WHITE,LGRAY,0);
		//按键处理
		IsPWUpdated=true;
		if(KeyState.KeyEvent==KeyEvent_ESC&&CFG->ThingsToDoWhenExit!=NULL)CFG->ThingsToDoWhenExit();	
		KeyState.KeyEvent=KeyEvent_None;	
		return;
		}
	//正常输入密码
	LCD_ShowChinese(27,27,"请输入管理员密码",WHITE,LGRAY,0);
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
		LCD_ShowChinese(8,65,"密码错误，剩余",RED,LGRAY,0);
	  LCD_ShowIntNum(98,65,RetryCount,2,RED,LGRAY,12);
	  LCD_ShowChinese(115,65,"次尝试",RED,LGRAY,0);
		}
	//按键操作
	if(KeyState.KeyEvent==KeyEvent_Enter)
		{
		if(CurrentEnterIdx==7)
			{
			//从传入的配置内还原密码
			for(i=0;i<4;i++)
				{
				buf=CFG->PassWord[i]^0xAA;
				PassDisBuf[i*2]='0'+((buf>>4)&0x0F);
				PassDisBuf[1+(i*2)]='0'+(buf&0x0F);
				}	
			PassDisBuf[8]=0;
			//对密码数据进行比对
			if(!strncmp(PassWordStor,PassDisBuf,9))
				{
				RetryCount=10; //复位重试次数
				if(CFG->ThingsToDoWhenEnter!=NULL)CFG->ThingsToDoWhenEnter(); 
				KeyState.KeyEvent=KeyEvent_None; //清除按键事件
				return;
				}
			else //密码出错
				{
				RetryCount--;
				if(!RetryCount)
					{
					AttackLockDelay=4800;
					LogData.IsEnablePunish=true;
					ForceWriteRuntimelog(); //强制写入日志标记锁定发生
					if(CFG->ThingsToDoWhenExit!=NULL)CFG->ThingsToDoWhenExit();
					return;
					}
        PasswordEnterInit();
				IsPassWordError=true;
				}
			}
		//还没输完按下enter继续输下一位
		else
			{			
			CurrentEnterIdx++;
			IsPassWordError=false; //清除错误提示
			}
		}		
	//按下退出
	if(KeyState.KeyEvent==KeyEvent_ESC&&CFG->ThingsToDoWhenExit!=NULL)CFG->ThingsToDoWhenExit();
	//加减数值
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
	//按键渲染选择
	if(KeyState.KeyEvent==KeyEvent_None)IsPWUpdated=true;
	else
		{
		KeyState.KeyEvent=KeyEvent_None;
		IsPWUpdated=false;
		}
	}
