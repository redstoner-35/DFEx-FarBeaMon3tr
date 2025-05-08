#include "ht32.h"
#include "delay.h"
#include "Pindefs.h"
#include "LCD_Init.h"

const u8 PosGammaParameter[16]=
	{
	0x07,0x0E,0x08,0x07,
	0x10,0x07,0x02,0x07,
	0x09,0x0F,0x25,0x36,
	0x00,0x08,0x04,0x10
	};

//负向Gamma的参数
const u8 NegGammaParameter[16]=
	{
	0x0A,0x0D,0x08,0x07,
	0x0F,0x07,0x02,0x07,
	0x09,0x0F,0x25,0x35,
	0x00,0x09,0x04,0x10
	};
 	
//部分模式的参数配置	
const u8 PartialModeParam[6]=
	{
	0x05,0x3C,0x3C,0x05,
	0x3C,0x3C 
	};	
	
//声明函数
void SPI_SetClockPrescaler(HT_SPI_TypeDef* SPIx, u32 SPI_ClockPrescaler);
	
//显示方向设定和其他全局变量
volatile bool IsPDMATranferDone=false;
LCDDisplayDirDef Direction=LCDDisplay_Hori_Invert;

/******************************************************************************
      函数说明：LCD所对应的硬件引脚和SPI外设的初始化
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_HardwareInit(void)
	{
	SPI_InitTypeDef SPI_InitStructure;
	//配置屏幕SPI的GPIO
	GPIO_DirectionConfig(LCD_SCK_IOG,LCD_SCK_IOP,GPIO_DIR_OUT);
	GPIO_DirectionConfig(LCD_SDA_IOG,LCD_SDA_IOP,GPIO_DIR_OUT);
	GPIO_DirectionConfig(LCD_CS_IOG,LCD_CS_IOP,GPIO_DIR_OUT);
		
	AFIO_GPxConfig(LCD_SCK_IOB,LCD_SCK_IOP, AFIO_FUN_SPI);	
	AFIO_GPxConfig(LCD_SDA_IOB,LCD_SDA_IOP, AFIO_FUN_SPI);	
	AFIO_GPxConfig(LCD_CS_IOB,LCD_CS_IOP, AFIO_FUN_SPI);		
	//配置寄存器选择和复位脚
	AFIO_GPxConfig(LCD_RS_IOB,LCD_RS_IOP, AFIO_FUN_GPIO);
  GPIO_DirectionConfig(LCD_RS_IOG,LCD_RS_IOP,GPIO_DIR_OUT);//配置为输出 
	GPIO_SetOutBits(LCD_RS_IOG,LCD_RS_IOP); //RS=1，默认写数据
  
	AFIO_GPxConfig(LCD_RST_IOB,LCD_RST_IOP, AFIO_FUN_GPIO);	
	GPIO_DirectionConfig(LCD_RST_IOG,LCD_RST_IOP,GPIO_DIR_OUT);//配置为输出 
	GPIO_ClearOutBits(LCD_RST_IOG,LCD_RST_IOP); //RST=0
	//配置背光选择
	AFIO_GPxConfig(LCD_BLEN_IOB,LCD_BLEN_IOP, AFIO_FUN_GPIO);	
	GPIO_DirectionConfig(LCD_BLEN_IOG,LCD_BLEN_IOP,GPIO_DIR_OUT);//配置为输出
  GPIO_DriveConfig(LCD_BLEN_IOG,LCD_BLEN_IOP,GPIO_DV_16MA); //16mA驱动能力		
	GPIO_SetOutBits(LCD_BLEN_IOG,LCD_BLEN_IOP); //BLEN为低有效，输出1关闭背光
		
	//配置SPI
	SPI_InitStructure.SPI_Mode = SPI_MASTER;
	SPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	SPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_8;
	SPI_InitStructure.SPI_SELMode = SPI_SEL_HARDWARE;
	SPI_InitStructure.SPI_SELPolarity = SPI_SELPOLARITY_LOW; //屏幕CS为低有效
	SPI_InitStructure.SPI_FirstBit = SPI_FIRSTBIT_MSB; //发送顺序为 MSB D6 D5 D4 ... D1 LSB(和控制器要求一致)
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_LOW;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_FIRST;
	SPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
	SPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	SPI_InitStructure.SPI_ClockPrescaler = 2; //fSCLK=48MHz/2=24MHz(虽然手册里面说ST7735S要求写时钟频率小于1/66nS≈15.15MHZ，但是24MHz也能用那就往死里干了)
	SPI_Init(HT_SPI0, &SPI_InitStructure);
	SPI_SELOutputCmd(HT_SPI0, ENABLE);
	SPI_Cmd(HT_SPI0, ENABLE);//启用CS输出然后启用SPI
	//配置PDMA中断
	PDMA_ClearFlag(PDMA_SPI0_TX,PDMA_FLAG_TC); //清除Flag
	PDMA_IntConfig(PDMA_SPI0_TX,PDMA_INT_GE|PDMA_INT_TC,ENABLE); //使能中断
	NVIC_EnableIRQ(PDMACH0_1_IRQn);//启用PDMA 0-1中断
	}

/******************************************************************************
      函数说明：LCD写入数据(8bit字长)
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA8(u8 dat)
	{
	SPI_SendData(HT_SPI0,dat);
	while(HT_SPI0->SR&0x100); //等待SPI完成操作
	}	
	
/******************************************************************************
      函数说明：在特定情况下除能并复位LCD
      入口数据：无
      返回值：  无
******************************************************************************/	
void LCD_DeInit(void)
	{
	GPIO_SetOutBits(LCD_BLEN_IOG,LCD_BLEN_IOP); //BLEN为低有效，输出1关闭背光
	GPIO_ClearOutBits(LCD_RST_IOG,LCD_RST_IOP);
	}	
	
