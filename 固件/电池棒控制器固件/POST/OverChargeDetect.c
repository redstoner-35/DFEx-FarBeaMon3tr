#include "ADC.h"
#include "Config.h"
#include "IP2366_REG.h"
#include <math.h>

//ȫ�ֱ���
bool OCState=false;
static float IBattBuf=0;
static char OCDeAssertCounter=80;

//���¼��ٳ���ѹ
static void DecreseVFull(void)
	{
	CfgData.InputConfig.FullVoltage-=20; //����ѹ����20mV
	if(CfgData.InputConfig.FullVoltage<4100)CfgData.InputConfig.FullVoltage=4100; //������һֱ���µ�������4.1V
	WriteConfiguration(&CfgUnion,false);
	IP2366_UpdateFullVoltage(CfgData.InputConfig.FullVoltage); //���³���ѹ
	}

//������ģ��
void OverChargeDetectModule(void)
	{
	float IDiff;
	bool IsAssert,IsDeAssert;
	//���������ֵ	
	IDiff=fabsf(IBattBuf-ADCO.Ibatt);
	if(ADCO.Ibatt<0||IBattBuf<0)IDiff=0; //������Ϊ0
	//�ж��Ƿ�����Asser����
  if(OCState)IsAssert=false;	 //�Ѿ�������
	else if(ADCO.Ibatt<-0.05)IsAssert=false; //�ŵ�׶�ǿ��Deassert
	else if(ADCO.Vbatt>(4.215*BattCellCount))IsAssert=true;	 //��ص�ѹ���ˣ�ǿ�ƴ���
	else if(IDiff>0.35&&ADCO.Ibatt<0.2)IsAssert=true; //�������˲�����0�ҵ�ǰ�����ӽ�0�Ŵ���	
	else IsAssert=false;
	//������������
	if(IsAssert&&!OCState)
		{
		OCDeAssertCounter=80;
		OCState=true; //���ϵͳ����
		if(ADCO.Vbatt>(4.215*BattCellCount))DecreseVFull(); //��ص�ѹ���ߣ����ٳ���ѹ
		}
	//�ж��Ƿ�����DeAssert����
	if(IsAssert||!OCState)IsDeAssert=false; //�������˳�����
	else if(ADCO.Vbatt>(4.2075*BattCellCount))IsDeAssert=false;	 //��ص�ѹ����
	else if(ADCO.Ibatt>0.3||ADCO.Ibatt<-0.05)IsDeAssert=true; //��ص����������˻��ߴ��ڹ���״̬
	else IsDeAssert=false;
	//�˳���������
	if(IsDeAssert&&OCState)
		{
		if(OCDeAssertCounter>0)OCDeAssertCounter--; //ʱ��û���������״̬
		else OCState=false; //ʱ�䵽�����״̬	
		}
	else if(OCState&&OCDeAssertCounter<80)OCDeAssertCounter++; //�����ǰ���ڹ��䱣��״̬�����˳�������������λ��ʱ��
	}
