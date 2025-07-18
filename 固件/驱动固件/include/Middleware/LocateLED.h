#ifndef _LOCLED_
#define _LOCLED_

typedef enum
	{
	LocateLED_NotEdit=0,
	LocateLED_WaitKeyRelease=1,
	LocateLED_Sel=2,
	LocateLED_SelFading=3
	
	}LocLEDEditDef;

//函数
void LocateLED_Enable(void);
LEDStateDef LocateLED_ShowType(void);
char LocateLED_Edit(void);
void LocateLED_TIMHandler(void);	
void InitLocateLEDEditSys(void);	
	
//外部参考
extern xdata LocLEDEditDef LocLEDState;
	
#endif
