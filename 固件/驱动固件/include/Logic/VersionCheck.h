#ifndef VersionCheck
#define VersionCheck

typedef enum
	{
	VersionCheck_InAct,
	VersionCheck_StartInit,
	VersionCheck_ShowNumber,
	VersionCheck_ShowNumberWait,
	VersionCheck_LoadNextNumber,
	}VersionChkFSMDef;

//外部参考
extern xdata VersionChkFSMDef VChkFSMState;	
	
//函数
void VersionCheck_Trigger(void);
char VersionCheckFSM(void);	
	
#endif
