#ifndef _FASTOP_
#define _FASTOP_

//特殊宏
#define abs(x) x>0?x:x*-1  //求某数的绝对值

//判断是否小于0的快捷方式
#define IsNegative16(x) x&0x8000 //使用取符号位进行判断16bit整数是否小于0（比直接比较省空间）
#define IsNegative8(x) x&0x80		//使用取符号位进行判断8bit整数是否小于0（比直接比较省空间）

#endif
