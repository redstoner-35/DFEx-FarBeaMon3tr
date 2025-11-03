#ifndef _OC_
#define _OC_

//内部define
typedef enum
	{
	OCFSM_Idle, //风扇处于关闭状态
	OCFSM_Voltage_PreBIAS, //系统工作在电压模式，预先送PWMDAC基准
	OCFSM_Start_DCDC, //等待DCDC启动
	OCFSM_PWMRampingUp, //开启风扇电源，风扇起转
	OCFSM_RaiseVOUT, //输出通道进行电压抬升处理
	OCFSM_NormalOperation, //风扇已经完成启动，正常操作
	OCFSM_ShutOFF //输出状态机关闭风扇的流程
	}OCFSMStateDef;

typedef struct
	{
	unsigned char SysMinSpeed;
	float SysMinVolt;
	int SysStartUpDuty;
	}MinMaxDutyVOutDef;

	
//外部参考
extern xdata float TargetVoltage;          //目标风扇电压(仅电压模式生效)
extern xdata float TargetfanSpeed; //目标风扇速度
extern bit IsUpdateFanSpeed; //更新风扇速度	
extern xdata MinMaxDutyVOutDef VMinMaxCfg; //存储系统最小最大数据	
	
//函数
void OutputChannel_Calc(void);  //输出通道运算处理
void OutputChannel_Init(void);	//初始化输出通道状态机	
bit GetIfFanOutputEnabled(void); //获取风扇输出是否已经开启
void OutputChannel_DeInit(void); //输出通道强制复位
	
#endif
