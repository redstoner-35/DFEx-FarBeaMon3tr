#ifndef _TURBOICCMAX_
#define _TURBOICCMAX_

/*************************************************************
这个文件是新增的驱动固件的极亮和爆闪电流自动配置系统。如果你看
到编译失败报错指向这个文件，请勿直接修改这个文件！您可以对照输
出的错误信息在工程设置（左上角魔术棒，找到C51选项卡，在Define
这一栏里面）内补上对应的宏定义条目设置好目标的极亮输出电流后报
错信息会自行消失。
*************************************************************/

//使用自定义LED
#ifdef Custom_LED_ICCMAX

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

	#message "Currently Selected LED is DFEx_SuperLED+ FV7212D,Turbo ICC=30.3A."
	#define TurboICCMAX 30300

//FL7022D灯珠
#elif defined(USING_LED_FL7022D)|defined(USING_LED_N7175HE)

	#message "Currently Selected LED is DFEx_SuperLED+ FL7022D(NightWatch N7-175HE),Turbo ICC=33.0A."
	#define TurboICCMAX 33000


#elif defined(USING_LED_FL7018I_PRO9)
	
	#message "Currently Selected LED is DFEx_SuperLED+ FL7018I with 0.9mR Shunt,Turbo ICC=36.6A."
	#define TurboICCMAX 36600  

//FL7018I灯珠
#elif defined(USING_LED_FL7018I)
	
	#message "Currently Selected LED is DFEx_SuperLED+ FL7018I,Turbo ICC=35.2A."
	#define TurboICCMAX 35200

//Luminus SFT-90X
#elif defined(USING_LED_SFT90X)
	
	#message "Currently Selected LED is Luminus SFT-90X-WS65M,Turbo ICC=20A."
	#define TurboICCMAX 20000


#elif defined(USING_LED_NBT160)

	#message "Currently Selected LED is NBT160.3(7070 Package),Turbo ICC=29A."
	#define TurboICCMAX 29000

//安全保护机制，请勿修改！！！！
#else

	#error "Error 00B:Invalid LED Type. You should specify which LED Type you want to use by add following define into project Configuration define line!"
	#error "<USING_LED_FV7212D> or <USING_LED_FL7022D> or <USING_LED_N7175HE> or <USING_LED_FL7018D> or <USING_LED_SFT90> or <USING_LED_NBT160> for existing LED."
	#error "If your LED did not listed,Use 'Custom_LED_ICCMAX=<Turbo Current(mA)>' to define how much current should the driver puts into the LED."

#endif


/********  爆闪和信标（脉冲）模式以及ECO模式电流定义区域 ********/  
#ifdef TurboICCMAX
  
  //竞技模式下的电流定义
  #if (TurboICCMAX < 32000UL)
     #define ECOTurboICCMAX TurboICCMAX-7000
  #else
	   //经济模式极亮限制在26A
	   #define ECOTurboICCMAX 26000
	#endif
	
	
	//爆闪电流定义
	#ifdef FullPowerStrobe
	    //全功率爆闪，极亮电流等于爆闪电流或者爆闪电流使用自定义
	    #ifdef CustomStrobeCurrent
				
				#if(CustomStrobeCurrent > 40000 | CustomStrobeCurrent < TurboICCMAX)
			  //非法的自定义爆闪电流
				#error "Error 00D: Customized Strobe Current Must between Turbo Current and less than 40 Amps!"
				
				#else
				
				//合法的自定义爆闪电流，使用设置值
				#define StrobeICCMAX CustomStrobeCurrent
				#message "Customized Strobe Current has been set.System will use Customized Strobe Current instead of turbo current."
				
				#endif
			#else
			  //没有自定义爆闪电流，使用极亮电流
				#define StrobeICCMAX TurboICCMAX
	 
				#if (TurboICCMAX < 22000UL)
				#warning "Tips:Strobe Current has been limited due to Turbo ICCMAX is less than 22A."
				#endif
			
	   #endif
		
	#else
   //启用低功率爆闪
	 #if (TurboICCMAX < 22000UL)
				
			#define StrobeICCMAX TurboICCMAX
			#warning "Tips:Strobe Current has been limited due to Turbo ICCMAX is less than 22A."
			
	 #else
			
			#message "Low Power Strobe mode Enabled.Strobe ICC will limit to 22Amps."
			#define StrobeICCMAX 22000UL
			
	 #endif	

	#endif
	//信标（脉冲）模式电流定义
	#ifdef FullPowerBeacon
   //全功率信标（脉冲）模式，极亮电流等于信标（脉冲）模式电流
	 #define BeaconICCMAX TurboICCMAX
	 
			#if (TurboICCMAX < 22000UL)
			#warning "Tips:Beacon/Pulsing mode Current has been limited due to Turbo ICCMAX is less than 22A."
			#endif
	 
	#else
   //启用低功率信标（脉冲）模式
	 #if (TurboICCMAX < 22000UL)
				
			#define BeaconICCMAX TurboICCMAX
			#warning "Tips:Beacon/Pulsing mode Current has been limited due to Turbo ICCMAX is less than 22A."
			
	 #else
			
			#message "Low Power Beacon/Pulsing mode Enabled.Beacon/Pulsing mode ICC will limit to 22Amps."
			#define BeaconICCMAX 22000UL
			
	 #endif	

	#endif
/********  爆闪和信标（脉冲）模式电流未定义，随便定义一个避免系统报错 ********/  
#else

	#define BeaconICCMAX 0
  #define StrobeICCMAX 0

#endif

//定义一个0值避免系统的实际代码部分报错
#ifndef TurboICCMAX
	#define TurboICCMAX 0
#endif

#endif