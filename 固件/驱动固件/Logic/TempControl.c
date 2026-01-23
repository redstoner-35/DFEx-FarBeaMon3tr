/****************************************************************************/
/** \file TempControl.c
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个文件为顶层应用层逻辑文件。负责实现系统的温度管理并在合适的
条件下限制输出功率以及强制关机，以保护系统免受过度高温影响。

**	
				2025年12月28日 12:42 1.新增在POWER模式下在电池即将耗尽，跳转到高亮继续
				                       运作前，逐步降低系统的外壳温度至高亮恒温值以避
															 切换到高亮后，系统瞬间因高温差大幅度降档导致亮
															 度瞬间跳楼一下影响用户使用体验。
														 2.修复积分器快速积分逻辑不正确，并且移除在温度误
														   差大于4度时快速升温的逻辑，以修复系统恒定亮度时
															 不稳定的问题。
														 3.针对主动散热DLC优化温控控制逻辑的参数，解决极亮
															 挡位下温度控制下冲严重的bug。
														 4.优化系统在挡位交接阶段重新计算电流值的配置使得
														   极亮在跳档至高亮时工作更稳定。
														 
				2025年12月26日 10:05 针对新的远光狗模式挡位调整温控计算暂停逻辑，以便于
														 优化系统逻辑。
														 
				2025年12月20日 Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "ADCCfg.h"
#include "LEDMgmt.h"
#include "delay.h"
#include "ModeControl.h"
#include "TempControl.h"
#include "BattDisplay.h"
#include "OutputChannel.h"
#include "TurboICCMAX.h"
#include "PWMCfg.h"
#include "LowVoltProt.h"
#include "SelfTest.h"
#include "FastOp.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros - for Parameter Definition
****************************************************************************/

//PI环参数和最小电流限制
#define ILEDRecoveryTime 90 //使用积分器缓慢升档的判断时长，如果积分器持续累加到这个时长，则执行一次调节(单位秒)
#define SlowStepDownTime 60 //使用积分器缓慢降档的判断时长，如果积分器持续累加到这个时长，则执行一次调节(单位秒)
#define IntegralCurrentTrimValue 2500 //积分器针对输出的电流修调的最大值(mA)
#define IntegralFactor 12 //积分系数(每单位=1/8秒，越大时间常数越高，6=每分钟进行40mA的调整)
#define ILEDStepDown 1500 //降档系统所能达到的最低电流(mA)
#define BatteryDynamicTurboDegFactor 20 //极亮模式下动态调节恒温温度实现无缝过渡的系数，单位(mV)系数越小，温度下降速度越快 

//常亮电流配置
#define ThermalFoldbackILEDDegVal 5700 //触发温度回折保护之后，降档系统立即减少的电流量(mA)
#define ILEDConstantGlowMin 8500 //降档系统内的低温温控的常亮电流设置(mA)
#define ILEDConstantGlowMinTurbo 11000 //降档系统内的极亮温控的常亮电流设置(mA)
#define ILEDConstantGlowMinECOTurbo 9000 //降档系统内的极亮温控（ECO模式）的常亮电流设置(mA)

//温度配置
#define ForceOffTemp 80 //过热关机温度
#define ForceDisableTurboTemp 65 //超过此温度无法进入极亮
#define TurboConstantTemperature 58 //极亮挡位的PID维持温度
#define ECOTurboConstantTemperature 52 //ECO模式下极亮挡位的PID维持温度
#define ConstantTemperature 50 //非极亮挡位温控启动后维持的温度
#define ReleaseTemperature 43 //温控释放的温度

/****************************************************************************/
/*	Local pre-processor symbols/macros for Parameter Processing and Checking
****************************************************************************/ 
#define MinumumILED CalcIREFValue(ILEDStepDown)		//最小LED电流计算
#define LeaveTurboTemperature ForceOffTemp-10   	//退出极亮温度计算
#define TurboTempDegEndTemp (ConstantTemperature+2)                  //计算极亮接近退出的温度
#define TurboVoltDeg (TurboConstantTemperature-TurboTempDegEndTemp)  //计算极亮到常亮的温度差
#define TurboTempDegEnd TurboOFFVoltage+(BatteryDynamicTurboDegFactor*TurboVoltDeg) //极亮不衰减性能的电池电压

