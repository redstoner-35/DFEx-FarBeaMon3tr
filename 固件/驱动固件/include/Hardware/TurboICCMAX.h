#ifndef _TURBOICCMAX_
#define _TURBOICCMAX_

/*************************************************************
����ļ��������������̼��ļ����ͱ��������Զ�����ϵͳ������㿴
������ʧ�ܱ���ָ������ļ�������ֱ���޸�����ļ��������Զ�����
���Ĵ�����Ϣ�ڹ������ã����Ͻ�ħ�������ҵ�C51ѡ�����Define
��һ�����棩�ڲ��϶�Ӧ�ĺ궨����Ŀ���ú�Ŀ��ļ������������
����Ϣ��������ʧ��
*************************************************************/


//ʹ���Զ���LED
#ifdef Custom_LED_ICCMAX

	//�жϵ����Ƿ�Ϸ�
	#if (Custom_LED_ICCMAX < 20000 | Custom_LED_ICCMAX > 36000)
	#error "Error 00C: Turbo Current Value is Out of range"
	#else
  //��ֵ�Ϸ�������
	#warning "Turbo ICC has been set to override mode,Please Verity ICC Settings to avoid destroy your LED."
	#define TurboICCMAX Custom_LED_ICCMAX
	#endif

//FV7212D����
#elif defined(USING_LED_FV7212D)

	#warning "Currently Selected LED is DFEx_SuperLED+ FV7212D,Turbo ICC=30A."
	#define TurboICCMAX 30000

//FL7022D����
#elif defined(USING_LED_FL7022D)|defined(USING_LED_N7175HE)

	#warning "Currently Selected LED is DFEx_SuperLED+ FL7022D(NightWatch N7-175HE),Turbo ICC=33.5A."
	#define TurboICCMAX 33500

//FL7018I����
#elif defined(USING_LED_FL7018I)
	
	#warning "Currently Selected LED is DFEx_SuperLED+ FL7018I,Turbo ICC=34A."
	#define TurboICCMAX 34000

//Luminus SFT-90
#elif defined(USING_LED_SFT90)
	
	#warning "Currently Selected LED is Luminus SFT_90,Turbo ICC=20A."
	#define TurboICCMAX 20000


#elif defined(USING_LED_NBT160)

	#warning "Currently Selected LED is NBT160.3(7070 Package),Turbo ICC=28A."
	#define TurboICCMAX 28000

//��ȫ�������ƣ������޸ģ�������
#else

	#error "Error 00B:Invalid LED Type. You should specify which LED Type you want to use by add following define into project Configuration define line!"
	#error "<USING_LED_FV7212D> or <USING_LED_FL7022D> or <USING_LED_N7175HE> or <USING_LED_FL7018D> or <USING_LED_SFT90> or <USING_LED_NBT160> for existing LED."
	#error "If your LED did not listed,Use 'Custom_LED_ICCMAX=<Turbo Current(mA)>' to define how much current should the driver puts into the LED."

#endif


/********  �������ű꣨���壩ģʽ������������ ********/  
#ifdef TurboICCMAX
	//������������
	#ifdef FullPowerStrobe
   //ȫ���ʱ����������������ڱ�������
	 #define StrobeICCMAX TurboICCMAX
	 
			#if (TurboICCMAX < 22000)
			#warning "Tips:Strobe Current has been limited due to Turbo ICCMAX is less than 22A."
			#endif
	 
	#else
   //���õ͹��ʱ���
	 #if (TurboICCMAX < 22000)
				
			#define StrobeICCMAX TurboICCMAX
			#warning "Tips:Strobe Current has been limited due to Turbo ICCMAX is less than 22A."
			
	 #else
			
			#warning "Low Power Strobe mode Enabled.Strobe ICC will limit to 22Amps."
			#define StrobeICCMAX 22000
			
	 #endif	

	#endif
	//�ű꣨���壩ģʽ��������
	#ifdef FullPowerBeacon
   //ȫ�����ű꣨���壩ģʽ���������������ű꣨���壩ģʽ����
	 #define BeaconICCMAX TurboICCMAX
	 
			#if (TurboICCMAX < 22000)
			#warning "Tips:Beacon/Pulsing mode Current has been limited due to Turbo ICCMAX is less than 22A."
			#endif
	 
	#else
   //���õ͹����ű꣨���壩ģʽ
	 #if (TurboICCMAX < 22000)
				
			#define BeaconICCMAX TurboICCMAX
			#warning "Tips:Beacon/Pulsing mode Current has been limited due to Turbo ICCMAX is less than 22A."
			
	 #else
			
			#warning "Low Power Beacon/Pulsing mode Enabled.Beacon/Pulsing mode ICC will limit to 22Amps."
			#define BeaconICCMAX 22000
			
	 #endif	

	#endif
/********  �������ű꣨���壩ģʽ����δ���壬��㶨��һ������ϵͳ���� ********/  
#else

	#define BeaconICCMAX 0
  #define StrobeICCMAX 0

#endif

//����һ��0ֵ����ϵͳ��ʵ�ʴ��벿�ֱ���
#ifndef TurboICCMAX
	#define TurboICCMAX 0
#endif

#endif