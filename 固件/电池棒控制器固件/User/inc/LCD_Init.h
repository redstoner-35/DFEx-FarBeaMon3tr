#ifndef _LCINIT_
#define _LCINIT_

//内部包含
#include <stdbool.h>

//引脚功能自动定义
#define LCD_SCK_IOB STRCAT2(GPIO_P,LCD_SCK_IOBank)
#define LCD_SCK_IOG STRCAT2(HT_GPIO,LCD_SCK_IOBank)
#define LCD_SCK_IOP STRCAT2(GPIO_PIN_,LCD_SCK_IOPinNum) 

#define LCD_SDA_IOB STRCAT2(GPIO_P,LCD_SDA_IOBank)
#define LCD_SDA_IOG STRCAT2(HT_GPIO,LCD_SDA_IOBank)
#define LCD_SDA_IOP STRCAT2(GPIO_PIN_,LCD_SDA_IOPinNum) 

#define LCD_CS_IOB STRCAT2(GPIO_P,LCD_CS_IOBank)
#define LCD_CS_IOG STRCAT2(HT_GPIO,LCD_CS_IOBank)
#define LCD_CS_IOP STRCAT2(GPIO_PIN_,LCD_CS_IOPinNum) 

#define LCD_RS_IOB STRCAT2(GPIO_P,LCD_RS_IOBank)
#define LCD_RS_IOG STRCAT2(HT_GPIO,LCD_RS_IOBank)
#define LCD_RS_IOP STRCAT2(GPIO_PIN_,LCD_RS_IOPinNum) 

#define LCD_RST_IOB STRCAT2(GPIO_P,LCD_RST_IOBank)
#define LCD_RST_IOG STRCAT2(HT_GPIO,LCD_RST_IOBank)
#define LCD_RST_IOP STRCAT2(GPIO_PIN_,LCD_RST_IOPinNum) 

#define LCD_BLEN_IOB STRCAT2(GPIO_P,LCD_BLEN_IOBank)
#define LCD_BLEN_IOG STRCAT2(HT_GPIO,LCD_BLEN_IOBank)
#define LCD_BLEN_IOP STRCAT2(GPIO_PIN_,LCD_BLEN_IOPinNum) 


typedef enum
	{
	LCDDisplay_Vert_Normal=0,
	LCDDisplay_Vert_Invert=1,
	LCDDisplay_Hori_Normal=2,
	LCDDisplay_Hori_Invert=3
	}LCDDisplayDirDef;


#define USE_HORIZONTAL 2  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

//LCD分辨率设置
#define LCD_W 160
#define LCD_H 80

//操作处理
#define LCD_EnableBackLight() GPIO_ClearOutBits(LCD_BLEN_IOG,LCD_BLEN_IOP) //BLEN为低有效，输出0启动背光	

//外部显示方向配置
extern LCDDisplayDirDef Direction;
extern volatile bool IsPDMATranferDone;
	
//寄存器操作函数
void LCD_DeInit(void); //除能并复位LCD
void LCD_WR_DATA8(unsigned char dat); //写一个字节
void LCD_WR_DATA(unsigned short dat);//写入两个字节
void LCD_WR_REG(unsigned char dat);//写入一个指令
void LCD_Address_Set(unsigned short x1,unsigned short y1,unsigned short x2,unsigned short y2);//设置坐标函数
void LCD_Init(void);//LCD初始化
void LCD_HardwareInit(void); //LCD硬件初始化

#endif
