#include "lcd.h"
#include "lcd_init.h"
#include "lcdfont.h"
#include "delay.h"


/******************************************************************************
      ����˵������ָ�����������ɫ
      ������ݣ�xsta,ysta   ��ʼ����
                xend,yend   ��ֹ����
								color       Ҫ������ɫ
      ����ֵ��  ��
******************************************************************************/
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)
{          
	u32 color1;
	PDMACH_InitTypeDef PDMAInitStr; 
	//������
	if(xsta>=xend)return;
	if(ysta>=yend)return;
	//����LCD��ַ
	color1=((u32)color)&0xFFFF;
	LCD_Address_Set(xsta,ysta,xend-1,yend-1);//������ʾ��Χ
	//������SPIΪ16bitģʽ
	SPI_PDMACmd(HT_SPI0,SPI_PDMAREQ_TX,ENABLE);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16); //����Ϊ16bit���ݳ���
	//��ʼ��PDMA
	PDMAInitStr.PDMACH_AdrMod=SRC_ADR_FIX | DST_ADR_FIX; //Դ��ַ�̶�
	PDMAInitStr.PDMACH_BlkCnt=(xend-xsta)*(yend-ysta);//ָ������������������Ϊ��Ҫ��������
	PDMAInitStr.PDMACH_BlkLen=1; //ÿ����1��Word��С
	PDMAInitStr.PDMACH_DataSize=WIDTH_16BIT;
	PDMAInitStr.PDMACH_DstAddr=(u32)(&HT_SPI0->DR);
	PDMAInitStr.PDMACH_Priority=M_PRIO; //128�����ݿ飬���С2�ֽ�(1Word),�����ȼ�
	PDMAInitStr.PDMACH_SrcAddr=(u32)(&color1);//ȡ��ɫ������Ϊ��ʼ��ַ
	PDMA_Config(PDMA_SPI0_TX,&PDMAInitStr);//��ʼ��TX DMA
  //�������䲢�ȴ��������
	IsPDMATranferDone=false;  //����flag
	PDMA_EnaCmd(PDMA_SPI0_TX,ENABLE);	//����SPI DMAͨ��
  while(!IsPDMATranferDone); //�ȴ��������
	//�ر�SPI 16bit Txģʽ
	while(HT_SPI0->SR&0x100); //�ȴ�SPI��ɲ���
	SPI_PDMACmd(HT_SPI0,SPI_PDMAREQ_TX,DISABLE);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8); //����Ϊ8bit���ݳ���	
}

/******************************************************************************
      ����˵������ָ��λ�û���
      ������ݣ�x,y ��������
                color �����ɫ
      ����ֵ��  ��
******************************************************************************/
void LCD_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_Address_Set(x,y,x,y);//���ù��λ�� 
	LCD_WR_DATA(color);
} 

