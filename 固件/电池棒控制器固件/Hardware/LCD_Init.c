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

//����Gamma�Ĳ���
const u8 NegGammaParameter[16]=
	{
	0x0A,0x0D,0x08,0x07,
	0x0F,0x07,0x02,0x07,
	0x09,0x0F,0x25,0x35,
	0x00,0x09,0x04,0x10
	};
 	
//����ģʽ�Ĳ�������	
const u8 PartialModeParam[6]=
	{
	0x05,0x3C,0x3C,0x05,
	0x3C,0x3C 
	};	
	
//��������
void SPI_SetClockPrescaler(HT_SPI_TypeDef* SPIx, u32 SPI_ClockPrescaler);
	
//��ʾ�����趨������ȫ�ֱ���
volatile bool IsPDMATranferDone=false;
LCDDisplayDirDef Direction=LCDDisplay_Hori_Invert;

/******************************************************************************
      ����˵����LCD����Ӧ��Ӳ�����ź�SPI����ĳ�ʼ��
      ������ݣ���
      ����ֵ��  ��
******************************************************************************/
void LCD_HardwareInit(void)
	{
	SPI_InitTypeDef SPI_InitStructure;
	//������ĻSPI��GPIO
	GPIO_DirectionConfig(LCD_SCK_IOG,LCD_SCK_IOP,GPIO_DIR_OUT);
	GPIO_DirectionConfig(LCD_SDA_IOG,LCD_SDA_IOP,GPIO_DIR_OUT);
	GPIO_DirectionConfig(LCD_CS_IOG,LCD_CS_IOP,GPIO_DIR_OUT);
		
	AFIO_GPxConfig(LCD_SCK_IOB,LCD_SCK_IOP, AFIO_FUN_SPI);	
	AFIO_GPxConfig(LCD_SDA_IOB,LCD_SDA_IOP, AFIO_FUN_SPI);	
	AFIO_GPxConfig(LCD_CS_IOB,LCD_CS_IOP, AFIO_FUN_SPI);		
	//���üĴ���ѡ��͸�λ��
	AFIO_GPxConfig(LCD_RS_IOB,LCD_RS_IOP, AFIO_FUN_GPIO);
  GPIO_DirectionConfig(LCD_RS_IOG,LCD_RS_IOP,GPIO_DIR_OUT);//����Ϊ��� 
	GPIO_SetOutBits(LCD_RS_IOG,LCD_RS_IOP); //RS=1��Ĭ��д����
  
	AFIO_GPxConfig(LCD_RST_IOB,LCD_RST_IOP, AFIO_FUN_GPIO);	
	GPIO_DirectionConfig(LCD_RST_IOG,LCD_RST_IOP,GPIO_DIR_OUT);//����Ϊ��� 
	GPIO_ClearOutBits(LCD_RST_IOG,LCD_RST_IOP); //RST=0
	//���ñ���ѡ��
	AFIO_GPxConfig(LCD_BLEN_IOB,LCD_BLEN_IOP, AFIO_FUN_GPIO);	
	GPIO_DirectionConfig(LCD_BLEN_IOG,LCD_BLEN_IOP,GPIO_DIR_OUT);//����Ϊ���
  GPIO_DriveConfig(LCD_BLEN_IOG,LCD_BLEN_IOP,GPIO_DV_16MA); //16mA��������		
	GPIO_SetOutBits(LCD_BLEN_IOG,LCD_BLEN_IOP); //BLENΪ����Ч�����1�رձ���
		
	//����SPI
	SPI_InitStructure.SPI_Mode = SPI_MASTER;
	SPI_InitStructure.SPI_FIFO = SPI_FIFO_DISABLE;
	SPI_InitStructure.SPI_DataLength = SPI_DATALENGTH_8;
	SPI_InitStructure.SPI_SELMode = SPI_SEL_HARDWARE;
	SPI_InitStructure.SPI_SELPolarity = SPI_SELPOLARITY_LOW; //��ĻCSΪ����Ч
	SPI_InitStructure.SPI_FirstBit = SPI_FIRSTBIT_MSB; //����˳��Ϊ MSB D6 D5 D4 ... D1 LSB(�Ϳ�����Ҫ��һ��)
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_LOW;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_FIRST;
	SPI_InitStructure.SPI_RxFIFOTriggerLevel = 1;
	SPI_InitStructure.SPI_TxFIFOTriggerLevel = 0;
	SPI_InitStructure.SPI_ClockPrescaler = 2; //fSCLK=48MHz/2=24MHz(��Ȼ�ֲ�����˵ST7735SҪ��дʱ��Ƶ��С��1/66nS��15.15MHZ������24MHzҲ�����Ǿ����������)
	SPI_Init(HT_SPI0, &SPI_InitStructure);
	SPI_SELOutputCmd(HT_SPI0, ENABLE);
	SPI_Cmd(HT_SPI0, ENABLE);//����CS���Ȼ������SPI
	//����PDMA�ж�
	PDMA_ClearFlag(PDMA_SPI0_TX,PDMA_FLAG_TC); //���Flag
	PDMA_IntConfig(PDMA_SPI0_TX,PDMA_INT_GE|PDMA_INT_TC,ENABLE); //ʹ���ж�
	NVIC_EnableIRQ(PDMACH0_1_IRQn);//����PDMA 0-1�ж�
	}

