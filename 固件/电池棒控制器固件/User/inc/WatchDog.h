#ifndef _WD_
#define _WD_

//���ز���
#define WDTReloadSec 2

//ι���궨��
#define WatchDog_Feed() WDT_Restart()

//����
void WatchDog_Init(void); //�������Ź�ģ��

#endif
