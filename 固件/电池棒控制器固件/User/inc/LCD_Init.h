#ifndef _LCINIT_
#define _LCINIT_

//�ڲ�����
#include <stdbool.h>

//���Ź����Զ�����
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


#define USE_HORIZONTAL 2  //���ú�������������ʾ 0��1Ϊ���� 2��3Ϊ����

//LCD�ֱ�������
#define LCD_W 160
#define LCD_H 80

//��������
#define LCD_EnableBackLight() GPIO_ClearOutBits(LCD_BLEN_IOG,LCD_BLEN_IOP) //BLENΪ����Ч�����0��������	

//�ⲿ��ʾ��������
extern LCDDisplayDirDef Direction;
extern volatile bool IsPDMATranferDone;
	
//�Ĵ�����������
void LCD_DeInit(void); //���ܲ���λLCD
void LCD_WR_DATA8(unsigned char dat); //дһ���ֽ�
void LCD_WR_DATA(unsigned short dat);//д�������ֽ�
void LCD_WR_REG(unsigned char dat);//д��һ��ָ��
void LCD_Address_Set(unsigned short x1,unsigned short y1,unsigned short x2,unsigned short y2);//�������꺯��
void LCD_Init(void);//LCD��ʼ��
void LCD_HardwareInit(void); //LCDӲ����ʼ��

#endif
