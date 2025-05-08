#include "GUI.h"
#include "Config.h"
#include "IP2366_REG.h"
#include <string.h>

//�ַ���
static char PDO20VMsg[16]={0};

const BoolListEntryDef PDOParam[7]=
	{
		{
		"PPS1",
		false,
		&CfgData.PDOCFG.EnablePPS1,
		false,
		false
		},
   	{
		"PPS2",
		false,
		&CfgData.PDOCFG.EnablePPS2,
		false,
		false
		},
		{
		PDO20VMsg,
		true,
		&CfgData.PDOCFG.Enable20V,
		false,
		false
		},
		{
		"15V",
		false,
		&CfgData.PDOCFG.Enable15V,
		false,
		false
		},
		{
		"12V PDO",
		false,
		&CfgData.PDOCFG.Enable12V,
		false,
		false
		},
		{
		"9V PDO",
		false,
		&CfgData.PDOCFG.Enable9V,
		false,
		false
		},
		{ //ռλ��
		"",
		false,
		&AlwaysFalse,
		true,
		false
		}		
	};

//����PDO�㲥����ǰ��Ҫ׼��һ���ַ���	
void PreparePDO20VStr(void)
	{
	memset(PDO20VMsg,0x00,sizeof(PDO20VMsg));
	if(CfgData.MaxVPD==PDMaxIN_20V)strncpy(PDO20VMsg,"20V",sizeof(PDO20VMsg));
  else strncpy(PDO20VMsg,"20V��28V EPR",sizeof(PDO20VMsg));
	}
	
//��������
void LeaveDisMgmtMenu(void);

const MenuConfigDef PDOCfgMenu=
	{
	MenuType_BoolListSetup,
	//����������
	PDOParam,
	//ö�ٱ༭�����
	NULL,
  NULL,
  NULL,		
	//������Ⱦ�Ĵ���
	NULL, //��Ⱦ����
	NULL, //��������
	//�����ò˵�
	"PDO�㲥����",
	NULL,
	NULL,
	&LeaveDisMgmtMenu, 
	//������˳����캯��
	&PreparePDO20VStr,  //����ǰ��Ҫ����һ���ַ�������PDOģʽ�����Ƿ���EPR��ص���ʾ
	NULL
	};	
