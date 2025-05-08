#ifndef _GUI_
#define _GUI_

//�ڲ�����
#include "lcd.h"
#include "LCD_Init.h"
#include <stdbool.h>
#include <stdlib.h>

typedef enum
	{
	Msg_Statu, //״̬��Ϣ
	Msg_Warning, //����
	Msg_POSTOK, //�Լ����
	Msg_Fault  //����
	}MessageTypeDef;

typedef struct
	{
	int *Source; //�����ĵ���ֵ��Դ
	int min;  //��Сֵ
	int max;  //���ֵ
	int Step; //ÿ�ΰ����ı������ֵ
	char *Unit; //��λ
	char *MinName;
	char *MaxName; //������������С��ֵ������
	void (*ThingsToDoWhenExit)(void); //ִ���˳�����ʱ�Ĵ���
	}intEditMenuCfg;

typedef struct
	{
	char *SelName; //�ò˵��������
	bool IsChinese; //�Ƿ�Ϊ����
	bool *ValueSource; //ָ�룬����ֵ��Դ
	bool IsPlaceHolder; //�Ƿ�Ϊռλ��
	bool IsMaster;  //��ĸ�˵����ܣ���������ΪMaster�Ķ�������ΪOFFʱ���е�Slave�����޷����༭��һ���˵�ֻ�ܴ���һ��Master
	}BoolListEntryDef;


typedef struct
	{
	const char PassWord[4]; //����(ֻ��������),����LSB�洢
	void (*ThingsToDoWhenEnter)(void); //����ȷ�Ϻ�Ҫ��������
	void (*ThingsToDoWhenExit)(void); //�����˳���Ҫ��������
	}PasswordInputDef;
	
typedef struct
	{
	char *SelName; //�ò˵��������
	bool IsChinese; //�Ƿ�Ϊ����
	int EnumValue; //�ò˵����Ӧ��enumֵ
	bool IsPlaceHolder; //�Ƿ�Ϊռλ��
	}EnumEditEntryDef;

typedef struct
	{
	char *SelName; //�ò˵��������
	bool IsPlaceHolder; //�Ƿ�Ϊռλ��
	bool *IsCanBeSelect; //�Ƿ���Ա�ѡ��
	//����ǰ��Ҫ���еĴ���
  void (*ThingsToDoBeforeEnter)(void);
	//�˳���ص��Ĳ˵�
	}SetupMenuSelDef;	

typedef enum
	{
	MenuType_Custom, //�Զ���˵���ʹ���Զ�����Ⱦ�Ͱ�������
	MenuType_Setup, //������˵���ʹ��ģ��
	MenuType_EnumSetup, //ö����ֵ�༭��˵�
	MenuType_BoolListSetup, //Bool�б������
	}MenuTypeDef;	
	
typedef struct
	{
	MenuTypeDef Type;
	//����list��ı༭
  const BoolListEntryDef *BoolEntry; //����list�����		
	//ö����༭ģʽ�µ�ת������
  const EnumEditEntryDef *Entry;  //����ѡ������		
	int (*ReadEnumSource)(void); //����˵�ʱ��ȡenum�����
	void (*FedEnumToWhat)(int); //��������ʱ��enum��Ŀ��ֵд���ĵĺ���
	//�Զ���ģʽ�µ���Ⱦ����
	void (*CustomMenuRender)(void); //�Զ�����Ⱦ����
	void (*CustomKeyProcess)(void); //�Զ��尴����Ӧ����
	//���ò˵�ģʽ������
	char *MenuTitle; //���ò˵�������
	const SetupMenuSelDef *Sel; //���õ�ѡ��
	void (*AdditionalRender)(void); //�˵��еĶ�����Ⱦ
	void (*ThingsToDoWhenExit)(void); //�˳��˵�����ִ��֮����Ҫ�ɵ�����
	//������˳��ò˵�֮ǰ��Ҫ��������
	void (*ThingsToDoBeforeEnter)(void); 
	void (*ThingsToDoBeforeLeave)(void); //�Զ��尴����Ӧ����	
	}MenuConfigDef;	
	
