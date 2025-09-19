#ifndef _FASTOP_
#define _FASTOP_

//特殊宏
#define abs(x) x>0?x:x*-1  //求某数的绝对值

//判断是否大于某数的的快捷方式
#define IsLargerThanThreeU16(x) (x&0xFFFC) //位操作判断16bit无符号整数是否大于3
#define IsLargerThanThreeU8(x) (x&0xFC) //位操作判断8bit无符号整数是否大于3
#define IsLargerThanOneU16(x) (x&0xFFFE) //位操作判断16bit无符号整数是否大于1
#define IsLargerThanOneU8(x) (x&0xFE) //位操作判断8bit无符号整数是否大于1

//判断是否小于0的快捷方式
#define IsNegative16(x) (x&0x8000) //使用取符号位进行判断16bit有符号整数是否小于0（比直接比较省空间）
#define IsNegative8(x) (x&0x80)		//使用取符号位进行判断8bit有符号整数是否小于0（比直接比较省空间）

#endif
