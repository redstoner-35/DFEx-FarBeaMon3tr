#include "ADCCfg.h"
#include "LEDMgmt.h"
#include "delay.h"
#include "ModeControl.h"
#include "TempControl.h"
#include "BattDisplay.h"
#include "OutputChannel.h"
#include "PWMCfg.h"
#include "LowVoltProt.h"
#include "SelfTest.h"

//内部变量
static xdata int TempIntegral;
static xdata int TempProtBuf;
static char StepDownTIM;  //降档显示计时
static unsigned char StepUpLockTIM; //计时器

//内部状态位
static bit IsThermalStepDown; //标记位，是否降档
static bit IsTempLIMActive;  //温控是否已经启动
static bit IsSystemShutDown; //是否触发温控强制关机

//外部状态位
bit IsDisableTurbo;  //禁止再度进入到极亮档
bit IsForceLeaveTurbo; //是否强制离开极亮档

//内部宏定义
#define MinumumILED CalcIREFValue(ILEDStepDown)


//换挡的时候根据当前恒温的电流重新PI值
void RecalcPILoop(int LastCurrent)	
	{
	int buf,ModeCur;
	//目标挡位不需要计算
	if(!CurrentMode->IsNeedStepDown)return;
	//获取当前挡位电流
	ModeCur=QueryCurrentGearILED();
	//计算P值缓存
	buf=TempProtBuf+(TempIntegral/IntegralFactor); //计算电流扣减值
	if(buf<0)buf=0; //电流扣减值不能小于0
  buf=LastCurrent-buf; //旧挡位电流减去扣减值得到实际电流(mA)
	TempProtBuf=ModeCur-LastCurrent; //P值缓存等于新挡位的电流-旧挡位实际电流(mA)
	if(TempProtBuf<0)TempProtBuf=0; //不允许比例缓存小于0
	TempIntegral=0; //积分缓存=0
	}
	
//输出当前温控的限流值
int ThermalILIMCalc(void)
	{
	int result;
	//判断温控是否需要进行计算
	if(!IsTempLIMActive)result=Current; //温控被关闭，电流限制进来多少返回去多少
	//开始温控计算
	else
		{
		result=TempProtBuf+(TempIntegral/IntegralFactor); //根据缓存计算结果
		if(result<0)result=0; //不允许负值出现
		result=Current-result; //计算限流值结果
		if(result<MinumumILED) //已经调到底了，禁止PID继续累加
			{
		  TempProtBuf=Current-MinumumILED; //将比例输出结果限幅为最小电流
		  TempIntegral=0;
		  result=MinumumILED; //电流限制不允许小于最低电流
			}
		}
	//返回结果	
	IsThermalStepDown=result==Current?0:1; //如果输入等于输出，则降档没发生
	return result; 
	}
//获取温控环路的恒温值
static int QueryConstantTemp(void)	
	{
	//极亮的时候使用更高的温控拉长降档时间
	return CurrentMode->ModeIdx==Mode_Turbo?TurboConstantTemperature:ConstantTemperature;
	}

//温控PI环计算
void ThermalPILoopCalc(void)	
	{
	int ProtFact,Err;
	//PI环关闭，复位数值
	if(!IsTempLIMActive)
		{
		TempIntegral=0;
		TempProtBuf=0;
		IsThermalStepDown=0;
		}
	//进行PI环的计算(仅在输出开启的时候进行或者爆闪模式运行过程中强制进行)
	else if(GetIfOutputEnabled()||CurrentMode->ModeIdx==Mode_Strobe)
		{
		//获取恒温温度值
		ProtFact=QueryConstantTemp();
		//温度误差为正
		if(Data.Systemp>ProtFact)
			{
			//计算误差
			Err=Data.Systemp-ProtFact;  //误差值等于目标温度-恒温温度
			//比例项(P)
			StepUpLockTIM=24; //升档之后温度过高则之后停止3秒
			if(Err>2)
				{
				ProtFact=(CurrentBuf/2300)+1;
			  if(Data.Systemp>(ForceDisableTurboTemp-5))ProtFact*=5; //温度过高，扩张比例系数
				TempProtBuf+=(ProtFact*Err);	//向buf提交比例项
				if(TempProtBuf>(CurrentMode->Current-MinumumILED))TempProtBuf=(CurrentMode->Current-MinumumILED); 
				StepUpLockTIM=60; //触发比例项降档，停7.5秒
				}
			//积分项(I)
			if(TempIntegral<IntegrateFullScale)TempIntegral++;
			}
		//温度小于恒温值
		else if(Data.Systemp<ProtFact)
			{
			//计算误差
			Err=ProtFact-Data.Systemp;	 //误差等于比例减积分	
			//比例项
			if(StepUpLockTIM)StepUpLockTIM--; //当前触发降档还没达到快速升档的时间
			else
				{
				if(Err&0x7E)TempProtBuf-=Err; //进行升档
				if(TempProtBuf<0)TempProtBuf=0;
				}
			//积分项
			if(TempIntegral>(-IntegrateFullScale))TempIntegral--;		
			}
		}
	}
