#include "ht32.h"
#include "delay.h"
#include "GUI.h"
#include "24Cxx.h"

#define ProgramSize 0x1FBFF  //程序的大小
#define CRCWordAddress 0x1FC00  //存储CRC字的存储器

//设置Debug接口状态
void SetDebugPortState(bool IsEnable)
{
if(IsEnable)
	{
	GPIO_PullResistorConfig(HT_GPIOA, GPIO_PIN_12, GPIO_PR_DISABLE);	
  GPIO_PullResistorConfig(HT_GPIOA, GPIO_PIN_13, GPIO_PR_DISABLE);
	AFIO_GPxConfig(GPIO_PA, GPIO_PIN_12, AFIO_FUN_DEFAULT);
	AFIO_GPxConfig(GPIO_PA, GPIO_PIN_13, AFIO_FUN_DEFAULT);
	}
else //关闭Debug
	{
	AFIO_GPxConfig(GPIO_PA, GPIO_PIN_12, AFIO_FUN_GPIO);
	AFIO_GPxConfig(GPIO_PA, GPIO_PIN_13, AFIO_FUN_GPIO);
	GPIO_PullResistorConfig(HT_GPIOA, GPIO_PIN_12, GPIO_PR_DOWN);	
  GPIO_PullResistorConfig(HT_GPIOA, GPIO_PIN_13, GPIO_PR_DOWN);
	GPIO_InputConfig(HT_GPIOA, GPIO_PIN_13, DISABLE);
	GPIO_InputConfig(HT_GPIOA, GPIO_PIN_12, DISABLE);
	}
}

//计算主程序区域的CRC-32
unsigned int MainProgramRegionCRC(char UIDBUF[15])
 {
 unsigned int DATACRCResult;
 int i;
 char StorBuf;
 CKCU_PeripClockConfig_TypeDef CLKConfig={{0}};
 //初始化CRC32      
 CLKConfig.Bit.CRC = 1;
 CKCU_PeripClockConfig(CLKConfig,ENABLE);//启用CRC-32时钟  
 CRC_DeInit(HT_CRC);//清除配置
 HT_CRC->SDR = 0x0;//CRC-32 poly: 0x04C11DB7  
 HT_CRC->CR = CRC_32_POLY | CRC_BIT_RVS_WR | CRC_BIT_RVS_SUM | CRC_BYTE_RVS_SUM | CRC_CMPL_SUM;
 //开始校验
 for(i=0;i<ProgramSize;i+=4)HT_CRC->DR=*(u32 *)i;//将内容写入到CRC寄存器内
 for(i=0;i<15;i++)wb(&HT_CRC->DR,UIDBUF[i]);
 for(i=4;i<8;i++)HT_CRC->DR=i<4?*(u32*)(0x40080310+(i*4)):~*(u32*)(0x40080310+((i-4)*4));//写入FMC UID
 //校验完毕计算结果
 DATACRCResult=HT_CRC->CSR;
 CRC_DeInit(HT_CRC);//清除CRC结果
 HT_CRC->SDR = 0x0;//CRC-32 poly: 0x04C11DB7  
 HT_CRC->CR = CRC_32_POLY | CRC_BIT_RVS_WR | CRC_BIT_RVS_SUM | CRC_BYTE_RVS_SUM | CRC_CMPL_SUM;
 for(i=0;i<16;i++)
	{
	switch(DATACRCResult&0x03)
		{
		case 0:StorBuf='E';break;
		case 1:StorBuf='3';break;
		case 2:StorBuf='5';break;
		case 3:StorBuf='A';break;
		}
  DATACRCResult>>=2; //右移两位
	StorBuf^=0x01<<(i%8); //和i的8次结果进行XOR
	wb(&HT_CRC->DR,StorBuf+i);
	}
 //取出最终结果并关闭CRC计算器
 DATACRCResult=HT_CRC->CSR;
 CRC_DeInit(HT_CRC);//清除CRC结果	
 CLKConfig.Bit.CRC = 1;
 CKCU_PeripClockConfig(CLKConfig,DISABLE);//禁用CRC-32时钟节省电力
 return DATACRCResult; //返回结果
 }	