/******************************************************************************
      ����˵������ˮƽ��(ʹ��DMA����)
      ������ݣ�x1,x2   x����ʼ�ͽ�������
                y   y����
                color   �ߵ���ɫ
      ����ֵ��  ��
******************************************************************************/
static void LCD_DrawHoriLine(u16 x1,u16 x2,u16 y,u16 color)
{
	u32 color1;
	u16 len;
	PDMACH_InitTypeDef PDMAInitStr; 
	//����LCD��ַ
	color1=((u32)color)&0xFFFF;
	LCD_Address_Set(x1,y,x2-1,y);//������ʾ��Χ
	//������SPIΪ16bitģʽ
	SPI_PDMACmd(HT_SPI0,SPI_PDMAREQ_TX,ENABLE);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16); //����Ϊ16bit���ݳ���
	//��ʼ��PDMA
	len=x1<x2?x2-x1:x1-x2;
	PDMAInitStr.PDMACH_AdrMod=SRC_ADR_FIX | DST_ADR_FIX; //Դ��ַ�̶�
	PDMAInitStr.PDMACH_BlkCnt=len;//ָ������������������=�߳�
	PDMAInitStr.PDMACH_BlkLen=1; //ÿ����1��Word��С
	PDMAInitStr.PDMACH_DataSize=WIDTH_16BIT;
	PDMAInitStr.PDMACH_DstAddr=(u32)(&HT_SPI0->DR);
	PDMAInitStr.PDMACH_Priority=M_PRIO; //128�����ݿ飬���С2�ֽ�(1Word),�����ȼ�
	PDMAInitStr.PDMACH_SrcAddr=(u32)(&color1);//ȡ��ɫ������Ϊ��ʼ��ַ
	PDMA_Config(PDMA_SPI0_TX,&PDMAInitStr);//��ʼ��TX DMA
  //�������䲢�ȴ��������
	IsPDMATranferDone=false;  //����flag
	PDMA_EnaCmd(PDMA_SPI0_TX,ENABLE);	//����SPI DMAͨ��
  while(!IsPDMATranferDone); //�ȴ��������
	//�ر�SPI 16bit Txģʽ
	while(HT_SPI0->SR&0x100); //�ȴ�SPI��ɲ���
	SPI_PDMACmd(HT_SPI0,SPI_PDMAREQ_TX,DISABLE);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8); //����Ϊ8bit���ݳ���	
}

/******************************************************************************
      ����˵��������ֱ��(ʹ��DMA����)
      ������ݣ�y1,y2   y����ʼ�ͽ�������
                x   x����
                color   �ߵ���ɫ
      ����ֵ��  ��
******************************************************************************/
static void LCD_DrawVertLine(u16 y1,u16 y2,u16 x,u16 color)
{
	u32 color1;
	u16 len;
	PDMACH_InitTypeDef PDMAInitStr; 
	//����LCD��ַ
	color1=((u32)color)&0xFFFF;
	LCD_Address_Set(x,y1,x,y2-1);//������ʾ��Χ
	//������SPIΪ16bitģʽ
	SPI_PDMACmd(HT_SPI0,SPI_PDMAREQ_TX,ENABLE);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16); //����Ϊ16bit���ݳ���
	//��ʼ��PDMA
	len=y1<y2?y2-y1:y1-y2;
	PDMAInitStr.PDMACH_AdrMod=SRC_ADR_FIX | DST_ADR_FIX; //Դ��ַ�̶�
	PDMAInitStr.PDMACH_BlkCnt=len;//ָ������������������=�߳�
	PDMAInitStr.PDMACH_BlkLen=1; //ÿ����1��Word��С
	PDMAInitStr.PDMACH_DataSize=WIDTH_16BIT;
	PDMAInitStr.PDMACH_DstAddr=(u32)(&HT_SPI0->DR);
	PDMAInitStr.PDMACH_Priority=M_PRIO; //128�����ݿ飬���С2�ֽ�(1Word),�����ȼ�
	PDMAInitStr.PDMACH_SrcAddr=(u32)(&color1);//ȡ��ɫ������Ϊ��ʼ��ַ
	PDMA_Config(PDMA_SPI0_TX,&PDMAInitStr);//��ʼ��TX DMA
  //�������䲢�ȴ��������
	IsPDMATranferDone=false;  //����flag
	PDMA_EnaCmd(PDMA_SPI0_TX,ENABLE);	//����SPI DMAͨ��
  while(!IsPDMATranferDone); //�ȴ��������
	//�ر�SPI 16bit Txģʽ
	while(HT_SPI0->SR&0x100); //�ȴ�SPI��ɲ���
	SPI_PDMACmd(HT_SPI0,SPI_PDMAREQ_TX,DISABLE);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8); //����Ϊ8bit���ݳ���	
}