/*   积分器满量程自动定义，切勿修改！    */
#define IntegrateFullScale IntegralCurrentTrimValue*IntegralFactor

#if (IntegrateFullScale > 32760)

#error "Error 001:Invalid Integral Configuration,Trim Value or time-factor out of range!"

#endif

#if (IntegrateFullScale <= 0)

#error "Error 002:Invalid Integral Configuration,Trim Value or time-factor must not be zero or less than zero!"

#endif

/*	温控数值监测，切勿修改！    	*/
#if (ForceOffTemp > 85)
#error "Error 003:Emergency Shutdown Temperature must not exceeded 85 Celsius!"
#endif

#if ((ForceOffTemp-15) < ForceDisableTurboTemp)
#error "Error 004:Force Disble Turbo Temperature must less than Emergency Shutdown Temperature for at least 15 Celsius!"
#endif

#if (ForceOffTemp < (TurboConstantTemperature+8))
#error "Error 005:Force Disble Turbo Temperature must higher than Constant Temperature of Turbo Mode for at least 8 Celsius!"
#endif

#if (TurboConstantTemperature <= ConstantTemperature)
#error "Error 006:Constant Temperature of Turbo Mode must lagger than Constant Temperature of other mode!"
#endif

#if (ConstantTemperature < (ReleaseTemperature+5))
#error "Error 007:Constant Temperature of other mode must lagger than Thermal Control Release Temp for 5 Celsius!"
#endif

#if (ReleaseTemperature < 38)
#error "Error 008:Thermal Control Release Temp is too low and will not release at summer!"
#elif (ReleaseTemperature < 41)
#warning "Warning 001:Thermal Control Release Temp is too low and might not be able to release at summer."
#endif

/****************************************************************************/
/*	Local variable and Flag definitions('static')
****************************************************************************/
static xdata int TempIntegral;
static xdata int TempProtBuf;
static xdata unsigned char StepDownTIM;  //降档显示计时
static xdata unsigned char StepUpLockTIM; //计时器

static bit IsNearThermalFoldBack; //标记位，是否接近于退出极亮温度
static bit IsThermalStepDown; //标记位，是否降档
static bit IsTempLIMActive;  //温控是否已经启动
static bit IsSystemShutDown; //是否触发温控强制关机

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/
bit IsDisableTurbo;  //禁止再度进入到极亮档
bit IsForceLeaveTurbo; //是否强制离开极亮档
bit IsPauseThermalCalc=0; //是否暂停温控计算

/****************************************************************************/
/*	Function implementation - local('static')
****************************************************************************/

//负责温度使能控制的施密特触发实现函数（滞回控制）
static bit TempSchmittTrigger(bit ValueIN,char HighThreshold,char LowThreshold)	
	{
	if(Data.Systemp>HighThreshold)return 1;
	if(Data.Systemp<LowThreshold)return 0;
	//数值保持，没有改变
	return ValueIN;
	}

//获取温控环路的恒温值
static int QueryConstantTemp(void)	
	{
	int Buf;
	if(CurrentMode->ModeIdx==Mode_Turbo)
		{
		//POWER模式下极亮的时候使用更高的温控拉长降档时间
		if(IsPowerModeEnabled)
			{
			//Power模式下为了实现和高亮挡位的温度无缝过渡，在电池电压接近极亮关断电压时，逐步降档
		  if(CellVoltage>=TurboTempDegEnd)Buf=TurboConstantTemperature;
		  else
				{
				//降档温度按照极亮关闭电压往上**mV（由温控参数定义）每℃的斜率，逐步从高温过渡到低温
				Buf=(CellVoltage-TurboOFFVoltage)/BatteryDynamicTurboDegFactor;
				if(IsNegative16(Buf))Buf=0;
				Buf+=TurboTempDegEndTemp;
				}
			return Buf;
			}
		//ECO模式使用52度温控
		else return ECOTurboConstantTemperature;
		}
	//正常使用其余挡位的温控
  return ConstantTemperature;
	}

