#ifndef _OC_
#define _OC_

//输出通道状态机
typedef enum
	{
	//输出通道彻底关闭，待机状态
	OutCH_Standby=0,
	//输出通道正常启动流程
	OutCH_PWMDACPreCharge=1,  //PWMDAC预偏置
	OutCH_StartAUXPSU=2, //启动辅助PSU
	OutCH_EnableBoost=3, //启动主Boost
	OutCH_ReleasePreCharge=4, //逐步复位PreCharge DAC让输出电压慢慢爬升，从CV状态过渡到FB注入的CC状态
	OutCH_SubmitDuty=5, //应用占空比，LED爬升到目标电流
	//输出通道正常运行阶段
	OutCH_OutputEnabled=6,
	OutCH_1LumenOpenRun=7,
	//安全关闭阶段
	OutCH_GracefulShut=8,
	OutCH_WaitVOUTDecay=9,
	//输出通道在爆闪等阶段进入idle(LED断开，输出配置为14.7V)
	OutCH_EnterIdle=10,	
	OutCH_OutputIdle=11,
  //输出通道启动失败
	OutCH_PreChargeFailed=12
	}OutChFSMStateDef;

//输出通道参数设置
#define MainChannelShuntmOhm 1.00 //主通道的检流电阻阻值(mR)
#define CurrentSenseOpAmpGain 100 //电流检测放大器的增益

//输出通道电流参考和PWMDAC整定计算宏（绝对不要修改！会爆炸！）	
#define CalcIREFValue(x) ((x/2)+(x/6))
#define CalcPWMDACDuty(x) (((1129000UL-(4815UL*x))*24UL)/5000UL)  //使用整数方式计算PWMDAC预充电压

//输出PWMDAC预充电压配置
#define PWMDACPreCharge 144 	//PWMDAC在正常情况下的预充电压(LSB=0.1V)
#define OneLumenOut 145 //PWMDAC在1流明挡位下的预充电压(LSB=0.1V，FV7212D=14500mV，其他的灯珠需要自己试)
	
/*自动计算系统PWMDAC的预充配置数值，请勿修改！！！！*/	
#define CVPreChargeDACVal CalcPWMDACDuty(PWMDACPreCharge)
#define OneLumenCVDACVal CalcPWMDACDuty(OneLumenOut)
	
#if (OneLumenCVDACVal>CVPreChargeDACVal)
	 //1LM挡位的目标输出电压所需要的PWMDAC数值高于预启动所需的数值，会引起闪烁故最终偏置的结果使用1LM的DAC值
	 #define CVPreStartDACVal OneLumenCVDACVal
	 #define OneLMDACVal OneLumenCVDACVal
#else
   //1LM挡位的目标输出电压所需要的PWMDAC数值低于预启动所需的数值，可以分开使用
	 #define CVPreStartDACVal CVPreChargeDACVal
	 #define OneLMDACVal OneLumenCVDACVal
#endif

#if (OneLumenCVDACVal < 0 | OneLumenCVDACVal > 2399)
   #error "Error 009: Invalid CV PWMDAC Output Config Value for One Lumen(Ultra Low mode)Output!"
#endif

#if (CVPreChargeDACVal < 0 | CVPreChargeDACVal > 2399)
   #error "Error 00A: Invalid CV PWMDAC Output Config Value for System StartUp!"
#endif


//外部参考
extern xdata volatile int Current; //电流值
extern xdata int CurrentBuf; //当前已应用的电流值
extern bit IsCurrentRampUp;  //电流正在上升过程中的标记位（用于和MPPT试探联动）
	
//函数
void OutputChannel_Init(void);
void OutputChannel_Calc(void);
void OCFSM_TIMHandler(void);
void OutputChannel_TestRun(void); //输出通道试运行
void OutputChannel_DeInit(void); //输出通道复位
bit GetIfOutputEnabled(void);		//外部获取输出是否正常启用的函数
bit GetIfSystemInPOFFSeq(void);	//获取系统是否在安全关机阶段
	
#endif
