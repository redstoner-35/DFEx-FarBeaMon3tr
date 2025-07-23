#ifndef _SMENU_
#define _SMENU_

typedef enum
	{
	SetupMenu_InACT=0,
	SetupMenu_InactiveExitWait=1,
	SetupMenu_Prepare,
	SetupMenu_ShowContent,
	SetupMenu_WaitShowContent,
	SetupMenu_SubmitContent,
	
	//位域编辑
	SetupMenu_BitFieldEdit
	}SetupMenuFSMDef;

//内部参数定义
#define TotalSetupNum 8	
	
	
//外部参考
extern xdata SetupMenuFSMDef SetupFSMState;
	
//函数
LEDStateDef SetupMenuFSM(void);
void TriggerSetupMenuDisplay(void);

#endif
