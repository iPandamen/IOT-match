/************************************************************************
*@file		timer.c
*@author	Panda
*@version 	V0.0.0
*@date		2019-09
*@brief		定时器初始化
*************************************************************************/

#include "timer.h"

/*-------------------------- TIM1 ----------------------------------*/

/*********************************************************************
*@brief		TIM1 Init
*@param		arr: 重装载值
*			pre: 预分频系数
*@retval	None
*@attention  CH1 蜂鸣器
**********************************************************************/
void TIM1_Init(uint16_t arr,uint16_t pre)
{
	GPIO_InitTypeDef GPIO_InitStructure;	// GPIO init struct
	NVIC_InitTypeDef NVIC_InitStructure;	// NVIC config struct
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;	//TIM_TimeBase init struct
	TIM_OCInitTypeDef TIM_OCInitStructure;	// TIM OC  init Struct
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); // 使能 GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	// 使能 AFIO 时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);	//使能TIM1的时钟
	
	// TIM1 CH1 GPIO 初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;         //选择引脚8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //输出频率最大50MHz
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
#if TIM1_IT_EN	
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
#endif	/* TIM1_IT_EN */
	
	TIM_DeInit(TIM1);	//重新将Timer设置为缺省值

	TIM_InternalClockConfig(TIM1);//采用内部时钟给TIM1提供时钟源

	TIM_TimeBaseStructure.TIM_Prescaler = pre;	//设置预分频系数
	TIM_TimeBaseStructure.TIM_Period = arr;		//设置重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//设置时钟分割
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		//设置计数器模式为向上计数模式
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);	//将配置应用到TIM1中
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 	//Specifies the TIM mode
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCNPolarity_Low;	//Specifies the complementary output polarity.
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_Low;	//Specifies the output polarity
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;	//Specifies the TIM Output Compare pin state during Idle state
	TIM_OCInitStructure.TIM_OCNIdleState =TIM_OCNIdleState_Reset;	//Specifies the TIM Output Compare pin state during Idle state
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//Specifies the TIM Output Compare state
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;	//Specifies the TIM complementary Output Compare state
	TIM_OCInitStructure.TIM_Pulse = 50; 

	// CH1 Config
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);  	
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable); 
	
	TIM_ClearFlag(TIM1, TIM_FLAG_Update);	//清除溢出中断标志
	
	TIM_ARRPreloadConfig(TIM1, ENABLE);//禁止ARR预装载缓冲器
	
#if TIM1_IT_UPDATE_EN	
	TIM_ITConfig(TIM1,TIM_IT_Update ,ENABLE);//开启TIM1的中断
#endif /* TIM1_IT_UPDATE_EN */
#if TIM1_IT_TRIGGER_EN
	TIM_ITConfig(TIM1,TIM_IT_Trigger,ENABLE);//开启TIM1的中断
#endif	/* TIM1_IT_TRIGGER_EN */
	TIM_CtrlPWMOutputs(TIM1,ENABLE);	//Enables the TIM peripheral Main Outputs. 
	
	TIM_Cmd(TIM1,DISABLE);	//开启定时器1
	
}


#if TIM1_IT_EN
void TIM1_UP_IRQHandler(void)
{
	static int32_t count = 0;
	
   //检测是否发生溢出更新事件
   if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
   {
      count++;
      //清除TIM1的中断待处理位
      TIM_ClearITPendingBit(TIM1 , TIM_FLAG_Update);
	}
 }
#endif	/* TIM1_IT_EN */



/*-------------------------- TIM2 ----------------------------------*/

/***********************************************************
*@brief		TIM2 CH2 Init
*@param		None
*@retval	None
************************************************************/
static void TIM2_CH2_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;         //选择引脚5
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //输出频率最大50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //带上拉电阻输出

	GPIO_Init(GPIOA,&GPIO_InitStructure);
}

/***********************************************************
*@brief		TIM2 NVIC config
*@param		None
*@retval	None
************************************************************/
static void TIM2_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

/***********************************************************
*@brief		TIM2 Time Base config
*@param		arr: 装载值
*			pre: 预分频系数
*@retval	None
************************************************************/
static void TIM2_TimeBase_Config(uint16_t arr,uint16_t pre)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);// 使能TIM2的时钟

	TIM_DeInit(TIM2);	//重新将Timer设置为缺省值

	TIM_TimeBaseStructure.TIM_Prescaler = pre;
	TIM_TimeBaseStructure.TIM_Period = arr;	//设置计数溢出大小，每计1000个数就产生一个更新事件
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	//设置时钟分割
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//设置计数器模式为向上计数模式
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);	//将配置应用到TIM2中

	TIM_ClearFlag(TIM2, TIM_FLAG_Update);	//清除溢出中断标志
	
	TIM_ARRPreloadConfig(TIM2, ENABLE);	//禁止ARR预装载缓冲器