//生成字符串
static void MakeCRCVendorString(char Str[16],unsigned int CRCIN)
	{
	char i;
	//循环获取
	for(i=0;i<16;i++)
		{
		switch(CRCIN&0x03)
			{
			case 0:Str[i]='Q';break;
			case 1:Str[i]='!';break;
			case 2:Str[i]='s';break;
			case 3:Str[i]='&';break;
			}
		CRCIN>>=2; //右移两位
		Str[i]+=i; //字符串加上i
		Str[i]^=0x01<<(i%8); //和i的8次结果进行XOR
		}
	}
 
//启用flash锁定
void CheckForFlashLock(void)
 { 
 int i=0;
 FLASH_OptionByte Option;
 char UIDBUF[16],OTPData[16];
 unsigned int ProgramAreaCRC; 
 ShowPostInfo(45,"固件完整性检查\0","5A",Msg_Statu);
 //检查option byte是否开启
 FLASH_GetOptionByteStatus(&Option);
 if(Option.MainSecurity != 0)
    { 
		//读取UID
		if(M24C512_ReadUID(UIDBUF,15))	 
			{
			ShowPostInfo(45,"存储器读取异常\0","E5",Msg_Fault);
			SelfTestErrorHandler();
			} 
		if(M24C512_ReadSecuSct(OTPData,0x00,16))
			{
			ShowPostInfo(45,"安全密钥读取异常\0","EF",Msg_Fault);
			SelfTestErrorHandler();
			} 
		//准备计算和检查CRC
    ProgramAreaCRC=MainProgramRegionCRC(UIDBUF);
    MakeCRCVendorString(UIDBUF,ProgramAreaCRC);
		for(i=0;i<16;i++)if(*(u32 *)CRCWordAddress!=ProgramAreaCRC||UIDBUF[i]!=OTPData[i]) //进行UID字符检查，如果检查不通过则直接锁死
			{
			ShowPostInfo(45,"固件已被篡改!","EC",Msg_Fault);
			SelfTestErrorHandler();
			}
    return;
    }
 //启用HSI(给flash设置option byte需要HSI启用)
 CKCU_HSICmd(ENABLE);
 while(CKCU_GetClockReadyStatus(CKCU_FLAG_HSIRDY) != SET)
   {
	 delay_ms(1);
	 i++;
	 if(i==50)break;
	 }
 if(i==50)return;
 //编程程序的CRC32值
 if(M24C512_ReadUID(UIDBUF,15))	 
	 {
	 ShowPostInfo(45,"存储器读取异常\0","E5",Msg_Fault);
	 SelfTestErrorHandler();
	 } 
 ProgramAreaCRC=MainProgramRegionCRC(UIDBUF);
 if(FLASH_ErasePage(CRCWordAddress)!=FLASH_COMPLETE)return;
 if(FLASH_ProgramWordData(CRCWordAddress, ProgramAreaCRC)!=FLASH_COMPLETE)return;
 if(*((u32 *)CRCWordAddress)!=ProgramAreaCRC)return;
 //生成字符串并且写入到安全扇区
 MakeCRCVendorString(UIDBUF,ProgramAreaCRC);	 
 if(M24C512_WriteSecuSct(UIDBUF,0x00,16))
	 	{
		ShowPostInfo(45,"安全密钥写入失败\0","EE",Msg_Fault);
		SelfTestErrorHandler();
		} 
 //打开主安全功能
 Option.OptionProtect=1; //锁定选项byte
 Option.MainSecurity=1;  //打开ROP
 for(i=0;i<4;i++)Option.WriteProtect[i]=0xFFFFFFFF;//所有ROM上锁
 FLASH_EraseOptionByte();
 if(FLASH_ProgramOptionByte(&Option)!=FLASH_COMPLETE)return;
 ShowPostInfo(45,"检测到首次启动\0","5B",Msg_Warning); 		
 delay_Second(1);
 ShowPostInfo(45,"系统固件已锁定\0","5C",Msg_Warning);	
 delay_Second(1);
 NVIC_SystemReset();  //刷完之后重启
 while(1);
 }
