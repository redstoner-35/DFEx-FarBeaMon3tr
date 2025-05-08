#ifndef _PinDefs_
#define _PinDefs_

/***************************************************
整个工程所有的外设的引脚定义都在这里了，你可以按照
需要去修改。但是要注意，有些引脚是不可以修改的因为是
外设的输出引脚。
******* 目前已使用引脚  *******
PA：0 1 2 9 12 13 14
PB：0 1 2 3 4 8

******* 不可修改的引脚 *******
PA9：按键所在引脚，同时触发boot下载模式方便固件更新
PA12-13：预留给SWDIO debug port
PA14：预留给MCTM的PWM output channel#0
PB8：预留给MCTM的PWM output channel#3
***************************************************/

//ADC输入
#define ISenseOut_IOPN 2  //电流监测反馈输入
#define ISenseREF_IOPN 1  //电流监测基准输入
#define TempVBatt_IOPN 0  //温度和电池电压输入

//ADC切换引脚
#define TVSEL_IOBank B
#define TVSEL_IOPinNum 11 //ADC切换输入(PB11)

//按键
#define KeyUp_IOBank B
#define KeyUp_IOPN 12  //侧按向上按钮（PB12）

#define KeyDown_IOBank B
#define KeyDown_IOPN 13  //侧按向下按钮（PB13）

//电源管理部分控制MCU LDO自锁的引脚
#define LDO_EN_IOBank B
#define LDO_EN_IOPinNum 8 //LDOEN Pin=PB8

//I2C
#define IIC_SCL_IOBank A
#define IIC_SCL_IOPinNum 14 //SCL Pin=PA14

#define IIC_SDA_IOBank B
#define IIC_SDA_IOPinNum 14 //SDA Pin=PB14

//IP2366的INT
#define IP2366_INT_IOBank A
#define IP2366_INT_IOPinNum 15 //INT Pin=PA15

//IP2366的EN
#define IP2366_EN_IOBank B
#define IP2366_EN_IOPinNum 0 //EN=PB0

//LCD引脚
#define LCD_SCK_IOBank B
#define LCD_SCK_IOPinNum 3 //LCDSCK=PB3(SPI0_CLK)

#define LCD_SDA_IOBank B
#define LCD_SDA_IOPinNum 4 //LCDSDA=PB4(SPI0_MOSI)

#define LCD_CS_IOBank B
#define LCD_CS_IOPinNum 2 //LCDCS=PB2(SPI0_SEL)

#define LCD_RS_IOBank B
#define LCD_RS_IOPinNum 10 //LCDRS=PB10

#define LCD_RST_IOBank B
#define LCD_RST_IOPinNum 1 //LCDRST=PB1

#define LCD_BLEN_IOBank C
#define LCD_BLEN_IOPinNum 7 //LCDBLEN=PC7

#endif