/******************************************************************************
      ����˵��������
      ������ݣ�x1,y1   ��ʼ����
                x2,y2   ��ֹ����
                color   �ߵ���ɫ
      ����ֵ��  ��
******************************************************************************/
void LCD_DrawLine(u16 x1,u16 y1,u16 x2,u16 y2,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
  //ˮƽ�ߣ�ֱ��ʹ��PDMA����
	if(y1==y2)
		{
		LCD_DrawHoriLine(x1,x2,y1,color);
		return;
		}
	//��ֱ�ߣ�ֱ��ʹ��PDMA����
	if(x1==x2)
		{
		LCD_DrawVertLine(y1,y2,x1,color);
		return;
		}	
	//�������������PDMA���ٲ��˵ģ���������
	delta_x=x2-x1; 
	delta_y=y2-y1;  //������������ 
	uRow=x1;
	uCol=y1;  //�����������
	if(delta_x>0)incx=1; //���õ������� 
	else if (delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//ˮƽ�� 
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		LCD_DrawPoint(uRow,uCol,color);//����
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}


/******************************************************************************
      ����˵����������
      ������ݣ�x1,y1   ��ʼ����
                x2,y2   ��ֹ����
                color   ���ε���ɫ
      ����ֵ��  ��
******************************************************************************/
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
	{
	//�ϱ߿�
	LCD_DrawHoriLine(x1,x2,y1,color);
	//�±߿�
	LCD_DrawHoriLine(x1,x2,y2,color);
	//��߿�
	LCD_DrawVertLine(y1,y2,x1,color);
	//�ұ߿�
	LCD_DrawVertLine(y1,y2,x2,color);
	}

/******************************************************************************
      ����˵������Բ
      ������ݣ�x0,y0   Բ������
                r       �뾶
                color   Բ����ɫ
      ����ֵ��  ��
******************************************************************************/
void Draw_Circle(u16 x0,u16 y0,u8 r,u16 color)
{
	int a,b;
	a=0;b=r;	  
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a,color);             //3           
		LCD_DrawPoint(x0+b,y0-a,color);             //0           
		LCD_DrawPoint(x0-a,y0+b,color);             //1                
		LCD_DrawPoint(x0-a,y0-b,color);             //2             
		LCD_DrawPoint(x0+b,y0+a,color);             //4               
		LCD_DrawPoint(x0+a,y0-b,color);             //5
		LCD_DrawPoint(x0+a,y0+b,color);             //6 
		LCD_DrawPoint(x0-b,y0+a,color);             //7
		a++;
		if((a*a+b*b)>(r*r))//�ж�Ҫ���ĵ��Ƿ��Զ
		{
			b--;
		}
	}
}

