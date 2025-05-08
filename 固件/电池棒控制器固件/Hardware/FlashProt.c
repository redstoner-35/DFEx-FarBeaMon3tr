#include "ht32.h"
#include "delay.h"
#include "GUI.h"
#include "24Cxx.h"

#define ProgramSize 0x1FBFF  //����Ĵ�С
#define CRCWordAddress 0x1FC00  //�洢CRC�ֵĴ洢��

//����Debug�ӿ�״̬
void SetDebugPortState(bool IsEnable)
{
if(IsEnable)
	{
	GPIO_PullResistorConfig(HT_GPIOA, GPIO_PIN_12, GPIO_PR_DISABLE);	
  GPIO_PullResistorConfig(HT_GPIOA, GPIO_PIN_13, GPIO_PR_DISABLE);
	AFIO_GPxConfig(GPIO_PA, GPIO_PIN_12, AFIO_FUN_DEFAULT);
	AFIO_GPxConfig(GPIO_PA, GPIO_PIN_13, AFIO_FUN_DEFAULT);
	}
else //�ر�Debug
	{
	AFIO_GPxConfig(GPIO_PA, GPIO_PIN_12, AFIO_FUN_GPIO);
	AFIO_GPxConfig(GPIO_PA, GPIO_PIN_13, AFIO_FUN_GPIO);
	GPIO_PullResistorConfig(HT_GPIOA, GPIO_PIN_12, GPIO_PR_DOWN);	
  GPIO_PullResistorConfig(HT_GPIOA, GPIO_PIN_13, GPIO_PR_DOWN);
	GPIO_InputConfig(HT_GPIOA, GPIO_PIN_13, DISABLE);
	GPIO_InputConfig(HT_GPIOA, GPIO_PIN_12, DISABLE);
	}
}

//���������������CRC-32
unsigned int MainProgramRegionCRC(char UIDBUF[15])
 {
 unsigned int DATACRCResult;
 int i;
 char StorBuf;
 CKCU_PeripClockConfig_TypeDef CLKConfig={{0}};
 //��ʼ��CRC32      
 CLKConfig.Bit.CRC = 1;
 CKCU_PeripClockConfig(CLKConfig,ENABLE);//����CRC-32ʱ��  
 CRC_DeInit(HT_CRC);//�������
 HT_CRC->SDR = 0x0;//CRC-32 poly: 0x04C11DB7  
 HT_CRC->CR = CRC_32_POLY | CRC_BIT_RVS_WR | CRC_BIT_RVS_SUM | CRC_BYTE_RVS_SUM | CRC_CMPL_SUM;
 //��ʼУ��
 for(i=0;i<ProgramSize;i+=4)HT_CRC->DR=*(u32 *)i;//������д�뵽CRC�Ĵ�����
 for(i=0;i<15;i++)wb(&HT_CRC->DR,UIDBUF[i]);
 for(i=4;i<8;i++)HT_CRC->DR=i<4?*(u32*)(0x40080310+(i*4)):~*(u32*)(0x40080310+((i-4)*4));//д��FMC UID
 //У����ϼ�����
 DATACRCResult=HT_CRC->CSR;
 CRC_DeInit(HT_CRC);//���CRC���
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
  DATACRCResult>>=2; //������λ
	StorBuf^=0x01<<(i%8); //��i��8�ν������XOR
	wb(&HT_CRC->DR,StorBuf+i);
	}
 //ȡ�����ս�����ر�CRC������
 DATACRCResult=HT_CRC->CSR;
 CRC_DeInit(HT_CRC);//���CRC���	
 CLKConfig.Bit.CRC = 1;
 CKCU_PeripClockConfig(CLKConfig,DISABLE);//����CRC-32ʱ�ӽ�ʡ����
 return DATACRCResult; //���ؽ��
 }	

