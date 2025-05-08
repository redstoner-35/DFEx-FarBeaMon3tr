#ifndef _I2CADDR_
#define _I2CADDR_

/****************************************************
这是系统中所有I2C从器件的地址。您可以根据需要去更改。
需要注意的是，这里写的所有地址都是设R/W位=0的8位地址。
地址格式如下：

MSB  [A7 A6 A5 ... A3 A2 A1 R/W(=0)]  LSB

*****************************************************/

#define IP2366ADDR 0xEA //IP2366的地址
#define M24C512ADDR 0xA0  //64K EEPROM负责存储系统的配置参数
#define M24C512SecuADDR 0xB0 //FM24C512独有的安全sector的地址
#define SMIOADDR 0x82 //SMBUS GPIO的地址
#define INA226ADDR 0x80 //负责Type-C功率测量的INA226的地址

#endif