/******************************************************************************
      函数说明：LCD写入数据(16bit字长)
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(u16 dat)
	{
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16); //配置为16bit数据长度
	SPI_SendData(HT_SPI0,dat);
	while(HT_SPI0->SR&0x100); //等待SPI完成操作
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8); //重新配置为8bit数据长度
	}	

/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(u8 dat)
{
	GPIO_ClearOutBits(LCD_RS_IOG,LCD_RS_IOP); //RS=0写命令
	SPI_SendData(HT_SPI0,dat);
	while(HT_SPI0->SR&0x100); //等待SPI完成操作
	GPIO_SetOutBits(LCD_RS_IOG,LCD_RS_IOP); //RS=1，默认写数据
}

/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2)
{	
	//发送列地址设置命令(0x2A)
	LCD_WR_REG(0x2a);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16); //配置为16bit数据长度
	//发送XSTART[15:0]
	SPI_SendData(HT_SPI0,x1+1);
	while(HT_SPI0->SR&0x100); //等待SPI完成操作
	//发送XEND[15:0]
	SPI_SendData(HT_SPI0,x2+1);
	while(HT_SPI0->SR&0x100); //等待SPI完成操作
	//发送行地址设置命令(0x2A)
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8); //重新配置为8bit数据长度
	LCD_WR_REG(0x2b);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16); //配置为16bit数据长度
	//发送YSTART[15:0]
	SPI_SendData(HT_SPI0,y1+26);
	while(HT_SPI0->SR&0x100); //等待SPI完成操作
	//发送YEND[15:0]
	SPI_SendData(HT_SPI0,y2+26);
	while(HT_SPI0->SR&0x100); //等待SPI完成操作
  //完成地址发送，重新配置回8bit模式后发出GRAM写指令，开始传送显示数据
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8);
	LCD_WR_REG(0x2c);//储存器写
}
/******************************************************************************
      函数说明：LCD所对应的寄存器层面的初始化
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_Init(void)
{
  u8 Dir,i;
	//选择哪个方向
	switch(Direction)
		{
		case LCDDisplay_Vert_Normal:Dir=0x08;break;
		case LCDDisplay_Vert_Invert:Dir=0xC8;break;
		case LCDDisplay_Hori_Normal:Dir=0x78;break;
		case LCDDisplay_Hori_Invert:Dir=0xA8;break;
		}
	//生成100mS的复位脉冲
	GPIO_SetOutBits(LCD_BLEN_IOG,LCD_BLEN_IOP); //BLEN为低有效，输出1关闭背光
	GPIO_ClearOutBits(LCD_RST_IOG,LCD_RST_IOP);
	delay_ms(100);
	GPIO_SetOutBits(LCD_RST_IOG,LCD_RST_IOP);
	delay_ms(100);
	//进行初始化序列
	SPI_SetClockPrescaler(HT_SPI0,8); //初始化阶段将SPI频率设置为6MHz，避免初始化异常
	LCD_WR_REG(0x11);     //Sleep out
	delay_ms(120);                //Delay 120ms
	LCD_WR_REG(0xB1);     //Normal mode
	LCD_WR_DATA8(0x05);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_REG(0xB2);     //Idle mode
	LCD_WR_DATA8(0x05);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_DATA8(0x3C);   
	LCD_WR_REG(0xB3);     //Partial mode
	for(i=0;i<sizeof(PartialModeParam);i++)LCD_WR_DATA8(PartialModeParam[i]);
		
	LCD_WR_REG(0xB4);     //Dot inversion
	LCD_WR_DATA8(0x03);   
	LCD_WR_REG(0xC0);     //AVDD GVDD
	LCD_WR_DATA8(0xAB);   
	LCD_WR_DATA8(0x0B);   
	LCD_WR_DATA8(0x04);   
	LCD_WR_REG(0xC1);     //VGH VGL
	LCD_WR_DATA8(0xC5);   //C0
	LCD_WR_REG(0xC2);     //Normal Mode
	LCD_WR_DATA8(0x0D);   
	LCD_WR_DATA8(0x00);   
	LCD_WR_REG(0xC3);     //Idle
	LCD_WR_DATA8(0x8D);   
	LCD_WR_DATA8(0x6A);   
	LCD_WR_REG(0xC4);     //Partial+Full
	LCD_WR_DATA8(0x8D);   
	LCD_WR_DATA8(0xEE);   
	LCD_WR_REG(0xC5);     //VCOM
	LCD_WR_DATA8(0x0F);   
	//设置Gamma
	LCD_WR_REG(0xE0);     //positive gamma
  for(i=0;i<sizeof(PosGammaParameter);i++)LCD_WR_DATA8(PosGammaParameter[i]);
	LCD_WR_REG(0xE1);     //negative gamma
	for(i=0;i<sizeof(NegGammaParameter);i++)LCD_WR_DATA8(NegGammaParameter[i]);
		 
	LCD_WR_REG(0xFC);    
	LCD_WR_DATA8(0x80);  
		
	LCD_WR_REG(0x3A);     
	LCD_WR_DATA8(0x05);   
	LCD_WR_REG(0x36);
	//显示方向设置
	LCD_WR_DATA8(Dir);
	LCD_WR_REG(0x21);     //Display inversion
	
	//初始化结束，发送显示启动指令
	LCD_WR_REG(0x29);     //Display on
	SPI_SetClockPrescaler(HT_SPI0,2); //初始化结束后，将SPI频率设置为24MHz
	}
