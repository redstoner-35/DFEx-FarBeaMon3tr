#include "ht32.h"
#include "Key.h"
#include "Pindefs.h"
#include "delay.h"

//��������ֵ����
#define iabs(x) x>0?x:-x

//�ڲ���ȫ�ֱ���
extern short SleepTimer;
KeyTimerDef KeyTimer[2]; //������ʱ����[0]=�ϣ�[1]=��
KeyStateStor KeyState; //����״̬�洢

//�ಿ������ʱ������
void SideKey_TIMCallback(void)
	{
	char i;
	KeyTimerDef *Timer;
	//ѭ������������ʱ��
	if(SleepTimer>0)SleepTimer--; //����ʱ
	for(i=0;i<2;i++)
		{
		Timer=&KeyTimer[i]; //ȡ��ַ
		if(Timer->EnablePressTimer&&Timer->PressTime<HoldTime)Timer->PressTime++;
		else if(!Timer->EnablePressTimer)Timer->PressTime=0;
		}
	}

//�ಿ�����жϴ���
void SideKey_IntCallback(void)
	{
	//���ϼ�����
	if(EXTI_GetEdgeFlag(KeyUp_EXTI_CHANNEL))
		{
    EXTI_ClearEdgeFlag(KeyUp_EXTI_CHANNEL);
    KeyTimer[0].EnablePressTimer=true; //������ʱ��
    EXTI_IntConfig(KeyUp_EXTI_CHANNEL,DISABLE); //�����ж�
		KeyState.KeyShift[0]=0x0000;
		}
	//���¼�����
	if(EXTI_GetEdgeFlag(KeyDown_EXTI_CHANNEL))
		{
		EXTI_ClearEdgeFlag(KeyDown_EXTI_CHANNEL);
    KeyTimer[1].EnablePressTimer=true; //������ʱ��
    EXTI_IntConfig(KeyDown_EXTI_CHANNEL,DISABLE); //�����ж�
		KeyState.KeyShift[1]=0x0000;
		}
	}	
	
//�ఴ�߼�����
void SideKey_LogicHandler(void)
	{
	//���ϼ�����
	if(KeyTimer[0].EnablePressTimer)
		{
		delay_ms(4);
		//��λȥ���ȴ��ɿ�
		SleepTimer=480; //����ʱ�临λΪһ����
		KeyState.KeyShift[0]<<=1;
		if(GPIO_ReadInBit(KeyUp_IOG,KeyUp_IOP)==SET)KeyState.KeyShift[0]++;
		else KeyState.KeyShift[0]&=0xFFFE;
		//�������ɿ�
		if(KeyState.KeyShift[0]==0xFFFF)
			{
			KeyTimer[0].EnablePressTimer=false;
			if(KeyState.KeyEvent==KeyEvent_None&&!KeyState.IsUpHold)KeyState.KeyEvent=KeyEvent_Up; //������ʾΪUp
			KeyState.IsUpHold=false; //�����ɿ����false
			EXTI_ClearEdgeFlag(KeyUp_EXTI_CHANNEL);
			EXTI_IntConfig(KeyUp_EXTI_CHANNEL, ENABLE); //���ö�Ӧ�İ����ж�	
			}
		}
	//���¼�����
	if(KeyTimer[1].EnablePressTimer)
		{
		delay_ms(4);
		SleepTimer=480; //����ʱ��һ����
		//��λȥ���ȴ��ɿ�
		KeyState.KeyShift[1]<<=1;
		if(GPIO_ReadInBit(KeyDown_IOG,KeyDown_IOP)==SET)KeyState.KeyShift[1]++;
		else KeyState.KeyShift[1]&=0xFFFE;
		//�������ɿ�
		if(KeyState.KeyShift[1]==0xFFFF)
			{
			KeyTimer[1].EnablePressTimer=false;
			if(KeyState.KeyEvent==KeyEvent_None&&!KeyState.IsDownHold)KeyState.KeyEvent=KeyEvent_Down; //������ʾΪDown
			KeyState.IsDownHold=false; //�����ɿ����flag
		  EXTI_ClearEdgeFlag(KeyDown_EXTI_CHANNEL);
			EXTI_IntConfig(KeyDown_EXTI_CHANNEL, ENABLE); //���ö�Ӧ�İ����ж�	
			}
		}	
	//���ϼ������㹻ʱ��
	if(KeyTimer[0].PressTime==HoldTime)
		{
		KeyState.IsUpHold=true;
		if(!KeyTimer[1].EnablePressTimer)KeyState.KeyEvent=KeyEvent_Enter; //ֻ��һ���������£��ж�Ϊȷ��
		else if(iabs(KeyTimer[0].PressTime-KeyTimer[1].PressTime)<3)KeyState.KeyEvent=KeyEvent_BothEnt; //���������������Ұ��µ�ʱ�伸��һ�£��ж�Ϊͬʱ����
		KeyTimer[0].PressTime++; //ʱ���1ȷ������ֻ��һ��
		}
	//���¼������㹻ʱ��	
	else if(KeyTimer[1].PressTime==HoldTime)
		{
		KeyState.IsDownHold=true;
		if(!KeyTimer[0].EnablePressTimer)KeyState.KeyEvent=KeyEvent_ESC; //ֻ��һ���������£��ж�Ϊ�˳�
		KeyTimer[1].PressTime++; //ʱ���1ȷ������ֻ��һ��
		}		
	}
	
