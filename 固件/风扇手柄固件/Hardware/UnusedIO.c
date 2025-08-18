#include "GPIO.h"
#include "PinDefs.h"
#include "cms8s6990.h"


void MaskUnusedIO(void)
	{
	//Mask掉无用的IO强制输出0	
	P0TRIS|=P0UnusedPIN;
	P1TRIS|=P1UnusedPIN;
	P2TRIS|=P2UnusedPIN;
	P3TRIS|=P3UnusedPIN;
	//所有无用的IO强制输出0
	P0&=~P0UnusedPIN;
	P1&=~P1UnusedPIN;
	P2&=~P2UnusedPIN;
	P3&=~P3UnusedPIN;
	//所有对外IO关闭上下拉电阻
	P0UP&=~P0UnusedPIN;
	P0RD&=~P0UnusedPIN;
	
	P1UP&=~P1UnusedPIN;
	P1RD&=~P1UnusedPIN;
		
	P2UP&=~P2UnusedPIN;
	P2RD&=~P2UnusedPIN;
		
	P3UP&=~P3UnusedPIN;
	P3RD&=~P3UnusedPIN;
	}