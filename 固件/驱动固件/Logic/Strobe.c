#include "ModeControl.h"
#include "Strobe.h"

//�ⲿƵ��Flag
extern volatile bit StrobeFlag;
extern volatile bit LFStrobeFlag;

//�ڲ�����
static bit StrobeFlagSel;
static xdata char StrobeSelIdx; //����ѡ��index
static xdata char StrobeCounter; //����������ʱ

//�ڲ������¼�˳��
static code char StrobeSeq[]={1,3,10,9,7,8,5,11,6,4};

//������������λ
void ResetStrobeModule(void)
	{
	StrobeFlagSel=0;
	StrobeSelIdx=0;
	StrobeCounter=0;
	}

//����״̬������
void RandStrobeHandler(void)
	{
	if(StrobeCounter)StrobeCounter--;
	else
		{
		//װ�ؼ���ֵ
		StrobeCounter=StrobeSeq[StrobeSelIdx];
		StrobeSelIdx=StrobeCounter%sizeof(StrobeSeq);
		//ȡ������flag
		StrobeFlagSel=~StrobeFlagSel;
		}
	}
	
//����Flag�������
bit StrobeOutputHandler(void)
	{
	//���ݱ���flagѡ��һ��Ƶ��
	return StrobeFlagSel?StrobeFlag:LFStrobeFlag;
	}