/******************************************************************************
      ����˵������ʾ���ִ�
      ������ݣ�x,y��ʾ����
                *s Ҫ��ʾ�ĺ��ִ�
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ� ��ѡ 16 24 32
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowChinese(u16 x,u16 y,char *s,u16 fc,u16 bc,u8 mode)
{
	while(*s!=0)
	{
	LCD_ShowChinese12x12(x,y,s,fc,bc,12,mode);
	s+=2;
	x+=13;
	}
}

/******************************************************************************
      ����˵������ʾ����12x12����
      ������ݣ�x,y��ʾ����
                *s Ҫ��ʾ�ĺ���
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowChinese12x12(u16 x,u16 y,char *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 i,j,m=0;
	u16 k;
	u16 HZnum;//������Ŀ
	u16 TypefaceNum;//һ���ַ���ռ�ֽڴ�С
	u16 x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	                         
	HZnum=sizeof(tfont12)/sizeof(typFNT_GB12);	//ͳ�ƺ�����Ŀ
	for(k=0;k<HZnum;k++)if((tfont12[k].Index[0]==*(s))&&(tfont12[k].Index[1]==*(s+1))) 
		{ 		
		LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);	//���õ�ַ��Χ
		if(!mode)SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16); //����Ϊ16bit���ݳ���		
		//��ʼ��������
		for(i=0;i<TypefaceNum;i++)
			{
			//�ǵ��ӷ�ʽ��ֱ��д��������
			if(!mode)for(j=0;j<8;j++)
				{	
				SPI_SendData(HT_SPI0,tfont12[k].Msk[i]&(0x01<<j)?fc:bc);
				m++;
				while(HT_SPI0->SR&0x100); //�ȴ�SPI��ɲ���
				if(m%sizey)continue;
				m=0;
				break;
				}
			//���ӷ�ʽ,ʹ�û���ʵ��	
			else for(j=0;j<8;j++)	
				{
				if(tfont12[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//��һ����
				x++;
				if((x-x0)!=sizey)continue;
				x=x0;
				y++;
				break;
				}
			}
		//���ͽ������Ѳ���������		
		SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8); //����Ϊ8bit���ݳ���
		//���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
		return;	
		}				  	
	} 

/******************************************************************************
      ����˵������ʾ�����ַ�
      ������ݣ�x,y��ʾ����
                num Ҫ��ʾ���ַ�
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowChar(u16 x,u16 y,u8 num,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 buf,sizex,t,m=0;
	u16 i,TypefaceNum;//һ���ַ���ռ�ֽڴ�С
	u16 x0=x;
	const u8 *temp;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	//ȷ�������ַ�����Ӧ��pixel data
	if(num<' '||num>0x80)num='?';    //���������쳣��ֵ��дΪ?
	switch(sizey)
		{
		case 12:temp=&ascii_1206[num-' '][0];break;  //����6x12����
		case 16:temp=&ascii_1608[num-' '][0];break;  //����8x16����
		case 24:temp=&ascii_2412[num-' '][0];break;  //����12*24����
	  default : return; //���������ֿ���û�У�����ʾ
		}
  //������ʾ				
	LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //���ù��λ�� 
  if(!mode)SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16); //����Ϊ16bit���ݳ���
	for(i=0;i<TypefaceNum;i++)
		{ 
		//ȡ��������
		buf=temp[i];
		//�ǵ���ģʽ��ֱ��дPixelData
		if(!mode)for(t=0;t<8;t++)
			{
			SPI_SendData(HT_SPI0,buf&0x01?fc:bc);
			buf>>=1;	//������������һλ
			m++;
			while(HT_SPI0->SR&0x100); //�ȴ�SPI��ɲ���
			if(m%sizex)continue;	
			m=0;
			break;
			}
		//����ģʽ,ʹ�û��㷽ʽʵ��
		else for(t=0;t<8;t++)
			{
			if(buf&0x01)LCD_DrawPoint(x,y,fc);//��һ����
			buf>>=1;	//������������һλ
			x++;
			if((x-x0)!=sizex)continue;
			x=x0;
			y++;
			break;
			}
		}
  //��ʾ��������ʾΪ8bit����
  SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8);
	}

/******************************************************************************
      ����˵������ʾ�ַ���
      ������ݣ�x,y��ʾ����
                *p Ҫ��ʾ���ַ���
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowString(u16 x,u16 y,char *p,u16 fc,u16 bc,u8 sizey,u8 mode)
{         
	x+=1;
	while(*p!='\0')
	{       
		LCD_ShowChar(x,y,*p,fc,bc,sizey,mode);
		x+=(sizey/2)+1;
		p++;
	}  
}
/******************************************************************************
      ����˵������ʾ��Ӣ�Ļ�����ݵ��ַ���
      ������ݣ�x,y��ʾ����
                *p Ҫ��ʾ���ַ���
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowHybridString(u16 x,u16 y,char *str,u16 fc,u16 bc,u8 mode)
{
int i=0;
u16 xpos=x+1;
while(str[i]!=0)
	{
	//����	
	if(xpos>=LCD_W||str[i]==0x0D||str[i]==0x0A)
		{
		y+=12; //Y�ƶ�����һ��
		if(y>=LCD_H)return; //��ʾ���䳬����Χ��
		if(xpos<LCD_W)i++; //�ַ����������У���ת����һ���ַ�
		xpos=x+1; //���ص��ʼ��X��
		}		
	//�����ַ�
	else if(str[i]&0x80)
		{
		if(!(str[i+1]&0x80))return; //�ڶ����ַ�Ϊ�Ƿ�ֵ
		LCD_ShowChinese12x12(xpos,y,&str[i],fc,bc,12,mode);
		i+=2;
		xpos+=13;
		}
	//Ӣ���ַ�
	else
		{
		LCD_ShowChar(xpos,y,str[i],fc,bc,12,mode); //��ʾ�ַ�
		xpos+=7;
	  i++;
		}
	}
};

/******************************************************************************
      ����˵������ʾ����
      ������ݣ�m������nָ��
      ����ֵ��  ��
******************************************************************************/
u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;
	return result;
}


