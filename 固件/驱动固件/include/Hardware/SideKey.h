/************************************************************************************/
/** \file SideKey.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 这个头文件为系统多模态按键识别模块的外部声明文件，负责声明按键控制器的初
始化、特殊配置以及获取按键事件的函数。

**	History: Initial Release
**	
/************************************************************************************/
#ifndef _SideKey_
#define _SideKey_
/************************************************************************************/
/* Extern Functions definition - Initialization */
/************************************************************************************/
void SideKeyInit(void);

/************************************************************************************/
/* Extern Functions definition - Specialized Key Controller Operation */
/************************************************************************************/
void MarkAsKeyPressed(void); //标记按键按下
void SideKey_SetIntOFF(void);		//关闭侧按的GPIO中断

/************************************************************************************/
/* Extern Functions definition - Event Management and Query */
/************************************************************************************/
bit GetSideKeyRawGPIOState(void); //复位计时器
char getSideKeyShortPressCount(void);//获取侧按按键的单击和连击次数
bit getSideKeyLongPressEvent(void);//获得侧按按钮长按2秒的事件
bit getSideKeyHoldEvent(void);//获得侧按按钮是否在保持一直按住的事件
bit IsKeyEventOccurred(void); //检测是否有按键事件发生
char getSideKeyNClickAndHoldEvent(void); //获取侧按按下N次+长按的按键数
void ClearShortPressEvent(void); //清除累计的短按事件
bit getSideKey1HEvent(void); //获取侧按按键是否执行了单击+长按事件

/************************************************************************************/
/* Extern Functions definition - Call Back For KeyEvent Handler */
/************************************************************************************/
void SideKey_TIM_Callback(void);//连按检测计时的回调处理
void SideKey_LogicHandler(void);//逻辑处理

#endif /* _SideKey_ */

/*********************************  End Of File  ************************************/
