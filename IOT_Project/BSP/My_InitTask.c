
/*  -------------------------Includes-----------------------------------------*/
#include "stm32f10x.h"
#include "delay.h"
#include "usart.h"
#include "LCD12864.h"
#include "keyboard_drv.h"
#include "pwm_output.h"
#include "adc.h"
#include "lf125k.h"
#include "eeprom.h"
#include "ultrasonic.h"
#include "rtc.h"
#include "SegLed_16bit.h"
#include "buzzer.h"
#include "hal_timer2.h"
#include "hal_uart4.h"
#include "WiFiToCloud.h"
#include "delay.h"

extern uint32_t rtc_second, is_rtc_update;
extern uint32_t ultrasonic_dist, is_ultrasonic_update;
extern uint8_t adc_nofify_rank;

void rtc_update_notify(uint32_t rtc_counter_val)
{
	is_rtc_update = 1;
	rtc_second = rtc_counter_val;
}

void ultrasonic_update_notify(uint32_t ultrasonic_new_dist)
{
	is_ultrasonic_update = 1;
	ultrasonic_dist = ultrasonic_new_dist;
}

void adc_update_notify(uint8_t nofify_rank)
{
	adc_nofify_rank = nofify_rank;
}


/*
@brief  system reset , (shell port).
@param  None.
@retval None.
*/
void Bsp_SystemReset (void)
{
	NVIC_SystemReset ();
}
void NVIC_Priority_Group_Configuration(void)
{	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}


/*  -------------------------Functions-----------------------------------------*/

/**** stm32的初始化工作全部在这里完�? *****/
void My_InitTask(void)
{	
	NVIC_Priority_Group_Configuration();
	
	//PA09 PA10 UART1 protocol
	USART1_init();
	
	//PC10 PC11 UART4
	UART4_Init(115200);
	
	//SYSTICK
	SYSTICK_init();

	//CH12864C
	lcd_init();

	//add timer2(10ms)
	TIM2_Init();

	//keyboard init
	kbd_init();
	
	//ADC1 INIT
	ADC1_init(adc_update_notify);

	//LF init
	LF125K_init();

	//i2c eeprom
	sEE_Init();

	//rtc init
	rtc_init(rtc_update_notify);
	
	//buzzer
	buzzerInit();
}


