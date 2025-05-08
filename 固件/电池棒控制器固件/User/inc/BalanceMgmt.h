#ifndef _BALMGMT_
#define _BALMGMT_

#include <stdbool.h>

//外部参考
extern int BalanceForceEnableTIM; //强制启用均衡系统的变量，往该变量写大于0的值启用均衡器
extern bool EnableExtendedBal; //手动均衡启动标志位
extern bool BalanceState; //均衡器当前状态

//函数
void BalanceMgmt_Init(void); //配置均衡控制器
void Balance_ForceDiasble(void); //强制关闭均衡
void Balance_IOMgmt(void); //运行过程中控制均衡器启用的模块

#endif
