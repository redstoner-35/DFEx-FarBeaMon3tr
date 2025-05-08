#include "PMBUS.h"
#include "INA226.h"
#include "delay.h"
#include "I2CAddr.h"

//��ʼ��INA226
//����ö������
INA226InitStatDef INA226_INIT(INAinitStrdef * INAConf)
 {
  unsigned int ConfREG=0x0000;//Ĭ�ϼĴ���Ϊ0
	double shuntResValue;//����������ʾ�����������ֵ
	unsigned int buf;
	unsigned int CalREG;
	char Retrycount=0;
	//���Ƚ��и�λ
  do{
		 //д����
		 buf=0xC127;
	   PMBUS_WordReadWrite(true,true,&buf,INA226ADDR,0);//���Ĵ�����дĬ������
	   delay_ms(1);
		 //��ȡ����
		 buf=0;
		 Retrycount++;//���Դ���+1
	   if(!PMBUS_WordReadWrite(true,false,&buf,INA226ADDR,0))continue;//��ȡʧ��
		 if(buf==0x4127)break;//��֤ͨ��
	  }while(Retrycount<5);	 
  //У�鸴λ�Ƿ�ɹ�
  if(Retrycount>=5)return A226_Error_SMBUS_NACK;//��������˺ܶ�θ�λ����ʧ�����˳�
  //��������ת����ƽ������
	ConfREG=(unsigned int)(INAConf->AvgCount&0x07)<<9; //����AVG[2:0]
  //�ڶ���������VBUS��ת��ʱ��
	ConfREG|=(unsigned int)(INAConf->VBUSConvTime&0x07)<<6; //����VBUSCT[2:0]
	//�����������÷�������ת��ʱ��
	ConfREG|=(unsigned int)(INAConf->IBUSConvTime&0x07)<<3;	//����VSHCT[2:0]
	//���һ����д��ģʽ����bit
	ConfREG&=0xFFF8;
	ConfREG|=((unsigned int)INAConf->ConvMode)&0x07;
	//������bitд������üĴ�������У��
	PMBUS_WordReadWrite(true,true,&ConfREG,INA226ADDR,0x0);//д��
	delay_ms(1);
	buf=0;
	PMBUS_WordReadWrite(true,false,&buf,INA226ADDR,0);
	buf&=0xFFF;	
	ConfREG&=0xFFF; //����߹��ĵ�λ��D11-D0������λ����������Mask��
	if(buf!=ConfREG)return A226_Error_ProgramReg;//У�飬���д��ȥ�����򱨴�
	//�Ƚ�������ID
	PMBUS_WordReadWrite(true,false,&buf,INA226ADDR,0xFE);
	if(buf!=0x5449)return A226_Error_NotGenuineDevice; //��Ӧ��ID�Բ��ϣ����طǹٷ��豸��ʾ
	PMBUS_WordReadWrite(true,false,&buf,INA226ADDR,0xFF);
  buf&=0xFFF0; //Mask��RevIDֻ����die ID
	if(buf!=0x2260)return A226_Error_NotGenuineDevice; //��������DIE ID�Բ��ϣ��ǹٷ��豸
	//�����У׼�Ĵ�����ֵ
	shuntResValue=(double)(INAConf->ShuntValue)/(double)1000;//������ֵ��λ�Ӻ�ŷת��Ϊŷ
	shuntResValue=(double)0.00512/(CurrentLSB*shuntResValue);//�����У׼�Ĵ��������ݲ���ǿ��ȡ��
	//������õ�У��ֵ�Ƿ�Ϸ�
  CalREG=(unsigned int)shuntResValue;//ȡ��	
  if(CalREG&0x8000)return A226_Error_CalibrationReg;//���������У׼ֵ�Ƿ񳬹�65536����������򷵻ش�������
	//��У׼����д��INA226
	PMBUS_WordReadWrite(true,true,&CalREG,INA226ADDR,0x5);//д��
	delay_ms(1);
	buf=0;
	PMBUS_WordReadWrite(true,false,&buf,INA226ADDR,5);
	buf&=0x7FFF;
	CalREG&=0x7FFF; //У׼�Ĵ�����ֻ����Bit14-0������λMask��
	if(buf!=CalREG)return A226_Error_ProgramCalReg;//У�飬���д��ȥ�����򱨴�
	//���ñ����Ĵ���
	CalREG=(unsigned int)INAConf->AlertConfig; //Ӧ��bitfield(Alert[4:0])
	if(INAConf->IsEnableAlertLatch)CalREG|=0x01; //���ʹ�ܱ������湦������LEN=1
	if(INAConf->IsAlertPinInverted)CalREG|=0x02; //���ʹ�ܱ���������Է�ת��������APOL=1
	if(INAConf->AlertConfig!=A226_AlertDisable)CalREG|=0x400;  //���ʹ�ܱ�������������CNVR=1����Alert Pin���
	//дMask/Enable�Ĵ������ø澯����
	if(!PMBUS_WordReadWrite(true,true,&CalREG,INA226ADDR,0x6))return A226_Error_SetAlertCfg;
  //��Щ����ȫ����ɣ���������
	return A226_Init_OK;
 }
 
//���ñ����Ĵ�������ֵ
bool INA226_SetAlertRegister(unsigned int Value)
	{
	unsigned int regtemp;
	regtemp=Value&0xFFFF;
	if(!PMBUS_WordReadWrite(true,false,&regtemp,INA226ADDR,0x07))return false;
	//������Ϸ���true
	return true;
	}
 
//���INA226�Ƿ���˳������ת��
bool INA226_QueueIfGaugeCanReady(void)
	{
	unsigned int regtemp;
	//��ȡMask&Enable�Ĵ����ж�OVF�Ƿ����1
	if(!PMBUS_WordReadWrite(true,false,&regtemp,INA226ADDR,0x06))return false;
	if(regtemp&0x8)return true;//CNVR=1,ת�����
  //����ת��ʧ��
	return false;
	}	
	
 //��ȡ��ֵ
bool INA226_GetBusInformation(INADoutSreDef *INADout)
 {
 unsigned int regtemp;
 signed int calctemp;
 //��ȡMask&Enable�Ĵ����ж�OVF�Ƿ����1
 if(!PMBUS_WordReadWrite(true,false,&regtemp,INA226ADDR,0x06))return false;
 if(regtemp&0x4)//OVF=1���������
	 {
	 INADout->BusCurrent=0;
	 INADout->BusVolt=0;
	 INADout->BusPower=0;//������Ч
	 } 
	//��ʼ��ȡ����
  else
	 {
	 //ת����ѹ����
	 if(!PMBUS_WordReadWrite(true,false,&regtemp,INA226ADDR,0x02))return false;  //��ȡVBUS�Ĵ���
	 INADout->BusVolt=(float)((float)regtemp*(float)BusVoltLSB/(float)1000); //�������ѹ
	 //��ȡ��������
	 if(!PMBUS_WordReadWrite(true,false,&regtemp,INA226ADDR,0x04))return false;
	 if(regtemp&0x8000)//�������λΪ1˵������ֵ�Ǹ���
			calctemp=regtemp|0xFFFF8000;
		else 
			calctemp=regtemp&0x7FFF;//���˵����λ
		INADout->BusCurrent=(float)((float)calctemp*CurrentLSB);//�������õ�LSB���������
		if(!PMBUS_WordReadWrite(true,false,&regtemp,INA226ADDR,0x03))return false;
		INADout->BusPower=(float)((float)regtemp*PowerLSB);//��������ʣ�LSB�ǵ���ֵ*20(��λW)			
		} 
 //������ϣ�����true
 return true;
 }
