#include "lcd.h"
#include "lcd_init.h"
#include "lcdfont.h"
#include "delay.h"


/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
								color       要填充的颜色
      返回值：  无
******************************************************************************/
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)
{          
	u32 color1;
	PDMACH_InitTypeDef PDMAInitStr; 
	//检查参数
	if(xsta>=xend)return;
	if(ysta>=yend)return;
	//设置LCD地址
	color1=((u32)color)&0xFFFF;
	LCD_Address_Set(xsta,ysta,xend-1,yend-1);//设置显示范围
	//重配置SPI为16bit模式
	SPI_PDMACmd(HT_SPI0,SPI_PDMAREQ_TX,ENABLE);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16); //配置为16bit数据长度
	//初始化PDMA
	PDMAInitStr.PDMACH_AdrMod=SRC_ADR_FIX | DST_ADR_FIX; //源地址固定
	PDMAInitStr.PDMACH_BlkCnt=(xend-xsta)*(yend-ysta);//指定块数量，总数据量为需要填充的区域
	PDMAInitStr.PDMACH_BlkLen=1; //每个块1个Word大小
	PDMAInitStr.PDMACH_DataSize=WIDTH_16BIT;
	PDMAInitStr.PDMACH_DstAddr=(u32)(&HT_SPI0->DR);
	PDMAInitStr.PDMACH_Priority=M_PRIO; //128个数据块，块大小2字节(1Word),中优先级
	PDMAInitStr.PDMACH_SrcAddr=(u32)(&color1);//取颜色数据作为起始地址
	PDMA_Config(PDMA_SPI0_TX,&PDMAInitStr);//初始化TX DMA
  //启动传输并等待发送完毕
	IsPDMATranferDone=false;  //除能flag
	PDMA_EnaCmd(PDMA_SPI0_TX,ENABLE);	//启动SPI DMA通道
  while(!IsPDMATranferDone); //等待传输完毕
	//关闭SPI 16bit Tx模式
	while(HT_SPI0->SR&0x100); //等待SPI完成操作
	SPI_PDMACmd(HT_SPI0,SPI_PDMAREQ_TX,DISABLE);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8); //配置为8bit数据长度	
}

/******************************************************************************
      函数说明：在指定位置画点
      入口数据：x,y 画点坐标
                color 点的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_Address_Set(x,y,x,y);//设置光标位置 
	LCD_WR_DATA(color);
} 

/******************************************************************************
      函数说明：画水平线(使用DMA加速)
      入口数据：x1,x2   x轴起始和结束坐标
                y   y坐标
                color   线的颜色
      返回值：  无
******************************************************************************/
static void LCD_DrawHoriLine(u16 x1,u16 x2,u16 y,u16 color)
{
	u32 color1;
	u16 len;
	PDMACH_InitTypeDef PDMAInitStr; 
	//设置LCD地址
	color1=((u32)color)&0xFFFF;
	LCD_Address_Set(x1,y,x2-1,y);//设置显示范围
	//重配置SPI为16bit模式
	SPI_PDMACmd(HT_SPI0,SPI_PDMAREQ_TX,ENABLE);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16); //配置为16bit数据长度
	//初始化PDMA
	len=x1<x2?x2-x1:x1-x2;
	PDMAInitStr.PDMACH_AdrMod=SRC_ADR_FIX | DST_ADR_FIX; //源地址固定
	PDMAInitStr.PDMACH_BlkCnt=len;//指定块数量，总数据量=线长
	PDMAInitStr.PDMACH_BlkLen=1; //每个块1个Word大小
	PDMAInitStr.PDMACH_DataSize=WIDTH_16BIT;
	PDMAInitStr.PDMACH_DstAddr=(u32)(&HT_SPI0->DR);
	PDMAInitStr.PDMACH_Priority=M_PRIO; //128个数据块，块大小2字节(1Word),中优先级
	PDMAInitStr.PDMACH_SrcAddr=(u32)(&color1);//取颜色数据作为起始地址
	PDMA_Config(PDMA_SPI0_TX,&PDMAInitStr);//初始化TX DMA
  //启动传输并等待发送完毕
	IsPDMATranferDone=false;  //除能flag
	PDMA_EnaCmd(PDMA_SPI0_TX,ENABLE);	//启动SPI DMA通道
  while(!IsPDMATranferDone); //等待传输完毕
	//关闭SPI 16bit Tx模式
	while(HT_SPI0->SR&0x100); //等待SPI完成操作
	SPI_PDMACmd(HT_SPI0,SPI_PDMAREQ_TX,DISABLE);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8); //配置为8bit数据长度	
}

