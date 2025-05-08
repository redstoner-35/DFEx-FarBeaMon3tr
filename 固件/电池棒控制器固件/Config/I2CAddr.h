#ifndef _I2CADDR_
#define _I2CADDR_

/****************************************************
����ϵͳ������I2C�������ĵ�ַ�������Ը�����Ҫȥ���ġ�
��Ҫע����ǣ�����д�����е�ַ������R/Wλ=0��8λ��ַ��
��ַ��ʽ���£�

MSB  [A7 A6 A5 ... A3 A2 A1 R/W(=0)]  LSB

*****************************************************/

#define IP2366ADDR 0xEA //IP2366�ĵ�ַ
#define M24C512ADDR 0xA0  //64K EEPROM����洢ϵͳ�����ò���
#define M24C512SecuADDR 0xB0 //FM24C512���еİ�ȫsector�ĵ�ַ
#define SMIOADDR 0x82 //SMBUS GPIO�ĵ�ַ
#define INA226ADDR 0x80 //����Type-C���ʲ�����INA226�ĵ�ַ

#endif