#if TIM2_IT_UPDATE_EN
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);//开启TIM2的中断
#endif /* TIM2_IT_UPDATE_EN */
	
}

/***********************************************************
*@brief		TIM2 OC2 config, OCmode: PWM1
*@param		None
*@retval	None
************************************************************/
static void TIM2_OC2_Config(void)
{
	TIM_OCInitTypeDef	TIM_OCInitStructure;
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	// 
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	
	TIM_OC2Init(TIM2,&TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM2,TIM_OCPreload_Enable);
	
	TIM_SetCompare2(TIM2,500);

}

/***********************************************************
*@brief		TIM2 Init
*@param		None
*@retval	None
************************************************************/
void TIM2_Init(void)
{
	// GPIO config
#if TIM2_OC2_EN	
	TIM2_CH2_GPIO_Config();	
#endif	/* TIM2_OC2_EN*/
	
	//interrupt init
#if TIM2_IT_EN
	TIM2_NVIC_Configuration();
#endif	/* TIM2_IT_EN */
	
	//tim2 timer init
	TIM2_TimeBase_Config(1000-1,72-1);

#if TIM2_OC2_EN	
	//TI2 OC2 config
	TIM2_OC2_Config();
#endif 	/* TIM2_OC2_EN */
	//开启定时器2
	TIM_Cmd(TIM2,ENABLE);
}

#if TIM2_IT_EN

void TIM2_IRQHandler(void)
{
	static int32_t count = 0;
	
   //检测是否发生溢出更新事件
   if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)	// 没1ms 进入一次中断
   {
		count++;
		//清除TIM2的中断待处理位
		TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);
		//do keboard scaning
		if(count % 10 == 0)
		{	
			kbd_scan();
			NixieTube_Refresh();
		}
		if(count % 1000 == 0)
		{
			lock_getsta();
			lamp_getsta();
			buzzer_getsta();
		}
	}
 }
#endif /* TIM2_IT_EN */
 
/* TIM3--------------------------------------------------------------------*/


/*****************************************************************************************
*@brief		TIM3 Init
*@param		arr: 装载值
*			pre: 预分频系数
*@retval	None
******************************************************************************************/
void TIM3_Init(u16 arr,u16 pre)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//使能定时器3时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
	
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE); //Timer3部分重映射  
	
	//设置该引脚为复用输出功能,输出TIM3 CH2的PWM脉冲波形	GPIOC7
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIO
 
	//初始化TIM3
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler = pre; //设置用来作为TIMx时钟频率除数的预分频值 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	//初始化TIM3 Channel-1 PWM模式	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	
	
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC2
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM3在CCR2上的预装载寄存器
	
	//初始化TIM3 Channel-2 PWM模式	 
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC2
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM3在CCR2上的预装载寄存器
	
	//初始化TIM3 Channel-3 PWM模式	 
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC2
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM3在CCR2上的预装载寄存器
	
	//初始化TIM3 Channel-4 PWM模式	 
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC2
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM3在CCR2上的预装载寄存器
	
	TIM_Cmd(TIM3, ENABLE);  //使能TIM3
}

 
 

/********************************* TIM7 ******************************************/

 //定时器7中断服务程序		    
void TIM7_IRQHandler(void)
{ 	
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)//是更新中断
	{	 			   
//		USART2_RX_STA|=1<<15;
		UART5_RX_STA|=1<<15;	//标记接收完成
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update  );  //清除TIM7更新中断标志    
		TIM_Cmd(TIM7, DISABLE);  //关闭TIM7 
	}	    
}
 
//通用定时器7中断初始化
//这里时钟选择为APB1的2倍，而APB1为42M
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz 
//通用定时器中断初始化
//这里始终选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数		 
void TIM7_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);//TIM7时钟使能    
	
	//定时器TIM7初始化
	TIM_TimeBaseStructure.TIM_Period = arr; 	//设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE ); //使能指定的TIM7中断,允许更新中断
	
	TIM_Cmd(TIM7,ENABLE);//开启定时器7
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
}



