#ifndef LEDMgmt_
#define LEDMgmt_

typedef enum
	{
	LED_BothOFF=0x0C,  //Mask位=11，关闭所有LED
	LED_ROnly=0x08,   //Mask位=10，打开红灯
	LED_GOnly=0x04,   //Mask位=01，打开绿灯
	LED_RPlusG=0x00   //Mask位=00，打开所有LED
	}LEDCommandDef;

typedef enum
	{
	LED_OFF=0, //关闭
	//常亮
	LED_Red=1, //红色常亮
	LED_Amber=2, //黄色常亮
	LED_Green=3, //绿色常亮
	//持续闪烁
	LED_RedBlink=4, //红色闪烁
	//一次性快速闪烁
	LED_RedBlinkFifth=5, //红色快闪五次
	LED_GreenBlinkThird=6, //绿色快闪三次
	LED_RedBlinkThird=7 //红色快闪三次
	}LEDStateDef;

//LED亮度配置
#define LEDBrightnessFull 1400 //设置侧按LED的半亮度模式的亮度，范围1-2399对应1-100%	
#define LEDBrightnessHalf 250 //设置侧按LED的半亮度模式的亮度，范围1-2399对应1-100%	

//宏定义
#define IsOneTimeStrobe() LEDMode>LED_RedBlink //判断是否为一次性闪烁的宏（利用了enum值的特性）
	
//外部设置index	
extern volatile LEDStateDef LEDMode;
extern bit IsHalfBrightness; //版亮度模式
	
//LED控制函数
void LED_Init(void);
void LEDControlHandler(void);	
void MakeFastStrobe(LEDStateDef Mode);	

#endif