//�����ַ���
static void MakeCRCVendorString(char Str[16],unsigned int CRCIN)
	{
	char i;
	//ѭ����ȡ
	for(i=0;i<16;i++)
		{
		switch(CRCIN&0x03)
			{
			case 0:Str[i]='Q';break;
			case 1:Str[i]='!';break;
			case 2:Str[i]='s';break;
			case 3:Str[i]='&';break;
			}
		CRCIN>>=2; //������λ
		Str[i]+=i; //�ַ�������i
		Str[i]^=0x01<<(i%8); //��i��8�ν������XOR
		}
	}
 
//����flash����
void CheckForFlashLock(void)
 { 
 int i=0;
 FLASH_OptionByte Option;
 char UIDBUF[16],OTPData[16];
 unsigned int ProgramAreaCRC; 
 ShowPostInfo(45,"�̼������Լ��\0","5A",Msg_Statu);
 //���option byte�Ƿ���
 FLASH_GetOptionByteStatus(&Option);
 if(Option.MainSecurity != 0)
    { 
		//��ȡUID
		if(M24C512_ReadUID(UIDBUF,15))	 
			{
			ShowPostInfo(45,"�洢����ȡ�쳣\0","E5",Msg_Fault);
			SelfTestErrorHandler();
			} 
		if(M24C512_ReadSecuSct(OTPData,0x00,16))
			{
			ShowPostInfo(45,"��ȫ��Կ��ȡ�쳣\0","EF",Msg_Fault);
			SelfTestErrorHandler();
			} 
		//׼������ͼ��CRC
    ProgramAreaCRC=MainProgramRegionCRC(UIDBUF);
    MakeCRCVendorString(UIDBUF,ProgramAreaCRC);
		for(i=0;i<16;i++)if(*(u32 *)CRCWordAddress!=ProgramAreaCRC||UIDBUF[i]!=OTPData[i]) //����UID�ַ���飬�����鲻ͨ����ֱ������
			{
			ShowPostInfo(45,"�̼��ѱ��۸�!","EC",Msg_Fault);
			SelfTestErrorHandler();
			}
    return;
    }
 //����HSI(��flash����option byte��ҪHSI����)
 CKCU_HSICmd(ENABLE);
 while(CKCU_GetClockReadyStatus(CKCU_FLAG_HSIRDY) != SET)
   {
	 delay_ms(1);
	 i++;
	 if(i==50)break;
	 }
 if(i==50)return;
 //��̳����CRC32ֵ
 if(M24C512_ReadUID(UIDBUF,15))	 
	 {
	 ShowPostInfo(45,"�洢����ȡ�쳣\0","E5",Msg_Fault);
	 SelfTestErrorHandler();
	 } 
 ProgramAreaCRC=MainProgramRegionCRC(UIDBUF);
 if(FLASH_ErasePage(CRCWordAddress)!=FLASH_COMPLETE)return;
 if(FLASH_ProgramWordData(CRCWordAddress, ProgramAreaCRC)!=FLASH_COMPLETE)return;
 if(*((u32 *)CRCWordAddress)!=ProgramAreaCRC)return;
 //�����ַ�������д�뵽��ȫ����
 MakeCRCVendorString(UIDBUF,ProgramAreaCRC);	 
 if(M24C512_WriteSecuSct(UIDBUF,0x00,16))
	 	{
		ShowPostInfo(45,"��ȫ��Կд��ʧ��\0","EE",Msg_Fault);
		SelfTestErrorHandler();
		} 
 //������ȫ����
 Option.OptionProtect=1; //����ѡ��byte
 Option.MainSecurity=1;  //��ROP
 for(i=0;i<4;i++)Option.WriteProtect[i]=0xFFFFFFFF;//����ROM����
 FLASH_EraseOptionByte();
 if(FLASH_ProgramOptionByte(&Option)!=FLASH_COMPLETE)return;
 ShowPostInfo(45,"��⵽�״�����\0","5B",Msg_Warning); 		
 delay_Second(1);
 ShowPostInfo(45,"ϵͳ�̼�������\0","5C",Msg_Warning);	
 delay_Second(1);
 NVIC_SystemReset();  //ˢ��֮������
 while(1);
 }
