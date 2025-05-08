#ifndef _PMBUS_
#define _PMBUS_

//include
#include "I2C.h"
#include <stdbool.h>

//struct
typedef struct
 {
 char VRMAddress;//供电芯片地址
 char PageNumber;//目标的供电芯片页面
 char PageCommandRequired;//是否需要执行page切换，0为不需要
 char EXTCommandCode;//拓展命令代码
 char EXTRegCode;//拓展寄存器代码
 unsigned int *DataIO;//32bit数据输入输出
 char IsWrite;//是读取还是写入
 }MFREXTRWCFG;

//func
bool PMBUS_SendCommand(char Addr,char Command);//PMBUS发送无数据的命令
bool PMBUS_ByteReadWrite(bool IsWrite,unsigned char *Byte,char Addr,char Command);//单字节读写
bool PMBUS_WordReadWrite(bool MSBFirst,bool IsWrite,unsigned int *Word,char Addr,char Command);//双字节读写(word)
bool PMBUS_GetBlockWriteCount(int *Size,char Addr,char Command);//进行块写入操作前获取数据量
bool PMBUS_BlockReadWrite(bool IsWrite,unsigned char *Buf,char Addr,char Command);//数据块读写
bool ISL_ReadWriteMFRReg(MFREXTRWCFG *CFGSTR);//Renesas PMIC的MFR Reg专用读写函数
double PMBUS_2NPowCalc(int Exp);//进行PMBUS 2^N指数的换算
float PMBUS_SLinearToFloat(unsigned int SIN);//将SLINEAR16格式换算为浮点数(IEEE754半精度浮点)
 
#endif
