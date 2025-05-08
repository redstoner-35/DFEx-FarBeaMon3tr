#include "ht32.h"
#include "delay.h"
#include "LCD_Init.h"
#include "lcd.h"
#include "ADC.h"
#include "I2C.h"
#include "Key.h"
#include "GUI.h"
#include "Config.h"
#include "CapTest.h"
#include "LogSystem.h"
#include "WatchDog.h"
#include "BalanceMgmt.h"

//函数声明
void PowerMgmtSetup(void);
void KickIP2366ToWakeUp(void);
void IP2366_PreInit(void);
void IP2366_Telem(void);
void IP2366_PostInit(void);
void UpdateIfSysCanOFF(void);
void DetectIfIP2366Reset(void);
void EnteredInstantCapTest(void);
void PowermanagementSleepControl(void);
void CTestAverageACC(void);
void CTestFSMHandler(void);
void SysOverHeatProt(void);
void TCResetFSM(void);
void ApplyScreenDirection(void);
void IP2366_ReConfigOutWhenTypeCOFF(void);
void CheckForFlashLock(void);
void SetDebugPortState(bool IsEnable);
void AttackTimeCounter(void);
void PushDefaultResultToVBat(void);
void OverChargeDetectModule(void);
void HPPowerGuage_Start(void);

//常量
bool SensorRefreshFlag=false;

int main(void)
 {
 unsigned char WDTResetDelay=4;
 //初始化系统时钟
 CKCU_PeripClockConfig_TypeDef CLKConfig={{0}};
 CLKConfig.Bit.PA=1;
 CLKConfig.Bit.PB=1;
 CLKConfig.Bit.PC=1;
 CLKConfig.Bit.AFIO=1;
 CLKConfig.Bit.ADC=1;
 CLKConfig.Bit.EXTI=1;
 CLKConfig.Bit.SPI0 = 1;
 CLKConfig.Bit.PDMA = 1;
 CLKConfig.Bit.WDT = 1;
 CLKConfig.Bit.BKP = 1;
 CKCU_PeripClockConfig(CLKConfig,ENABLE);
 //启动SYSTICK和电源锁定引脚
 SetDebugPortState(false);
 delay_init(); //对延时函数的运行环境进行初始化 
 PowerMgmtSetup(); //进行自举
 //底层外设初始化
 LCD_HardwareInit();
 LCD_Init(); //进行LCD的硬件初始化和寄存器初始化
 SideKey_Init(); //侧按初始化 	
 PostScreenInit(); //显示自检的初始主界面
 LCD_EnableBackLight(); //启动LCD背光，LCD开始显示
 //开始图形化自检
 EnableHBTimer(); //初始化系统8Hz心跳定时器
 InternalADC_Init(); //ADC初始化 
 KickIP2366ToWakeUp(); //踢一脚2366唤醒
 SMBUS_Init(); //启动SMBUS
 IP2366_PreInit(); //进行2368初步配置关闭充放电，读取寄存器后再开
 LoadConfig(); //读取系统配置
 CheckIfHBTIMStart(); //检查心跳定时器是否启动
 CheckForFlashLock(); //给存储器上锁永久禁止读取
 POR_ReadCapData(); //读取测容数据
 BalanceMgmt_Init(); //配置均衡控制器
 RunLogModule_POR(); //读取数据
 ApplyScreenDirection(); //应用屏幕方向
 IP2366_PostInit(); //IP2366应用后配置
 HPPowerGuage_Start(); //初始化高精度功率计
 PushDefaultResultToVBat(); //将电池电压数据默认应用给结构体进行显示
 //自检结束,将进度条跳到100%
 ShowPostInfo(100,"系统初始化完成","AA",Msg_POSTOK);
 if(!CfgData.EnableFastBoot)delay_ms(300); //关闭快速启动则多延时一会让人可以看清
 WatchDog_Init(); //启动看门狗
 ClearScreen();
 EnteredInstantCapTest(); //尝试进入测试菜单
 SensorRefreshFlag=false; //清除flag
 //主循环
 while(1)
   {
	 //实时处理任务
	 ADC_GetResult(); 
	 SideKey_LogicHandler(); 
	 CTestFSMHandler(); //测容状态机处理
	 PowermanagementSleepControl(); //睡眠管理
	 SysOverHeatProt(); //过热保护处理
	 MenuRenderProcess(); //执行菜单渲染 
	 //125ms定时处理的入口和喂狗检测
	 if(!SensorRefreshFlag)continue;
	 if(WDTResetDelay>0)WDTResetDelay--; //这里是为了确保系统已经在循环内工作稳定了。才开始喂狗，不然两次喂狗间隔太短会立即触发重启
	 else WatchDog_Feed(); //喂狗
	 //进行其余125ms定时处理
	 Balance_IOMgmt(); //进行均衡器的控制
	 SideKey_TIMCallback(); 
	 UpdateIfSysCanOFF(); //更新系统是否可以关闭
	 IP2366_Telem(); //每0.125秒获取一次2366的状态
	 CTestAverageACC(); //测容系统遥测
	 UpdataRunTimeLog(); //更新日志
	 OverChargeDetectModule(); //过充检测
	 DetectIfIP2366Reset(); //监测IP2366是否复位
	 GUIDelayHandler(); //GUI延时处理
	 IP2366_ReConfigOutWhenTypeCOFF(); //从VBUS起来的重新连接
	 IntEditMenuKeyEffHandler(); //实现按下特效
	 TCResetFSM(); //实现Type-C重连
	 AttackTimeCounter(); //防止爆破攻击的保护
	 SensorRefreshFlag=false;
	 }
 return 0;
 }
