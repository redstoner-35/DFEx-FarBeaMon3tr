/************************************************************************************/
/** \file TurboICCMAX.h
/** \Author redstoner_35
/** \Project Xtern Ripper Hyper Boost For GT96
/** \Description 
						 这个文件是驱动固件的极亮和爆闪电流自动配置系统。该文件会根据预定义的LED
						 类型自动配置LED的电流参数以及极亮的电压。
/** \Note 
						 如果你看到编译失败报错指向这个文件，请勿直接修改这个文件！您可以对照输
             出的错误信息在工程设置（左上角魔术棒，找到C51选项卡，在Define这一栏里
						 面）内补上对应的宏定义条目设置好目标的极亮输出电流后报错信息会自行消失。
							
**	History: Initial Release
**	
/*************************************************************************************/
#ifndef _TURBOICCMAX_
#define _TURBOICCMAX_

#define TurboOFFVoltage 3380 //退出极亮的保护电压(mV)

/*************************************************************************************/
/*  Automatic definition Systems - Determinant LED Current at turbo based on config  */
/*************************************************************************************/	

#ifdef Custom_LED_ICCMAX	//使用自定义LED

	//判断电流是否合法
	#if (Custom_LED_ICCMAX < 22000 | Custom_LED_ICCMAX > 36000)
	#error "Error 00C: Turbo Current Value is Out of range"
	#else
  //数值合法，引用
	#warning "Turbo ICC has been set to override mode,Please Verity ICC Settings to avoid destroy your LED."
	#define TurboICCMAX Custom_LED_ICCMAX
	#endif

//FV7212D灯珠
#elif defined(USING_LED_FV7212D)

	#define TurboICCMAX 30300

//FL7022D灯珠
#elif defined(USING_LED_FL7022D)|defined(USING_LED_N7175HE)

	#define TurboICCMAX 33000

//FL7018I灯珠
#elif defined(USING_LED_FL7018I)
	
	#define TurboICCMAX 35000

//Luminus SFT-90X
#elif defined(USING_LED_SFT90X)
	
	#define TurboICCMAX 20000

//旧版12金线NBT160
#elif defined(USING_LED_NBT160)

	#define TurboICCMAX 28000

//专属定制版本加强NBT160
#elif defined(USING_LED_FV7011I)

	#define TurboICCMAX 35500

//安全保护机制，请勿修改！！！！
#else

	#error "Error 00B:Invalid LED Type. You should specify which LED Type you want to use by add following define into project Configuration define line!"
	#error "<USING_LED_FV7212D> or <USING_LED_FL7022D> or <USING_LED_N7175HE> or <USING_LED_FL7018D> or <USING_LED_SFT90> or <USING_LED_NBT160> for existing LED."
	#error "If your LED did not listed,Use 'Custom_LED_ICCMAX=<Turbo Current(mA)>' to define how much current should the driver puts into the LED."

#endif

/*************************************************************************************/
/*  Automatic definition Systems - Determinant LED Current at Strobe based on config */
/*************************************************************************************/	

#ifdef TurboICCMAX
  
	#if defined(USING_LED_FV7011I)
	   //使用FV7011I灯珠，ECO模式限制为25A
      #define ECOTurboICCMAX 25000
  #elif (TurboICCMAX < 32000UL)
	   //极亮电流小于32A模式下的电流定义
     #define ECOTurboICCMAX TurboICCMAX-7500
  #else
	   //经济模式极亮限制在26A
	   #define ECOTurboICCMAX 26000
	#endif
	
	
	//爆闪电流定义
	#ifdef FullPowerStrobe	
			//使用NBT160灯珠，为了保护金线避免金线被炸断，限制爆闪功率至每灯珠120W		
	    #if (defined(USING_LED_FV7011I)|defined(USING_LED_NBT160))
			  
				#define StrobeICCMAX 34000
				
	    //全功率爆闪，极亮电流等于爆闪电流或者爆闪电流使用自定义
	    #elif defined(CustomStrobeCurrent)
				
				#if(CustomStrobeCurrent > 40000 | CustomStrobeCurrent < TurboICCMAX)
			  //非法的自定义爆闪电流
				#error "Error 00D: Customized Strobe Current Must between Turbo Current and less than 40 Amps!"
				
				#else
				
				//合法的自定义爆闪电流，使用设置值
				#define StrobeICCMAX CustomStrobeCurrent
				
				#endif

				
			#else
			  //没有自定义爆闪电流，使用极亮电流
				#define StrobeICCMAX TurboICCMAX
				
	   #endif
		
	#else
   //启用低功率爆闪
	 #if (TurboICCMAX < 22000UL)
				
			#define StrobeICCMAX TurboICCMAX
			#define StrobeIsLessThanTurbo
			
	 #else
			
			#define StrobeICCMAX 22000UL
			
	 #endif	

	#endif
	//信标（脉冲）模式电流定义
	#ifdef FullPowerBeacon
	
	  //使用NBT160灯珠，为了保护金线避免金线被炸断，限制爆闪功率至每灯珠120W		
		#if (defined(USING_LED_FV7011I)|defined(USING_LED_NBT160))
	    #define StrobeLimitedByVTLED
			#define BeaconICCMAX 34000
	
	  #else
			//全功率信标（脉冲）模式，极亮电流等于信标（脉冲）模式电流
			#define BeaconICCMAX TurboICCMAX
	 			
		#endif
	 
	#else
   //启用低功率信标（脉冲）模式
	 #if (TurboICCMAX < 22000UL)
				
			#define BeaconICCMAX TurboICCMAX
			#define BeaconIsLessThanTurbo
	 #else
			
			#define BeaconICCMAX 22000UL
			
	 #endif	

	#endif
/********  爆闪和信标（脉冲）模式电流未定义，随便定义一个避免系统报错 ********/  
#else

	#define BeaconICCMAX 0
  #define StrobeICCMAX 0

#endif  /* TurboICCMAX */

/*************************************************************************************/
/*    Automatic definition Systems - Fail-Safe to Pre-Define a current when fault   */
/*************************************************************************************/	
//定义一个0值避免系统的实际代码部分报错
#ifndef TurboICCMAX
	#define TurboICCMAX 0
#endif

#endif  /* _TURBOICCMAX_ */

/***********************************  End Of File  ***********************************/