/******************************************************************************
      ����˵����LCDд������(8bit�ֳ�)
      ������ݣ�dat д�������
      ����ֵ��  ��
******************************************************************************/
void LCD_WR_DATA8(u8 dat)
	{
	SPI_SendData(HT_SPI0,dat);
	while(HT_SPI0->SR&0x100); //�ȴ�SPI��ɲ���
	}	
	
/******************************************************************************
      ����˵�������ض�����³��ܲ���λLCD
      ������ݣ���
      ����ֵ��  ��
******************************************************************************/	
void LCD_DeInit(void)
	{
	GPIO_SetOutBits(LCD_BLEN_IOG,LCD_BLEN_IOP); //BLENΪ����Ч�����1�رձ���
	GPIO_ClearOutBits(LCD_RST_IOG,LCD_RST_IOP);
	}	
	
/******************************************************************************
      ����˵����LCDд������(16bit�ֳ�)
      ������ݣ�dat д�������
      ����ֵ��  ��
******************************************************************************/
void LCD_WR_DATA(u16 dat)
	{
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16); //����Ϊ16bit���ݳ���
	SPI_SendData(HT_SPI0,dat);
	while(HT_SPI0->SR&0x100); //�ȴ�SPI��ɲ���
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8); //��������Ϊ8bit���ݳ���
	}	

/******************************************************************************
      ����˵����LCDд������
      ������ݣ�dat д�������
      ����ֵ��  ��
******************************************************************************/
void LCD_WR_REG(u8 dat)
{
	GPIO_ClearOutBits(LCD_RS_IOG,LCD_RS_IOP); //RS=0д����
	SPI_SendData(HT_SPI0,dat);
	while(HT_SPI0->SR&0x100); //�ȴ�SPI��ɲ���
	GPIO_SetOutBits(LCD_RS_IOG,LCD_RS_IOP); //RS=1��Ĭ��д����
}

/******************************************************************************
      ����˵����������ʼ�ͽ�����ַ
      ������ݣ�x1,x2 �����е���ʼ�ͽ�����ַ
                y1,y2 �����е���ʼ�ͽ�����ַ
      ����ֵ��  ��
******************************************************************************/
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2)
{	
	//�����е�ַ��������(0x2A)
	LCD_WR_REG(0x2a);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16); //����Ϊ16bit���ݳ���
	//����XSTART[15:0]
	SPI_SendData(HT_SPI0,x1+1);
	while(HT_SPI0->SR&0x100); //�ȴ�SPI��ɲ���
	//����XEND[15:0]
	SPI_SendData(HT_SPI0,x2+1);
	while(HT_SPI0->SR&0x100); //�ȴ�SPI��ɲ���
	//�����е�ַ��������(0x2A)
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8); //��������Ϊ8bit���ݳ���
	LCD_WR_REG(0x2b);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16); //����Ϊ16bit���ݳ���
	//����YSTART[15:0]
	SPI_SendData(HT_SPI0,y1+26);
	while(HT_SPI0->SR&0x100); //�ȴ�SPI��ɲ���
	//����YEND[15:0]
	SPI_SendData(HT_SPI0,y2+26);
	while(HT_SPI0->SR&0x100); //�ȴ�SPI��ɲ���
  //��ɵ�ַ���ͣ��������û�8bitģʽ�󷢳�GRAMдָ���ʼ������ʾ����
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8);
	LCD_WR_REG(0x2c);//������д
}
/******************************************************************************
      ����˵����LCD����Ӧ�ļĴ�������ĳ�ʼ��
      ������ݣ���
      ����ֵ��  ��
******************************************************************************/
void LCD_Init(void)
{
  u8 Dir,i;
	//ѡ���ĸ�����
	switch(Direction)
		{
		case LCDDisplay_Vert_Normal:Dir=0x08;break;
		case LCDDisplay_Vert_Invert:Dir=0xC8;break;
		case LCDDisplay_Hori_Normal:Dir=0x78;break;
		case LCDDisplay_Hori_Invert:Dir=0xA8;break;
		}
	//����100mS�ĸ�λ����
	GPIO_SetOutBits(LCD_BLEN_IOG,LCD_BLEN_IOP); //BLENΪ����Ч�����1�رձ���
	GPIO_ClearOutBits(LCD_RST_IOG,LCD_RST_IOP);
	delay_ms(100);
	GPIO_SetOutBits(LCD_RST_IOG,LCD_RST_IOP);
	delay_ms(100);
	//���г�ʼ������
	SPI_SetClockPrescaler(HT_SPI0,8); //��ʼ���׶ν�SPIƵ������Ϊ6MHz�������ʼ���쳣
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
	//����Gamma
	LCD_WR_REG(0xE0);     //positive gamma
  for(i=0;i<sizeof(PosGammaParameter);i++)LCD_WR_DATA8(PosGammaParameter[i]);
	LCD_WR_REG(0xE1);     //negative gamma
	for(i=0;i<sizeof(NegGammaParameter);i++)LCD_WR_DATA8(NegGammaParameter[i]);
		 
	LCD_WR_REG(0xFC);    
	LCD_WR_DATA8(0x80);  
		
	LCD_WR_REG(0x3A);     
	LCD_WR_DATA8(0x05);   
	LCD_WR_REG(0x36);
	//��ʾ��������
	LCD_WR_DATA8(Dir);
	LCD_WR_REG(0x21);     //Display inversion
	
	//��ʼ��������������ʾ����ָ��
	LCD_WR_REG(0x29);     //Display on
	SPI_SetClockPrescaler(HT_SPI0,2); //��ʼ�������󣬽�SPIƵ������Ϊ24MHz
	}
