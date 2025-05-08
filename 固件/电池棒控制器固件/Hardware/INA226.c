#include "PMBUS.h"
#include "INA226.h"
#include "delay.h"
#include "I2CAddr.h"

//初始化INA226
//返回枚举类型
INA226InitStatDef INA226_INIT(INAinitStrdef * INAConf)
 {
  unsigned int ConfREG=0x0000;//默认寄存器为0
	double shuntResValue;//浮点数，表示检流电阻的阻值
	unsigned int buf;
	unsigned int CalREG;
	char Retrycount=0;
	//首先进行复位
  do{
		 //写数据
		 buf=0xC127;
	   PMBUS_WordReadWrite(true,true,&buf,INA226ADDR,0);//往寄存器内写默认配置
	   delay_ms(1);
		 //读取数据
		 buf=0;
		 Retrycount++;//重试次数+1
	   if(!PMBUS_WordReadWrite(true,false,&buf,INA226ADDR,0))continue;//读取失败
		 if(buf==0x4127)break;//验证通过
	  }while(Retrycount<5);	 
  //校验复位是否成功
  if(Retrycount>=5)return A226_Error_SMBUS_NACK;//如果尝试了很多次复位还是失败则退出
  //首先设置转换的平均次数
	ConfREG=(unsigned int)(INAConf->AvgCount&0x07)<<9; //设置AVG[2:0]
  //第二步，设置VBUS的转换时间
	ConfREG|=(unsigned int)(INAConf->VBUSConvTime&0x07)<<6; //设置VBUSCT[2:0]
	//第三步，设置分流器的转换时间
	ConfREG|=(unsigned int)(INAConf->IBUSConvTime&0x07)<<3;	//设置VSHCT[2:0]
	//最后一步，写入模式配置bit
	ConfREG&=0xFFF8;
	ConfREG|=((unsigned int)INAConf->ConvMode)&0x07;
	//将配置bit写入进配置寄存器并且校验
	PMBUS_WordReadWrite(true,true,&ConfREG,INA226ADDR,0x0);//写入
	delay_ms(1);
	buf=0;
	PMBUS_WordReadWrite(true,false,&buf,INA226ADDR,0);
	buf&=0xFFF;	
	ConfREG&=0xFFF; //我这边关心的位是D11-D0，其他位不关心所以Mask掉
	if(buf!=ConfREG)return A226_Error_ProgramReg;//校验，如果写进去不对则报错
	//比较制造商ID
	PMBUS_WordReadWrite(true,false,&buf,INA226ADDR,0xFE);
	if(buf!=0x5449)return A226_Error_NotGenuineDevice; //供应商ID对不上，返回非官方设备提示
	PMBUS_WordReadWrite(true,false,&buf,INA226ADDR,0xFF);
  buf&=0xFFF0; //Mask掉RevID只保留die ID
	if(buf!=0x2260)return A226_Error_NotGenuineDevice; //读回来的DIE ID对不上，非官方设备
	//计算出校准寄存器的值
	shuntResValue=(double)(INAConf->ShuntValue)/(double)1000;//将电阻值单位从毫欧转换为欧
	shuntResValue=(double)0.00512/(CurrentLSB*shuntResValue);//计算出校准寄存器的内容并且强制取整
	//检查计算好的校验值是否合法
  CalREG=(unsigned int)shuntResValue;//取整	
  if(CalREG&0x8000)return A226_Error_CalibrationReg;//检查计算出的校准值是否超过65536，如果超了则返回错误类型
	//把校准数据写进INA226
	PMBUS_WordReadWrite(true,true,&CalREG,INA226ADDR,0x5);//写入
	delay_ms(1);
	buf=0;
	PMBUS_WordReadWrite(true,false,&buf,INA226ADDR,5);
	buf&=0x7FFF;
	CalREG&=0x7FFF; //校准寄存器我只关心Bit14-0，其他位Mask掉
	if(buf!=CalREG)return A226_Error_ProgramCalReg;//校验，如果写进去不对则报错
	//设置报警寄存器
	CalREG=(unsigned int)INAConf->AlertConfig; //应用bitfield(Alert[4:0])
	if(INAConf->IsEnableAlertLatch)CalREG|=0x01; //如果使能报警锁存功能则令LEN=1
	if(INAConf->IsAlertPinInverted)CalREG|=0x02; //如果使能报警输出极性反转功能责令APOL=1
	if(INAConf->AlertConfig!=A226_AlertDisable)CalREG|=0x400;  //如果使能报警功能则设置CNVR=1，打开Alert Pin输出
	//写Mask/Enable寄存器配置告警设置
	if(!PMBUS_WordReadWrite(true,true,&CalREG,INA226ADDR,0x6))return A226_Error_SetAlertCfg;
  //这些步骤全部完成，函数结束
	return A226_Init_OK;
 }
 
//设置报警寄存器的数值
bool INA226_SetAlertRegister(unsigned int Value)
	{
	unsigned int regtemp;
	regtemp=Value&0xFFFF;
	if(!PMBUS_WordReadWrite(true,false,&regtemp,INA226ADDR,0x07))return false;
	//操作完毕返回true
	return true;
	}
 
//检查INA226是否能顺利开启转换
bool INA226_QueueIfGaugeCanReady(void)
	{
	unsigned int regtemp;
	//读取Mask&Enable寄存器判断OVF是否等于1
	if(!PMBUS_WordReadWrite(true,false,&regtemp,INA226ADDR,0x06))return false;
	if(regtemp&0x8)return true;//CNVR=1,转换完成
  //否则转换失败
	return false;
	}	
	
 //读取数值
bool INA226_GetBusInformation(INADoutSreDef *INADout)
 {
 unsigned int regtemp;
 signed int calctemp;
 //读取Mask&Enable寄存器判断OVF是否等于1
 if(!PMBUS_WordReadWrite(true,false,&regtemp,INA226ADDR,0x06))return false;
 if(regtemp&0x4)//OVF=1，数据溢出
	 {
	 INADout->BusCurrent=0;
	 INADout->BusVolt=0;
	 INADout->BusPower=0;//数据无效
	 } 
	//开始获取数据
  else
	 {
	 //转换电压数据
	 if(!PMBUS_WordReadWrite(true,false,&regtemp,INA226ADDR,0x02))return false;  //读取VBUS寄存器
	 INADout->BusVolt=(float)((float)regtemp*(float)BusVoltLSB/(float)1000); //计算出电压
	 //读取电流功率
	 if(!PMBUS_WordReadWrite(true,false,&regtemp,INA226ADDR,0x04))return false;
	 if(regtemp&0x8000)//如果符号位为1说明电流值是负数
			calctemp=regtemp|0xFFFF8000;
		else 
			calctemp=regtemp&0x7FFF;//过滤掉最高位
		INADout->BusCurrent=(float)((float)calctemp*CurrentLSB);//按照设置的LSB计算出电流
		if(!PMBUS_WordReadWrite(true,false,&regtemp,INA226ADDR,0x03))return false;
		INADout->BusPower=(float)((float)regtemp*PowerLSB);//计算出功率，LSB是电流值*20(单位W)			
		} 
 //操作完毕，返回true
 return true;
 }