//温控系统中积分追踪温度变化实现恒亮的处理
static void ThermalIntegralHandler(bool IsStepDown,bool IsEnableFastAdj,int Err)
	{
	//条件定义，如果积分值小于上限且系统需要快速调整，则令积分器以和温度挂钩的可变速率工作
	#define IsEnableQuickItg (abs(TempIntegral)<(IntegrateFullScale-Err)&&IsEnableFastAdj)
	//进行积分器本次调整值的计算
	if(IsEnableQuickItg)
		{
		if(IsStepDown&&IsNearThermalFoldBack)Err+=2; //系统严重过热，更进一步增加速度
		Err<<=1;      															 //快速调整开启,令调整值=温差*1
		}
	else Err=1;  											//快速调整关闭，误差值=1，确保积分器正常响应
  //应用积分数值到积分缓存
	if(IsStepDown)TempIntegral+=Err;
	else TempIntegral-=Err;            //降档模式则增加Err值，升档模式则减少Err值
	#undef IsEnableQuickItg            //这个宏定义只是该函数的局部定义，需要在函数末尾禁用掉避免后续意外使用到导致问题
	}
	
//在积分缓慢调整模式下，如果积分器发生溢出，则将积分器对应的调整量映射到比例项内
static void ThermalIntegralCommitToProtHandler(void)	
	{
	//当前积分器累计的参数小于复位时间，退出
	if(abs(TempIntegral)<(ILEDRecoveryTime*8))return;
	//将积分器内累积的变化成比例应用到比例项并清零积分器
	TempProtBuf+=TempIntegral/IntegralFactor;
	TempIntegral=0;						
	}
	
/********************************************************************************/
/* Global Function implementation - PI Controller Init and Result Export
*********************************************************************************/		
	
//换挡的时候根据当前恒温的电流重新计算PI值使得挡位电流同步
void RecalcPILoop(void)	
	{
	//目标挡位不需要计算,复位比例缓存
	if(!CurrentMode->IsNeedStepDown)TempProtBuf=0;
	//需要复位，执行对应处理
	else
		{	
		//判断系统当前执行的温控参数的限流结果是否小于当前运行的电流，如果小于，则比例缓存=0
		if(QuerySystemFullScaleCurrent()<=ThermalILIMCalc())TempProtBuf=0;
		//否则说明系统处于深度温控状态，此时则按照新档位电流值-当前限流值更新比例缓存使得系统亮度保持一致
		else TempProtBuf=QuerySystemFullScaleCurrent()-ThermalILIMCalc();
	  //不允许比例缓存小于0
		if(IsNegative16(TempProtBuf))TempProtBuf=0; 
		}
	//清除积分器缓存
	TempIntegral=0;
	}
	
//执行温控计算并根据缓存输出当前的LED限流值
int ThermalILIMCalc(void)
	{
	int result;
	//判断温控是否需要进行计算
	if(!IsTempLIMActive)
		{
		result=Current; 				//温控被关闭，电流限制进来多少返回去多少
		IsThermalStepDown=0;  	//指示温控已被关闭
		}
	//开始温控计算
	else
		{
		result=TempProtBuf+(TempIntegral/IntegralFactor); //根据缓存计算结果
		if(IsNegative16(result))result=0; //不允许负值出现
		result=Current-result; //计算限流值结果
		if(result<MinumumILED) //已经调到底了，禁止PID继续累加
			{
		  TempProtBuf=Current-MinumumILED; //将比例输出结果限幅为最小电流
		  TempIntegral=0;
		  result=MinumumILED; //电流限制不允许小于最低电流
			}
    //判断温控是否已经触发			
		if(result<(Current-CalcIREFValue(500)))IsThermalStepDown=1;	//温控已经让输出电流下调500mA，提示温控触发
		}
	//返回结果	                               
	return result; 
	}
	