//�˵�entry
extern const MenuConfigDef SafeAlmMenu;
extern const MenuConfigDef MainMenu;
extern const MenuConfigDef SetMainMenu;
extern const MenuConfigDef PowerSetMenu;	
extern const MenuConfigDef LVSetMenu;
extern const MenuConfigDef DisChgCfgMenu;	
extern const MenuConfigDef ChgSysSetMenu;	
extern const MenuConfigDef CapTestMenu;
extern const MenuConfigDef RSTMainMenu;
extern const MenuConfigDef ResetCTestMenu;
extern const MenuConfigDef ResetSysConfigtMenu;	
extern const MenuConfigDef CapTestHisMenu;	
extern const MenuConfigDef IChargeSetMenu;	
extern const MenuConfigDef PreChargeISetMenu;
extern const MenuConfigDef ChgVSetMenu;	
extern const MenuConfigDef ColHisMenu;
extern const MenuConfigDef ResetColMenu;	
extern const MenuConfigDef ChipStatMenu;	
extern const MenuConfigDef EasySetMainMenu;
extern const MenuConfigDef EnterAdvancedMenu;	
extern const MenuConfigDef PDOCfgMenu;	
extern const MenuConfigDef SecuCfgMenu;	
extern const MenuConfigDef EnterSecuMenu;	
extern const MenuConfigDef AboutMenu;	
extern const MenuConfigDef TSetMenu;	
extern const MenuConfigDef AdapterEmuMenu;	
extern const MenuConfigDef TCResetMenu;
extern const MenuConfigDef DisPlayDirMenu;
extern const MenuConfigDef ActOneShotCTestMenu;
extern const MenuConfigDef RechargeSetMenu;
extern const MenuConfigDef IstopSetMenu;
extern const MenuConfigDef MaxVPDMenu;
extern const MenuConfigDef PowerSetMenuNoEPR;
extern const MenuConfigDef LargeMainMenu;
extern const MenuConfigDef GUIPrefMenu;
extern const MenuConfigDef BalSysSetMenu;
extern const MenuConfigDef BALTestMenu;
extern const MenuConfigDef AutoBALMenu;
extern const MenuConfigDef IstopStdSetMenu;
extern const MenuConfigDef DisChgCfgMenuNoHSCP; //�ŵ�ϵͳ���ã�û�и�ѹHSCP��

//�ڲ���ֵ��������
int IntIncDec(int ValueIN,int Min,int Max,int PerStep); //
float FloatIncDec(float ValueIN,float Min,float Max,float PerStep); //GUI������и����������͵ݼ�
int iroundf(float IN); //��������������Ϊ����

//��������˵�
void PassWordMenuRender(const PasswordInputDef *CFG);
void PasswordEnterInit(void);

//�����༭�˵�
void IntEditMenuKeyEffHandler(void);	//ʵ�ֲ˵�����������Ч�ĺ���
void IntEditHandler(const intEditMenuCfg *CFG); //�����༭�˵���������������Ⱦ�Ͱ���
void IntEditInitHandler(void);	//�����༭����ʱ�Ĵ���
	
//���˵����õĳ���
extern bool AlwaysTrue;
extern bool AlwaysFalse;	
	
//�Լ������
void SelfTestErrorHandler(void);
	
//�ڲ�����
extern bool IsEnableAdvancedMode; //�Ƿ����߼�ģʽ
	
//�ⲿ���õ���ʾ����

void ShowTimeCode(u16 y,long Time);  //���ݴ����������ʾʱ��
void RenderMenuBG(void);	//�˵�������Ⱦ
#define ClearScreen() LCD_Fill(0,0,LCD_W,LCD_H,BLACK)
void SwitchingMenu(const MenuConfigDef *TargetMenuIdx); //�л��˵�
void ShowPostInfo(char Present,char *Msg,char *ID,MessageTypeDef Type); //չʾ�Լ���Ϣ
void MenuRenderProcess(void); //ִ�в˵���Ⱦ
void GUIDelayHandler(void); //GUI��ʱ����
void PostScreenInit(void); //���˵���ʼ��	
	
#endif
