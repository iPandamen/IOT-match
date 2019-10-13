


硬件资源使用

RTC: 实时时钟


fan:
lock：
lamp:




ADC：采样
	ADC1：
		ADC_Channel_10：PC0
		ADC_Channel_11：PC1 
		ADC_Channel_12：PC2
		ADC_Channel_13：PC3

TIM:定时器
	TIM1：用于控制蜂鸣器
		TIM1_CH1：PA8
	
	TIM2：用于系统定时，执行一些自动任务
	
	TIM3：控制舵机云平台
		TIM3_Init
			CH1：PC6
			CH2：PC7
			CH3：PC8
			CH4：PC9
		
		TIM3_pwm_init
			CH1：PC6
			CH2：PC7
			CH3：PB0
			CH4：PB1
	
	TIM7：

USART：
	USART1：串口通信 与PC进行通信
		TX:	PA9
		RX:	PA10
		
	USART2：
		TX：PA2
		RX：PA3
	
	USART3: 用于与语音模块通信
	
	
	UART4: 用于与WIFI进行通信
		TX: PC10
		RX: PC11
		
	UART5:	
		TX:PC12
		RX:PD2

	