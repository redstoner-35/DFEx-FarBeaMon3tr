#include "ModeControl.h"
#include "cms8s6990.h"
#include "stdbool.h"
#include "SysConfig.h"
#include "Flash.h"
#include "SideKey.h"
#include "SpecialMode.h"
#include "delay.h"
#include "LEDMgmt.h"
#include "SysReset.h"

//�ڲ�ȫ��
static xdata int CurrentIdx=0;
static xdata u8 CurrentCRC=0;

//CRC-8���� 
static u8 PEC8Check(char *DIN,char Len)
{
 unsigned char crcbuf=0xFF;
 char i;
 do
	{
  //��������
  crcbuf^=*DIN++;
  //����
  for(i=8;i;i--)
   {
	 if(crcbuf&0x80)crcbuf=(crcbuf<<1)^0x07;//���λΪ1������֮��Ͷ���ʽXOR
	 else crcbuf<<=1;//���λΪ0��ֻ��λ��XOR
	 }
	}
 while(--Len);
 //������
 return crcbuf;
}

//��EEPROM��Ѱ������һ��Sys����
static int SearchSysConfig(SysROMImg *ROMData)
	{
	char i;
	int Len=0;
	//����flash����ʼ��ȡ
	SetFlashState(1);
	do
		{		
		for(i=0;i<sizeof(SysROMImageDef);i++)Flash_Operation(DataFlash_Read,i+(Len*sizeof(SysROMImg)),&ROMData->ByteBuf[i]); //��ROM�ڶ�ȡ����
		if(ROMData->Data.CheckSum!=PEC8Check(ROMData->Data.SysConfig.ByteBuf,sizeof(SysStorDef)))break; //�ҵ���û�б�д��CRCУ�鲻���ĵط�����������
		Len++;
		}
	while(Len<SysCfgGroupLen);
	//��ȡ��һ����ȷ������
	if(Len>0)Len--;
	for(i=0;i<sizeof(SysROMImageDef);i++)Flash_Operation(DataFlash_Read,i+(Len*sizeof(SysROMImg)),&ROMData->ByteBuf[i]);
	//��ȡ������������һ�������ݵ�index
	return Len;
	}

//��ȡ�޼���������
void ReadSysConfig(void)
	{
	extern code ModeStrDef ModeSettings[ModeTotalDepth];
	SysROMImg ROMData;
	bit KState=GetIfKeyPressed();
	//��ȡ����
	CurrentIdx=SearchSysConfig(&ROMData);
	//���ж������ݵ�У��
	if(!KState&&ROMData.Data.CheckSum==PEC8Check(ROMData.Data.SysConfig.ByteBuf,sizeof(SysStorDef)))
		{
		//У��ɹ�����������
		SysMode=ROMData.Data.SysConfig.Data.IsSystemLocked?Operation_Locked:Operation_Normal;
		SysCfg.LocatorCfg=ROMData.Data.SysConfig.Data.LocatorCfg; 
		SysCfg.RampCurrent=ROMData.Data.SysConfig.Data.SysCurrent;
		IsRampEnabled=ROMData.Data.SysConfig.Data.IsRampEnabled?1:0;
		//�洢��ǰ��indexֵ
		CurrentCRC=ROMData.Data.CheckSum;
		CurrentIdx++; //��ǰλ�������ݣ���Ҫ��index+1�ƶ���δд���λ��
		}
	//У��ʧ���ؽ�����
	else 
		{
		SysMode=Operation_Normal; //Ĭ�ϴ��ڽ���ģʽ
		SysCfg.LocatorCfg=Locator_Green; //Ĭ�����̵���
		RestoreToMinimumSysCurrent();
		IsRampEnabled=0; //Ĭ��Ϊ��λģʽ
		SaveSysConfig(1); //�ؽ����ݺ������������
		if(KState)while(GetIfKeyPressed())
			{
			IsHalfBrightness=0;
			MakeFastStrobe(LED_Amber);
			delay_ms(40);
			}
		//����ϵͳ��λ
		TriggerSoftwareReset();
		}
	//��ȡ������ϣ�����flash	
	SetFlashState(0);
	}

//�ָ����޼�����ģʽ����͵���
void RestoreToMinimumSysCurrent(void)	
	{
	char i;
	extern code ModeStrDef ModeSettings[ModeTotalDepth];
	SysCfg.RampCurrent=100;
	for(i=0;i<ModeTotalDepth;i++)if(ModeSettings[i].ModeIdx==Mode_Ramp)
			SysCfg.RampCurrent=ModeSettings[i].MinCurrent; //�ҵ���λ�������޼�����ĵ�λ
	}

//�����޼���������
void SaveSysConfig(bit IsForceSave)
	{
	char i;
	SysROMImg SavedData;
	//����flash��CRCУ��ģ����Ҫ��ȡFlash������Ҫ������
	SetFlashState(1);
  //��ʼ�������ݹ���
	SavedData.Data.SysConfig.Data.IsSystemLocked=SysMode==Operation_Locked?true:false;
	SavedData.Data.SysConfig.Data.LocatorCfg=SysCfg.LocatorCfg;
  SavedData.Data.SysConfig.Data.SysCurrent=SysCfg.RampCurrent;
	SavedData.Data.SysConfig.Data.IsRampEnabled=IsRampEnabled?true:false;
	SavedData.Data.CheckSum=PEC8Check(SavedData.Data.SysConfig.ByteBuf,sizeof(SysStorDef)); //����CRC
	//�������ݱȶ�
	if(!IsForceSave&&SavedData.Data.CheckSum==CurrentCRC)
		{
		SetFlashState(0);//��ȡ������ϣ�����flash	
	  return; //�������������������ͬ	
		}
	//������Ҫ���棬��ʼ����Ƿ���Ҫ����
	if(IsForceSave||CurrentIdx>=SysCfgGroupLen) 
		{
		//�����Ѿ�д���ˣ�������0��1������ȫ����
		Flash_Operation(DataFlash_Erase,0x200,&i);  //����2=512-1023
		Flash_Operation(DataFlash_Erase,0,&i);      //����1=0-511
		//�ӵ�0��λ�ÿ�ʼд��
		CurrentIdx=0;
		}
	//д������
	for(i=0;i<sizeof(SysROMImageDef);i++)Flash_Operation(DataFlash_Write,i+(CurrentIdx*sizeof(SysROMImg)),&SavedData.ByteBuf[i]);	
	CurrentIdx++; //��index�ѱ�д�룬���д���¸�idx
	CurrentCRC=SavedData.Data.CheckSum; //���汾��index��CRC8
	SetFlashState(0);//д�������ϣ�����flash	
	}	
