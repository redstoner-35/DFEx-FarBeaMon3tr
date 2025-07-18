#include <stdbool.h>

/*-------------------------------------------------------------
 This is an automatically generated file by NTC resistor LUT 
 generator. DO NOT EDIT UNLESS YOU FULLY UNDERSTAND WHAT THIS
 FILE ACTUALLY DOES!
 NTC PARAMETER:100.00KΩ @ 25℃ B4250
 Table temperature range:-24℃ to 100℃
 Total ROM space for table:378 Bytes
 Target MCU Architecture:8051 Based MCU
-------------------------------------------------------------*/

//温度反馈的偏移值（如果你发现温度不准，可以在这里对温度监测系统进行TRIM）
#define TemperatureReportOffset 0

code unsigned long NTCTableTop[54]={
1179691,1104854, 1035294,   			//-19 到 -17 摄氏度
970604, 910411, 854373, 802176,   //-16 到 -13 摄氏度
753532, 708176, 665863, 626371,   //-12 到 -9 摄氏度
589493, 555039, 522834, 492718,   //-8 到 -5 摄氏度
464541, 438167, 413468, 390328,   //-4 到 -1 摄氏度
368638, 348299, 329218, 311309,   //0 到 3 摄氏度
294493, 278697, 263852, 249895,   //4 到 7 摄氏度
236769, 224417, 212791, 201842,   //8 到 11 摄氏度
191528, 181807, 172643, 163999,   //12 到 15 摄氏度
155843, 148145, 140876, 134011,   //16 到 19 摄氏度
127523, 121390, 115591, 110105,   //20 到 23 摄氏度
104914, 100000, 95346, 90938,   //24 到 27 摄氏度
86762, 82802, 79048, 75487,   //28 到 31 摄氏度
72108, 68901, 65857  };

code unsigned int NTCTableBottom[61]={
62965, 60218, 57607, 55125,   //35 到 38 摄氏度
52765, 50520, 48384, 46350,   //39 到 42 摄氏度
44415, 42572, 40816, 39143,   //43 到 46 摄氏度
37548, 36027, 34577, 33194,   //47 到 50 摄氏度
31874, 30615, 29412, 28263,   //51 到 54 摄氏度
27166, 26118, 25116, 24159,   //55 到 58 摄氏度
23243, 22367, 21529, 20728,   //59 到 62 摄氏度
19960, 19226, 18522, 17848,   //63 到 66 摄氏度
17203, 16584, 15991, 15422,   //67 到 70 摄氏度
14877, 14354, 13853, 13371,   //71 到 74 摄氏度
12909, 12466, 12040, 11631,   //75 到 78 摄氏度
11238, 10860, 10497, 10148,   //79 到 82 摄氏度
9813, 9491, 9181, 8882,   //83 到 86 摄氏度
8595, 8319, 8053, 7797   //87 到 90 摄氏度
};

//NTC温度换算函数
//传入参数：NTC阻值(Ω),温度是否有效的bool指针输出
//返回参数：温度值(℃)
int CalcNTCTemp(bool *IsNTCOK,unsigned long NTCRes){
unsigned char i;
volatile unsigned long NTCTableValue;
//电阻值大于查找表阻值上限，温度异常
if(NTCRes>(unsigned long)1179691)
  {
  *IsNTCOK=false;
  return -19+TemperatureReportOffset;
  }
//电阻值小于查找表阻值的阻值下限，温度异常
if(NTCRes<(unsigned long)7797)
  {
  *IsNTCOK=false;
  return 100+TemperatureReportOffset;
  }
//温度正常，开始查表
*IsNTCOK=true;
if(NTCRes>(unsigned long)62965)for(i=0;i<54;i++)if(NTCTableTop[i]<=NTCRes)return (-19+TemperatureReportOffset)+i;
for(i=0;i<61;i++)
  {
  NTCTableValue=(unsigned long)NTCTableBottom[i];
  NTCTableValue&=0xFFFF;
  if(NTCTableValue<=NTCRes)return (TemperatureReportOffset+35)+i;
  }
//数值查找失败，返回错误值
*IsNTCOK=false;
return 0;
}

