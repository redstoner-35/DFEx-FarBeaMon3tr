#include "ht32.h"
#include "delay.h"
#include "Pindefs.h"
#include "GUI.h"
#include "BalanceMgmt.h"

//电源管理引脚自动定义
#define LDO_EN_IOB STRCAT2(GPIO_P,LDO_EN_IOBank)
#define LDO_EN_IOG STRCAT2(HT_GPIO,LDO_EN_IOBank)
#define LDO_EN_IOP STRCAT2(GPIO_PIN_,LDO_EN_IOPinNum) 

//int引脚自动定义
#define IP2366_INT_IOB STRCAT2(GPIO_P,IP2366_INT_IOBank)
#define IP2366_INT_IOG STRCAT2(HT_GPIO,IP2366_INT_IOBank)
#define IP2366_INT_IOP STRCAT2(GPIO_PIN_,IP2366_INT_IOPinNum) 

//EN引脚自动定义
#define IP2366_EN_IOB STRCAT2(GPIO_P,IP2366_EN_IOBank)
#define IP2366_EN_IOG STRCAT2(HT_GPIO,IP2366_EN_IOBank)
#define IP2366_EN_IOP STRCAT2(GPIO_PIN_,IP2366_EN_IOPinNum) 

//内部变量
short SleepTimer; //睡眠定时 
short IPStallTime=0; //IP2368重启

//初始化IO完成自举操作
void PowerMgmtSetup(void)
  {
	 //配置GPIO
   AFIO_GPxConfig(LDO_EN_IOB,LDO_EN_IOP, AFIO_FUN_GPIO);
   GPIO_DirectionConfig(LDO_EN_IOG,LDO_EN_IOP,GPIO_DIR_OUT);//配置为输出 
	 GPIO_DriveConfig(LDO_EN_IOG,LDO_EN_IOP,GPIO_DV_16MA);	//设置为16mA最大输出	
	 GPIO_ClearOutBits(LDO_EN_IOG,LDO_EN_IOP);//默认输出设置为0
	 
	 AFIO_GPxConfig(IP2366_INT_IOB,IP2366_INT_IOP, AFIO_FUN_GPIO);
	 GPIO_DirectionConfig(IP2366_INT_IOG,IP2366_INT_IOP,GPIO_DIR_IN);//设置为高阻输入
	 GPIO_InputConfig(IP2366_INT_IOG,IP2366_INT_IOP,ENABLE);  //打开输入寄存器
	 GPIO_PullResistorConfig(IP2366_INT_IOG,IP2366_INT_IOP,GPIO_PR_DOWN); //打开下拉
		
   AFIO_GPxConfig(IP2366_EN_IOB,IP2366_EN_IOP, AFIO_FUN_GPIO);
   GPIO_DirectionConfig(IP2366_EN_IOG,IP2366_EN_IOP,GPIO_DIR_OUT);//配置为输出 
	 GPIO_DriveConfig(IP2366_EN_IOG,IP2366_EN_IOP,GPIO_DV_16MA);	//设置为16mA最大输出			
	 //设置输出
   GPIO_ClearOutBits(IP2366_EN_IOG,IP2366_EN_IOP); //2366-EN=0
	 delay_ms(100);
	 GPIO_SetOutBits(LDO_EN_IOG,LDO_EN_IOP);//输出设置为1
	 //配置启动计时器
	 SleepTimer=480; //一分钟无操作自动休眠
	}
	
//Type-C连接失败时，进行重新握手的部分
void IP2366StallRestore(void)
  {
	int wait;
	//计时累加部分
	IPStallTime++;
	if(IPStallTime>=32) //IP2366掉线大约4秒后开始重试
	  {
		IPStallTime=0;  //操作复位，等待下次计时
		//设置为高阻输入使2366休眠
		GPIO_InputConfig(IP2366_INT_IOG,IP2366_INT_IOP,ENABLE); 
	  GPIO_DirectionConfig(IP2366_INT_IOG,IP2366_INT_IOP,GPIO_DIR_IN);
		//给EN发送脉冲
		delay_ms(200);	
		GPIO_SetOutBits(IP2366_EN_IOG,IP2366_EN_IOP); 
	  delay_ms(200);
    GPIO_ClearOutBits(IP2366_EN_IOG,IP2366_EN_IOP);	//令2366-EN=1，尝试监测		
		//监测是否成功唤醒
		wait=300; //唤醒检测延时300mS
		do
		  {
			wait--;
      if(wait<=0)break;	//200mS后IP2368唤醒失败，退出
			delay_ms(1);		
			}
		while(GPIO_ReadInBit(IP2366_INT_IOG,IP2366_INT_IOP)==SET); //IP2366位于唤醒状态，等待唤醒结束
		//如果成功，则再100mS后锁住INT使2366保持唤醒
		if(wait>0)delay_ms(100);
		GPIO_InputConfig(IP2366_INT_IOG,IP2366_INT_IOP,DISABLE); 
	  GPIO_DirectionConfig(IP2366_INT_IOG,IP2366_INT_IOP,GPIO_DIR_OUT);//禁用IDR，设置为输出
	  GPIO_SetOutBits(IP2366_INT_IOG,IP2366_INT_IOP); //令INT保持在1,使IP2366永远唤醒不得睡眠	
    }
	}		
	
