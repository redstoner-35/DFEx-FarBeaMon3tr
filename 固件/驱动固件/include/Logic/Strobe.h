#ifndef _Strobe_
#define _Strobe_

//外部参考
extern bit EnableRandomStrobe;

//函数
void RandStrobeHandler(void);
bit StrobeOutputHandler(void);
void ResetStrobeModule(void);

#endif
