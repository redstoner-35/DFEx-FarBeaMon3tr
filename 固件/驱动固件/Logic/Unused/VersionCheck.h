#ifndef VersionCheck
#define VersionCheck

typedef enum
	{
	VersionCheck_InAct,
	VersionCheck_StartInit,
	VersionCheck_ShowNumber,
	VersionCheck_ShowNumberWait,
	VersionCheck_LoadNextNumber,
	VersionCheck_WaitUserRelease,
	}VersionChkFSMDef;

//外部参考
extern xdata VersionChkFSMDef VChkFSMState;	
	
//函数
void VersionCheck_TIMHandler(void);
void VersionCheck_Trigger(void);
char VersionCheckFSM(void);	
	
#endif
