#ifndef _Key_
#define _Key_

#include <stdbool.h>

//������ʱ
#define HoldTime 10 //����ʱ��

//������ʱ���ṹ��
typedef struct
	{
	char PressTime;
	bool EnablePressTimer;
	}KeyTimerDef;
	
typedef enum
	{
	KeyEvent_None,
	KeyEvent_Up,     //����
	KeyEvent_Down,   //����
	KeyEvent_Enter,  //ȷ��
	KeyEvent_ESC,     //�˳�
	KeyEvent_BothEnt, //����ͬʱ��ס
	}KeyEventDef;	

	
typedef struct
	{
	unsigned short KeyShift[2]; //������ȥ���Ĵ���
	bool IsUpHold; //���ϼ��Ƿ������ס
	bool IsDownHold; //���¼��Ƿ������ס
	KeyEventDef KeyEvent; //�����¼�
	}KeyStateStor;	
	
/*�����ϰ������Զ�Define���������޸ģ�*/
#define KeyUp_IOB STRCAT2(GPIO_P,KeyUp_IOBank)
#define KeyUp_IOG STRCAT2(HT_GPIO,KeyUp_IOBank)
#define KeyUp_IOP STRCAT2(GPIO_PIN_,KeyUp_IOPN) 	
	
#define KeyUp_EXTI_CHANNEL  STRCAT2(EXTI_CHANNEL_,KeyUp_IOPN)
#define _KeyUp_EXTI_IRQn STRCAT2(EXTI,KeyUp_IOPN)
#define KeyUp_EXTI_IRQn  STRCAT2(_KeyUp_EXTI_IRQn,_IRQn)

/*�����°������Զ�Define���������޸ģ�*/
#define KeyDown_IOB STRCAT2(GPIO_P,KeyDown_IOBank)
#define KeyDown_IOG STRCAT2(HT_GPIO,KeyDown_IOBank)
#define KeyDown_IOP STRCAT2(GPIO_PIN_,KeyDown_IOPN) 	
	
#define KeyDown_EXTI_CHANNEL  STRCAT2(EXTI_CHANNEL_,KeyDown_IOPN)
#define _KeyDown_EXTI_IRQn STRCAT2(EXTI,KeyDown_IOPN)
#define KeyDown_EXTI_IRQn  STRCAT2(_KeyDown_EXTI_IRQn,_IRQn)	

//�ص�����
void SideKey_TIMCallback(void);	//�ಿ������ʱ������
void SideKey_IntCallback(void); //�ಿ�����жϴ���
void SideKey_LogicHandler(void); //�߼�����

//�ⲿ����
extern KeyStateStor KeyState; //�������´���

//����
void SideKey_Init(void); //�ఴ��ʼ��

#endif