//显示温度控制启动
bit ShowThermalStepDown(void)	
	{
	StepDownReasonDef Reason;
	//判断系统是否在降档
	if(VshowFSMState!=BattVdis_Waiting)Reason=StepDown_OFF; //当前处于电量显示状态不允许打断
	else if(IsThermalStepDown)Reason=StepDown_Thermal; //温控降档触发
	else Reason=QuerySystemTurboILIMState(); //其余情况，根据状态显示温控降档状态
	//进行降档处理
  switch(Reason)		
		{
		case StepDown_OFF:StepDownTIM=0;break; //提示未触发	
    case StepDown_ECOModeEnabled:
		case StepDown_BattAlert: //电池警报
		  //当计时器=13和10时多闪一次制造出两次闪烁(ECO模式下)
			if(StepDownTIM==13||(Reason==StepDown_ECOModeEnabled&&StepDownTIM==10))
				{
				StepDownTIM++;
				return 1;
				}
		case StepDown_Thermal: //过热
			StepDownTIM++;
		  //时间到，复位定时器重新开始计时
			if(StepDownTIM&0x10)
				{
				StepDownTIM=0;
				return 1;
				}
			break;
		}
	//返回0
	return 0;
	}	
	
/********************************************************************************/
/* Global Function implementation - PI Loop Controller Calculation And Thermal 
	 Management Logic Handler
*********************************************************************************/	
	