/******************************************************************************
      函数说明：画垂直线(使用DMA加速)
      入口数据：y1,y2   y轴起始和结束坐标
                x   x坐标
                color   线的颜色
      返回值：  无
******************************************************************************/
static void LCD_DrawVertLine(u16 y1,u16 y2,u16 x,u16 color)
{
	u32 color1;
	u16 len;
	PDMACH_InitTypeDef PDMAInitStr; 
	//设置LCD地址
	color1=((u32)color)&0xFFFF;
	LCD_Address_Set(x,y1,x,y2-1);//设置显示范围
	//重配置SPI为16bit模式
	SPI_PDMACmd(HT_SPI0,SPI_PDMAREQ_TX,ENABLE);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16); //配置为16bit数据长度
	//初始化PDMA
	len=y1<y2?y2-y1:y1-y2;
	PDMAInitStr.PDMACH_AdrMod=SRC_ADR_FIX | DST_ADR_FIX; //源地址固定
	PDMAInitStr.PDMACH_BlkCnt=len;//指定块数量，总数据量=线长
	PDMAInitStr.PDMACH_BlkLen=1; //每个块1个Word大小
	PDMAInitStr.PDMACH_DataSize=WIDTH_16BIT;
	PDMAInitStr.PDMACH_DstAddr=(u32)(&HT_SPI0->DR);
	PDMAInitStr.PDMACH_Priority=M_PRIO; //128个数据块，块大小2字节(1Word),中优先级
	PDMAInitStr.PDMACH_SrcAddr=(u32)(&color1);//取颜色数据作为起始地址
	PDMA_Config(PDMA_SPI0_TX,&PDMAInitStr);//初始化TX DMA
  //启动传输并等待发送完毕
	IsPDMATranferDone=false;  //除能flag
	PDMA_EnaCmd(PDMA_SPI0_TX,ENABLE);	//启动SPI DMA通道
  while(!IsPDMATranferDone); //等待传输完毕
	//关闭SPI 16bit Tx模式
	while(HT_SPI0->SR&0x100); //等待SPI完成操作
	SPI_PDMACmd(HT_SPI0,SPI_PDMAREQ_TX,DISABLE);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8); //配置为8bit数据长度	
}

/******************************************************************************
      函数说明：画线
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   线的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawLine(u16 x1,u16 y1,u16 x2,u16 y2,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
  //水平线，直接使用PDMA加速
	if(y1==y2)
		{
		LCD_DrawHoriLine(x1,x2,y1,color);
		return;
		}
	//垂直线，直接使用PDMA加速
	if(x1==x2)
		{
		LCD_DrawVertLine(y1,y2,x1,color);
		return;
		}	
	//其余情况的歪线PDMA加速不了的，正常绘制
	delta_x=x2-x1; 
	delta_y=y2-y1;  //计算坐标增量 
	uRow=x1;
	uCol=y1;  //画线起点坐标
	if(delta_x>0)incx=1; //设置单步方向 
	else if (delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//水平线 
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		LCD_DrawPoint(uRow,uCol,color);//画点
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
      函数说明：画矩形
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   矩形的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
	{
	//上边框
	LCD_DrawHoriLine(x1,x2,y1,color);
	//下边框
	LCD_DrawHoriLine(x1,x2,y2,color);
	//左边框
	LCD_DrawVertLine(y1,y2,x1,color);
	//右边框
	LCD_DrawVertLine(y1,y2,x2,color);
	}

/******************************************************************************
      函数说明：画圆
      入口数据：x0,y0   圆心坐标
                r       半径
                color   圆的颜色
      返回值：  无
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
		if((a*a+b*b)>(r*r))//判断要画的点是否过远
		{
			b--;
		}
	}
}

/******************************************************************************
      函数说明：显示汉字串
      入口数据：x,y显示坐标
                *s 要显示的汉字串
                fc 字的颜色
                bc 字的背景色
                sizey 字号 可选 16 24 32
                mode:  0非叠加模式  1叠加模式
      返回值：  无
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
      函数说明：显示单个12x12汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChinese12x12(u16 x,u16 y,char *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 i,j,m=0;
	u16 k;
	u16 HZnum;//汉字数目
	u16 TypefaceNum;//一个字符所占字节大小
	u16 x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	                         
	HZnum=sizeof(tfont12)/sizeof(typFNT_GB12);	//统计汉字数目
	for(k=0;k<HZnum;k++)if((tfont12[k].Index[0]==*(s))&&(tfont12[k].Index[1]==*(s+1))) 
		{ 		
		LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);	//设置地址范围
		if(!mode)SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16); //配置为16bit数据长度		
		//开始发送流程
		for(i=0;i<TypefaceNum;i++)
			{
			//非叠加方式，直接写像素数据
			if(!mode)for(j=0;j<8;j++)
				{	
				SPI_SendData(HT_SPI0,tfont12[k].Msk[i]&(0x01<<j)?fc:bc);
				m++;
				while(HT_SPI0->SR&0x100); //等待SPI完成操作
				if(m%sizey)continue;
				m=0;
				break;
				}
			//叠加方式,使用画点实现	
			else for(j=0;j<8;j++)	
				{
				if(tfont12[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
				x++;
				if((x-x0)!=sizey)continue;
				x=x0;
				y++;
				break;
				}
			}
		//发送结束，把参数调回来		
		SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8); //配置为8bit数据长度
		//查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
		return;	
		}				  	
	} 

/******************************************************************************
      函数说明：显示单个字符
      入口数据：x,y显示坐标
                num 要显示的字符
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChar(u16 x,u16 y,u8 num,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 buf,sizex,t,m=0;
	u16 i,TypefaceNum;//一个字符所占字节大小
	u16 x0=x;
	const u8 *temp;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	//确定传入字符所对应的pixel data
	if(num<' '||num>0x80)num='?';    //传进来的异常数值填写为?
	switch(sizey)
		{
		case 12:temp=&ascii_1206[num-' '][0];break;  //调用6x12字体
		case 16:temp=&ascii_1608[num-' '][0];break;  //调用8x16字体
		case 24:temp=&ascii_2412[num-' '][0];break;  //调用12*24字体
	  default : return; //其余字体字库内没有，不显示
		}
  //进行显示				
	LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //设置光标位置 
  if(!mode)SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16); //配置为16bit数据长度
	for(i=0;i<TypefaceNum;i++)
		{ 
		//取字形数据
		buf=temp[i];
		//非叠加模式，直接写PixelData
		if(!mode)for(t=0;t<8;t++)
			{
			SPI_SendData(HT_SPI0,buf&0x01?fc:bc);
			buf>>=1;	//字形数据右移一位
			m++;
			while(HT_SPI0->SR&0x100); //等待SPI完成操作
			if(m%sizex)continue;	
			m=0;
			break;
			}
		//叠加模式,使用画点方式实现
		else for(t=0;t<8;t++)
			{
			if(buf&0x01)LCD_DrawPoint(x,y,fc);//画一个点
			buf>>=1;	//字形数据右移一位
			x++;
			if((x-x0)!=sizex)continue;
			x=x0;
			y++;
			break;
			}
		}
  //显示结束，显示为8bit长度
  SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8);
	}

/******************************************************************************
      函数说明：显示字符串
      入口数据：x,y显示坐标
                *p 要显示的字符串
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
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
      函数说明：显示中英文混合内容的字符串
      入口数据：x,y显示坐标
                *p 要显示的字符串
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowHybridString(u16 x,u16 y,char *str,u16 fc,u16 bc,u8 mode)
{
int i=0;
u16 xpos=x+1;
while(str[i]!=0)
	{
	//换行	
	if(xpos>=LCD_W||str[i]==0x0D||str[i]==0x0A)
		{
		y+=12; //Y移动到下一行
		if(y>=LCD_H)return; //显示区间超出范围了
		if(xpos<LCD_W)i++; //字符串触发换行，跳转到下一个字符
		xpos=x+1; //光标回到最开始的X轴
		}		
	//中文字符
	else if(str[i]&0x80)
		{
		if(!(str[i+1]&0x80))return; //第二个字符为非法值
		LCD_ShowChinese12x12(xpos,y,&str[i],fc,bc,12,mode);
		i+=2;
		xpos+=13;
		}
	//英文字符
	else
		{
		LCD_ShowChar(xpos,y,str[i],fc,bc,12,mode); //显示字符
		xpos+=7;
	  i++;
		}
	}
};

/******************************************************************************
      函数说明：显示数字
      入口数据：m底数，n指数
      返回值：  无
******************************************************************************/
u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;
	return result;
}