/******************************************************************************
      ����˵������ʾ��������
      ������ݣ�x,y��ʾ����
                num Ҫ��ʾ��������
                len Ҫ��ʾ��λ��
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowIntNum(u16 x,u16 y,u16 num,u8 len,u16 fc,u16 bc,u8 sizey)
{         	
	u8 t,temp;
	u8 enshow=0;
	u8 sizex=(sizey/2)+2;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+t*sizex,y,' ',fc,bc,sizey,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
} 


/******************************************************************************
      ����˵������ʾ��λС������
      ������ݣ�x,y��ʾ����
                num Ҫ��ʾС������
                len Ҫ��ʾ��λ��
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowFloatNum1(u16 x,u16 y,float num,u8 len,u16 fc,u16 bc,u8 sizey)
{   
   	
  int i,intPart,Mask,Idx=0;
	int buf,NumCount;
  //�Զ���������
	if(num<0)num*=(float)-1;
  intPart=(int)num;
  //��������
  if(intPart==0)
		{
		LCD_ShowChar(x+((sizey/2)+2)*Idx,y,'0',fc,bc,sizey,0);
		Idx++;
		}
	else //ʹ����ֵλ���Զ�ȷ��ϵͳ
		{
		for(i=0;i<5;i++)
			{
			Mask=mypow(10,i);
			if(!(intPart/Mask))break; //Ѱ���Զ����Mask��С
			}
		NumCount=i; //�����ܵ�λ��
		Mask=mypow(10,NumCount-1); //���λ��Mask=����-1
		for(i=0;i<NumCount;i++)
			{
			buf=(intPart/Mask);
			intPart%=Mask;
			buf%=10;
			LCD_ShowChar(x+((sizey/2)+2)*Idx,y,'0'+buf,fc,bc,sizey,0);
			Idx++;
			Mask/=10;
			}
		}
	//С������
	LCD_ShowChar(x+((sizey/2)+2)*Idx,y,'.',fc,bc,sizey,0);
	Idx++;
	num*=(float)mypow(10,len);
	Mask=mypow(10,len-1);
	intPart=(int)num;
	for(i=0;i<len;i++)
	{
	buf=(intPart/Mask);
	intPart%=Mask;
	buf%=10;
	LCD_ShowChar(x+((sizey/2)+2)*Idx,y,'0'+buf,fc,bc,sizey,0);
	Idx++;
	Mask/=10;
	}		
}
/******************************************************************************
      ����˵������ʾͼƬ
      ������ݣ�x,y�������
                length ͼƬ����
                width  ͼƬ���
                pic[]  ͼƬ����    
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowPicture(u16 x,u16 y,u16 length,u16 width,const u8 pic[])
{
	u32 size;
	u16 Data;
	//���õ�ַ
	LCD_Address_Set(x,y,x+length-1,y+width-1);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16);
	//��ʼ����д��
	size=length*width;
	do
		{
		Data=pic[1]|pic[0]<<8;
		while(HT_SPI0->SR&0x100);
		SPI_SendData(HT_SPI0,Data);
		pic+=2;
		}
	while(--size);
	//��ʾ��������ʾΪ8bit����
	while(HT_SPI0->SR&0x100); //�ı�SPI���ݳ�����Ҫ�����ݴ�������
  SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8);
}
