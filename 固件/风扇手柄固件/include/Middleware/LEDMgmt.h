#ifndef LEDMgmt_
#define LEDMgmt_

typedef enum
	{
	LED_OFF=0, //关闭
	LED_Green=1, //绿色常亮
	LED_Red=2, //红色常亮
	LED_RedBlink=3, //红色闪烁
	LED_Amber=4, //黄色常亮
	LED_RedBlink_Fast=5, //红色快闪
	LED_AmberBlinkFast=6, //黄色快速闪烁
	LED_RedBlinkFifth=7, //红色快闪五次
	LED_RedBlinkThird=8, //红色快闪三次
	LED_GreenBlinkThird=9,//绿色快闪三次
	}LEDStateDef;

//外部设置index	
extern volatile LEDStateDef LEDMode;

//内部宏
#define IsOneTimeStrobe() (LEDMode>6)	
	
//LED控制函数
void LED_DeInit(void);
void LED_Init(void);
void LEDControlHandler(void);	
void MakeFastStrobe(LEDStateDef LEDMode);	//制造一次快闪
	
	
#endif