//温控PI环计算
void ThermalPILoopCalc(void)	
	{
	int ProtFact,Err,ConstantILED;
	//PI环关闭，复位数值
	if(!IsTempLIMActive)
		{
		IsNearThermalFoldBack=0;
		TempIntegral=0;
		TempProtBuf=0;
		IsThermalStepDown=0;
		}
	//进行PI环的计算(仅在系统需要的时候执行温控计算)
	else if(!IsPauseThermalCalc)
		{			
		//获取恒温温度值和恒亮电流
		if(CurrentMode->ModeIdx==Mode_Turbo)
			{
			if(IsPowerModeEnabled)ConstantILED=CalcIREFValue(ILEDConstantGlowMinTurbo); //POWER模式下使用较高的常亮电流
			else ConstantILED=CalcIREFValue(ILEDConstantGlowMinECOTurbo);
			}
		else ConstantILED=CalcIREFValue(ILEDConstantGlowMin);  //其他挡位，执行正常的常亮电流
		
		if(IsNearThermalFoldBack)ConstantILED-=CalcIREFValue(ThermalFoldbackILEDDegVal); //接近温度上限，立即将常亮电流下调阻止系统继续温升
		ProtFact=QueryConstantTemp(); //获取目标常亮温度
		//温度误差为正（温度大于恒温值）
		if(Data.Systemp>ProtFact)
			{		
			/**************************************************************
			安全保护机制：马上就要摸到强制掉极亮的温度了，立刻使能标志位下
			调常亮电流强制继续使用P项降档快速拉低电流，这样可以避免温度继
			续上去在正常情况下触发退出极亮的保护机制
			**************************************************************/
			if(Data.Systemp>(LeaveTurboTemperature-4))IsNearThermalFoldBack=1;
			
			Err=Data.Systemp-ProtFact;  //误差值等于目标温度-恒温温度

			//当前温度误差值大于2且电流位于常亮电流之上，执行比例项
			if(Err>2&&CurrentBuf>ConstantILED)	
				{
				//触发比例项降档，令升档计时器停7.5秒
				StepUpLockTIM=60; 
				//比例项(P)
				if(CurrentMode->ModeIdx==Mode_Turbo)
					{
					//极亮模式下开启ECO用最高斜率增加降档速度，否则使用低一档的斜率
					if(!IsPowerModeEnabled)ProtFact=CurrentBuf/1800;
					else ProtFact=CurrentBuf/2200;
					}
				else ProtFact=CurrentBuf/2300;
				//计算比例项结果
				if(IsNegative16(ProtFact))ProtFact=0;
				ProtFact++; 																					//保证比例项始终有1确保可以正确降档
				if(IsLargerThanThreeU16(Err))ProtFact*=(Err+2); 			//温度误差大于3摄氏度，扩张比例系数
				//提交比例项	至比例缓存区域
				TempProtBuf+=(ProtFact*Err);   
				}		
			else
				{
				//触发积分项的降档操作，升档之后温度过高导致降档再次发生，令升档操作停止3秒
				StepUpLockTIM=24; 
				ThermalIntegralHandler(true,IsLargerThanThreeU16(Err)||IsNearThermalFoldBack,Err); 
				ThermalIntegralCommitToProtHandler();
				//限制积分模式运行时，比例项最低不能超过最小LED电流
				ConstantILED=MinumumILED;
				}
			//限制比例项的参数（如果应用了最新的比例和积分项之后输出电流小于限制值，则直接让比例项减去对应的限制值）
			if(ThermalILIMCalc()<ConstantILED)TempProtBuf-=(ConstantILED-ThermalILIMCalc());
      if(IsNegative16(TempProtBuf))TempProtBuf=0;				
			}
		//温度小于恒温值（温度误差为负）
		else if(Data.Systemp<ProtFact)
			{
			//计算误差并判断电流是否进入积分缓调区域
			Err=ProtFact-Data.Systemp;								//误差等于目标温度值减去系统温度
			//比例项(P)
			if(StepUpLockTIM)StepUpLockTIM--; //当前触发降档还没达到快速升档的时间
			else
				{
				//电流达到回升限制值，开始使用积分器监测缓慢回升
				if(CurrentBuf>(ConstantILED-CalcIREFValue(200)))
					{
					//执行积分结果commit到比例项的处理
				  ThermalIntegralCommitToProtHandler();
					}
				//执行比例升温
				else
					{
					if(IsLargerThanOneU16(Err))TempProtBuf-=Err; //进行升档
					if(IsNegative16(TempProtBuf))TempProtBuf=0;
					}			
				//温度下来了很多，系统已经令电流回升到强制降额前的常亮电流，则复位标记位
				if(IsNearThermalFoldBack)
					{
					ConstantILED+=CalcIREFValue(ThermalFoldbackILEDDegVal); 	//把减掉的电流值加回来得到原来的目标常亮是多少电流
					if(CurrentBuf>ConstantILED)IsNearThermalFoldBack=0;  
					}
				//积分项(I)
				ThermalIntegralHandler(false,IsLargerThanThreeU16(Err),Err); //当上调计时器=0且温度误差大于3℃的时候，使能快速调整	
				}
			//比例项数值限幅(不能是负数)
			if(IsNegative16(TempProtBuf))TempProtBuf=0; 
			}
		}
	}

//温度管理函数
void ThermalMgmtProcess(void)
	{
	bit ThermalStatus;
	//温度传感器正常，执行温度控制
	if(Data.IsNTCOK)
		{
		//手电温度过高时对极亮进行限制
		IsForceLeaveTurbo=TempSchmittTrigger(IsForceLeaveTurbo,LeaveTurboTemperature,ForceDisableTurboTemp-10);	//温度距离关机保护的间距不到10度，立即退出极亮
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
			else if(!ThermalStatus&&!TempProtBuf&&IsNegative16(TempIntegral))IsTempLIMActive=0; //施密特函数要求关闭温控，等待比例缓存为0解除限流后关闭
			}
		}
	//温度传感器故障，返回错误
	else ReportError(Fault_NTCFailed);
	}	
/*********************************  End Of File  ************************************/
