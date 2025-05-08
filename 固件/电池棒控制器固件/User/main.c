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

//��������
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

//����
bool SensorRefreshFlag=false;

int main(void)
 {
 unsigned char WDTResetDelay=4;
 //��ʼ��ϵͳʱ��
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
 //����SYSTICK�͵�Դ��������
 SetDebugPortState(false);
 delay_init(); //����ʱ���������л������г�ʼ�� 
 PowerMgmtSetup(); //�����Ծ�
 //�ײ������ʼ��
 LCD_HardwareInit();
 LCD_Init(); //����LCD��Ӳ����ʼ���ͼĴ�����ʼ��
 SideKey_Init(); //�ఴ��ʼ�� 	
 PostScreenInit(); //��ʾ�Լ�ĳ�ʼ������
 LCD_EnableBackLight(); //����LCD���⣬LCD��ʼ��ʾ
 //��ʼͼ�λ��Լ�
 EnableHBTimer(); //��ʼ��ϵͳ8Hz������ʱ��
 InternalADC_Init(); //ADC��ʼ�� 
 KickIP2366ToWakeUp(); //��һ��2366����
 SMBUS_Init(); //����SMBUS
 IP2366_PreInit(); //����2368�������ùرճ�ŵ磬��ȡ�Ĵ������ٿ�
 LoadConfig(); //��ȡϵͳ����
 CheckIfHBTIMStart(); //���������ʱ���Ƿ�����
 CheckForFlashLock(); //���洢���������ý�ֹ��ȡ
 POR_ReadCapData(); //��ȡ��������
 BalanceMgmt_Init(); //���þ��������
 RunLogModule_POR(); //��ȡ����
 ApplyScreenDirection(); //Ӧ����Ļ����
 IP2366_PostInit(); //IP2366Ӧ�ú�����
 HPPowerGuage_Start(); //��ʼ���߾��ȹ��ʼ�
 PushDefaultResultToVBat(); //����ص�ѹ����Ĭ��Ӧ�ø��ṹ�������ʾ
 //�Լ����,������������100%
 ShowPostInfo(100,"ϵͳ��ʼ�����","AA",Msg_POSTOK);
 if(!CfgData.EnableFastBoot)delay_ms(300); //�رտ������������ʱһ�����˿��Կ���
 WatchDog_Init(); //�������Ź�
 ClearScreen();
 EnteredInstantCapTest(); //���Խ�����Բ˵�
 SensorRefreshFlag=false; //���flag
 //��ѭ��
 while(1)
   {
	 //ʵʱ��������
	 ADC_GetResult(); 
	 SideKey_LogicHandler(); 
	 CTestFSMHandler(); //����״̬������
	 PowermanagementSleepControl(); //˯�߹���
	 SysOverHeatProt(); //���ȱ�������
	 MenuRenderProcess(); //ִ�в˵���Ⱦ 
	 //125ms��ʱ�������ں�ι�����
	 if(!SensorRefreshFlag)continue;
	 if(WDTResetDelay>0)WDTResetDelay--; //������Ϊ��ȷ��ϵͳ�Ѿ���ѭ���ڹ����ȶ��ˡ��ſ�ʼι������Ȼ����ι�����̫�̻�������������
	 else WatchDog_Feed(); //ι��
	 //��������125ms��ʱ����
	 Balance_IOMgmt(); //���о������Ŀ���
	 SideKey_TIMCallback(); 
	 UpdateIfSysCanOFF(); //����ϵͳ�Ƿ���Թر�
	 IP2366_Telem(); //ÿ0.125���ȡһ��2366��״̬
	 CTestAverageACC(); //����ϵͳң��
	 UpdataRunTimeLog(); //������־
	 OverChargeDetectModule(); //������
	 DetectIfIP2366Reset(); //���IP2366�Ƿ�λ
	 GUIDelayHandler(); //GUI��ʱ����
	 IP2366_ReConfigOutWhenTypeCOFF(); //��VBUS��������������
	 IntEditMenuKeyEffHandler(); //ʵ�ְ�����Ч
	 TCResetFSM(); //ʵ��Type-C����
	 AttackTimeCounter(); //��ֹ���ƹ����ı���
	 SensorRefreshFlag=false;
	 }
 return 0;
 }