/******************************************************************************
      函数说明：显示整数变量
      入口数据：x,y显示坐标
                num 要显示整数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
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
      函数说明：显示两位小数变量
      入口数据：x,y显示坐标
                num 要显示小数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void LCD_ShowFloatNum1(u16 x,u16 y,float num,u8 len,u16 fc,u16 bc,u8 sizey)
{   
   	
  int i,intPart,Mask,Idx=0;
	int buf,NumCount;
  //自动修正负数
	if(num<0)num*=(float)-1;
  intPart=(int)num;
  //整数部分
  if(intPart==0)
		{
		LCD_ShowChar(x+((sizey/2)+2)*Idx,y,'0',fc,bc,sizey,0);
		Idx++;
		}
	else //使用数值位数自动确定系统
		{
		for(i=0;i<5;i++)
			{
			Mask=mypow(10,i);
			if(!(intPart/Mask))break; //寻找自定义的Mask大小
			}
		NumCount=i; //计算总的位数
		Mask=mypow(10,NumCount-1); //最高位的Mask=数字-1
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
	//小数部分
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
      函数说明：显示图片
      入口数据：x,y起点坐标
                length 图片长度
                width  图片宽度
                pic[]  图片数组    
      返回值：  无
******************************************************************************/
void LCD_ShowPicture(u16 x,u16 y,u16 length,u16 width,const u8 pic[])
{
	u32 size;
	u16 Data;
	//设置地址
	LCD_Address_Set(x,y,x+length-1,y+width-1);
	SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_16);
	//开始无脑写入
	size=length*width;
	do
		{
		Data=pic[1]|pic[0]<<8;
		while(HT_SPI0->SR&0x100);
		SPI_SendData(HT_SPI0,Data);
		pic+=2;
		}
	while(--size);
	//显示结束，显示为8bit长度
	while(HT_SPI0->SR&0x100); //改变SPI数据长度需要等数据处理完了
  SPI_SetDataLength(HT_SPI0,SPI_DATALENGTH_8);
}