//显示温度控制启动
bit ShowThermalStepDown(void)	
	{
	StepDownReasonDef Reason;
	//判断系统是否在降档
	if(VshowFSMState!=BattVdis_Waiting)Reason=StepDown_OFF; //当前处于电量显示状态不允许打断
	else if(IsThermalStepDown)Reason=StepDown_Thermal; //温控降档触发
	else if(CurrentMode->ModeIdx==Mode_Turbo&&TurboILIM<QueryCurrentGearILED())Reason=StepDowm_BattAlert; //电池撑不住
	else Reason=StepDown_OFF;
	//进行降档处理
  switch(Reason)		
		{
		case StepDown_OFF:StepDownTIM=0;break; //提示未触发
		case StepDowm_BattAlert: //电池警报
		  //当计时器=10时多闪一次制造出两次闪烁
			if(StepDownTIM==10)
				{
				StepDownTIM++;
				return 1;
				}
		case StepDown_Thermal: //过热
			StepDownTIM++;
			if(StepDownTIM==13)
				{
				StepDownTIM=0;
				return 1;
				}
			break;
		}
	//返回0
	return 0;
	}

//负责温度使能控制的施密特触发器
static bit TempSchmittTrigger(bit ValueIN,char HighThreshold,char LowThreshold)	
	{
	if(Data.Systemp>HighThreshold)return 1;
	if(Data.Systemp<LowThreshold)return 0;
	//数值保持，没有改变
	return ValueIN;
	}

//温度管理函数
void ThermalMgmtProcess(void)
	{
	bit ThermalStatus;
	//温度传感器错误
	if(!Data.IsNTCOK)
		{
		ReportError(Fault_NTCFailed);
		return;
		}
	//手电温度过高时对极亮进行限制
	IsForceLeaveTurbo=TempSchmittTrigger(IsForceLeaveTurbo,ForceOffTemp-10,ForceDisableTurboTemp-10);	//温度距离关机保护的间距不到10度，立即退出极亮
	IsDisableTurbo=TempSchmittTrigger(IsDisableTurbo,ForceDisableTurboTemp,ForceDisableTurboTemp-10); //温度达到关闭极亮档的阈值，关闭极亮
	//过热关机保护
	IsSystemShutDown=TempSchmittTrigger(IsSystemShutDown,ForceOffTemp,ConstantTemperature-10);
  if(IsSystemShutDown)ReportError(Fault_OverHeat); //报故障
	else if(ErrCode==Fault_OverHeat)ClearError(); //消除掉当前错误
	//PI环使能控制
	if(!CurrentMode->IsNeedStepDown)IsTempLIMActive=0; //当前挡位不需要降档
	else //使用施密特函数决定温控是否激活
		{
		ThermalStatus=TempSchmittTrigger(IsTempLIMActive,QueryConstantTemp(),ReleaseTemperature); //获取施密特触发器的结果
		if(ThermalStatus)IsTempLIMActive=1;//施密特函数要求激活温控，立即激活
		else if(!ThermalStatus&&!TempProtBuf&&TempIntegral<0)IsTempLIMActive=0; //施密特函数要求关闭温控，等待比例缓存为0解除限流后关闭
		}
	}	