//�ಿ��������
void SideKey_Init(void)
	{
	EXTI_InitTypeDef EXTI_InitStruct;
	char i;
	//�����ⲿ�ж�ϵͳ��������
	EXTI_InitStruct.EXTI_Debounce = EXTI_DEBOUNCE_ENABLE; 
  EXTI_InitStruct.EXTI_DebounceCnt = 5;  //����ȥ��
  EXTI_InitStruct.EXTI_IntType = EXTI_NEGATIVE_EDGE; //�����ش���
	//�������ϰ�����GPIO
  AFIO_GPxConfig(KeyUp_IOB,KeyUp_IOP, AFIO_FUN_GPIO);//GPIO����
  GPIO_DirectionConfig(KeyUp_IOG,KeyUp_IOP,GPIO_DIR_IN);//����Ϊ����
	GPIO_PullResistorConfig(KeyUp_IOG,KeyUp_IOP,GPIO_PR_UP); //�����ڲ�����
	GPIO_InputConfig(KeyUp_IOG,KeyUp_IOP,ENABLE);//����IDR
	//�������ϰ������ж�
	AFIO_EXTISourceConfig(KeyUp_IOPN,KeyUp_IOB); //�����ж�Դ
	EXTI_InitStruct.EXTI_Channel = KeyUp_EXTI_CHANNEL; //ͨ��ѡ��Ϊ��Ӧ��ͨ��
  EXTI_Init(&EXTI_InitStruct);  
  EXTI_IntConfig(KeyUp_EXTI_CHANNEL, ENABLE); //���ö�Ӧ�İ����ж�	
  NVIC_EnableIRQ(KeyUp_EXTI_IRQn); //����IRQ
	//�������°�����GPIO
  AFIO_GPxConfig(KeyDown_IOB,KeyDown_IOP, AFIO_FUN_GPIO);//GPIO����
  GPIO_DirectionConfig(KeyDown_IOG,KeyDown_IOP,GPIO_DIR_IN);//����Ϊ����
	GPIO_PullResistorConfig(KeyDown_IOG,KeyDown_IOP,GPIO_PR_UP); //�����ڲ�����
	GPIO_InputConfig(KeyDown_IOG,KeyDown_IOP,ENABLE);//����IDR
	//�������ϰ������ж�
	AFIO_EXTISourceConfig(KeyDown_IOPN,KeyDown_IOB); //�����ж�Դ
	EXTI_InitStruct.EXTI_Channel = KeyDown_EXTI_CHANNEL; //ͨ��ѡ��Ϊ��Ӧ��ͨ��
  EXTI_Init(&EXTI_InitStruct);  
  EXTI_IntConfig(KeyDown_EXTI_CHANNEL, ENABLE); //���ö�Ӧ�İ����ж�	
  NVIC_EnableIRQ(KeyDown_EXTI_IRQn); //����IRQ	
	//��ʼ����ʱ��
	for(i=0;i<2;i++)KeyTimer[i].EnablePressTimer=false;
  //��ʼ�������ṹ��
  KeyState.IsDownHold=false;
	KeyState.IsUpHold=false;
	KeyState.KeyEvent=KeyEvent_None;
	for(i=0;i<2;i++)KeyState.KeyShift[i]=0xFFFF;
	}
