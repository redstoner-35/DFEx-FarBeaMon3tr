#ifndef _LOCLED_
#define _LOCLED_

typedef enum
	{
	LocateLED_NotEdit,
	LocateLED_Sel,
	LocateLED_WaitKeyRelease
	}LocLEDEditDef;

//函数
void LocateLED_Enable(void);
LEDStateDef LocateLED_ShowType(void);
char LocateLED_Edit(char ClickCount);
void LocateLED_TIMHandler(void);	
	
//外部参考
extern xdata LocLEDEditDef LocLEDState;
	
#endif