//强制关机
void ShutSysOFF(void)
	{
	extern bool IsEnablePowerOFF;
	if(!IsEnablePowerOFF)return; //不允许关机
	ClearScreen();
	LCD_DeInit(); //除能LCD
	Balance_ForceDiasble(); //发送命令关闭均衡系统
	GPIO_ClearOutBits(LDO_EN_IOG,LDO_EN_IOP);//输出设置为0,关闭LDO电源强迫单片机掉电
	delay_ms(300);
	NVIC_SystemReset();
	while(1);		
	}	
	
//给2366踢一脚把2366踹醒
void KickIP2366ToWakeUp(void)
	{
	int retry=3;
	char WakeMsg[]={"唤醒重试次数:5"};
	ShowPostInfo(12,"唤醒充电IC","05",Msg_Statu);
	//如果INT=0说明2366在睡觉
	if(GPIO_ReadInBit(IP2366_INT_IOG,IP2366_INT_IOP)==RESET)do
		{
		GPIO_SetOutBits(IP2366_EN_IOG,IP2366_EN_IOP); 
	  delay_ms(250);
    GPIO_ClearOutBits(IP2366_EN_IOG,IP2366_EN_IOP);	//令2366-EN=1，尝试监测
		delay_ms(200);	
		if(GPIO_ReadInBit(IP2366_INT_IOG,IP2366_INT_IOP)==SET)break; //唤醒成功
		//尝试失败，提示唤醒剩余次数
		retry--;
		WakeMsg[13]=0x30+(3-retry);
		ShowPostInfo(12,WakeMsg,"W0",Msg_Warning);		
    //延时2秒后重试			
    delay_Second(2);	
		}	
	while(retry); 
	//唤醒失败
	if(!retry)
		{
		ShowPostInfo(12,"充电IC唤醒失败","E2",Msg_Fault);
		SelfTestErrorHandler();
		}
	GPIO_SetOutBits(IP2366_INT_IOG,IP2366_INT_IOP);
	GPIO_DirectionConfig(IP2366_INT_IOG,IP2366_INT_IOP,GPIO_DIR_OUT);
	GPIO_InputConfig(IP2366_INT_IOG,IP2366_INT_IOP,DISABLE);  //设置为推挽输出保持2366唤醒
	}
	
//休眠状态判断
void PowermanagementSleepControl(void)
  {
	extern bool IsEnablePowerOFF;
	extern int BalanceForceEnableTIM;
	//当前未处于睡眠状态、均衡开启或者Type-C处于连接中，不执行
	if(!IsEnablePowerOFF||BalanceForceEnableTIM)
		{
		SleepTimer=480;	
		return; //复位计时器
		}
	if(SleepTimer<8)Balance_ForceDiasble(); //强制关闭均衡器	
	//时间未到继续计时
	if(SleepTimer>0)return;
	//开始检测
	GPIO_InputConfig(IP2366_INT_IOG,IP2366_INT_IOP,ENABLE); 
	GPIO_DirectionConfig(IP2366_INT_IOG,IP2366_INT_IOP,GPIO_DIR_IN);//设置为高阻输入使2366休眠
	if(GPIO_ReadInBit(IP2366_INT_IOG,IP2366_INT_IOP)==SET)
		{
		SleepTimer=10;
		return; //如果IP2366未进入睡眠则继续等待
		}
	//立即释放IO
	ClearScreen();
	GPIO_ClearOutBits(LDO_EN_IOG,LDO_EN_IOP);//输出设置为0,关闭LDO电源强迫单片机掉电
	while(1);		
	}
	
	
