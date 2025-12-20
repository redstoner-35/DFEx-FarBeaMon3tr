/************************************************************************************/
/** \file SelfTest.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个头文件为系统顶层逻辑模块的声明文件。该模块声明了系统错误管理和自我状
态监视模块相关的函数以及错误报告系统的接口变量共其他模块使用。

**	History: Initial Release
**	
/************************************************************************************/
#ifndef _SelfTest_
#define _SelfTest_
/*************************************************************************************/
/*	Global type definitions('typedef')
**************************************************************************************/
typedef enum	//错误类型枚举
	{
	Fault_None,    //没有错误发生
	Fault_DCDCFailedToStart, //DCDC无法启动 ID:1
	Fault_DCDCShort, //DCDC输出短路  ID:2
	Fault_InputOVP, //输入过压保护 ID:3
	Fault_DCDCOpen,  //LED开路 ID:4
	Fault_NTCFailed, //NTC故障 ID:5
	Fault_OverHeat, //过热故障 ID:6
	Fault_DCDCPreChargeFailed, //DCDC预充系统故障 ID:7
	Fault_RampConfigError,     //系统无法找到无极调光配置 ID:8
	}FaultCodeDef;	

/************************************************************************************/
/* Extern Functions definition - Error Management and Severe level query */
/************************************************************************************/	
void ReportError(FaultCodeDef Code); 	//报告错误
void ClearError(void); 								//消除错误
bit IsErrorFatal(void);								//查询错误是否致命
	
/************************************************************************************/
/* Extern Functions definition - Visual(LED) based Error Reporting Logic Handler */
/************************************************************************************/		
void DisplayErrorTIMHandler(void); //显示错误时候用到的计时器处理
void DisplayErrorIDHandler(void); //根据错误ID进行显示的处理	

/************************************************************************************/
/* Extern Functions definition - Self monitoring logic */
/************************************************************************************/	
void OutputFaultDetect(void); 
	
/************************************************************************************/
/* Extern Flags and Variable definition */
/************************************************************************************/
extern xdata FaultCodeDef ErrCode; 	//错误代码
extern bit IsInputLimited; 					//输入限流激活标志位
	
#endif /* _SelfTest_ */

/*********************************  End Of File  ************************************/
