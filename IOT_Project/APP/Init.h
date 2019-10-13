/****************************************************************
*	@file 		Init.h
*	@author		Panda
*	@version	V0.0.0
*	@date		2019-09
*	@brief		设备初始化
*****************************************************************/
#ifndef _INIT_H
#define _INIT_H
/* Includes ------------------------------------------------------------------*/
//	Standard Includes
// 	Libraries Includes
#include "stm32f10x.h"

#include "stm32_config.h"

#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "rtc.h"
#include "debug_printf.h"
#include "kfifo.h"

#include "app_lib.h"
#include "app_pwd.h"

#include "adc.h"


#include "lock.h"
#include "lamp.h"
#include "buzzer.h"
#include "fan.h"


#include "LCD12864.h"
#include "keyboard.h"
#include "timer.h"
#include "lf125k.h"
#include "as608.h"

#include "xfs5152ce.h"

#include "nixie_tube.h"
#include "servo.h"

#include "hal_iic.h"

#include "ldr.h"
#include "pcf8591.h"
#include "temperature.h"
#include "pwm_output.h"

#include "usart_protocol.h"

#include "si4332a.h"
#include "si4332b.h"


/* define --------------------------------------------------------------------*/

#define	LCD_PRINT(row, colum, buf, fmt, args...) do{\
												 memset(buf, 0, sizeof(buf)); \
												 sprintf(buf, fmt,##args);	 \
												 lcd_clr_row(row);			 \
												 lcd_write(row, colum, buf, strlen(buf));}while(0)


#define RCC_KEY1_PORT	RCC_APB2Periph_GPIOC	
#define KEY1_PORT		GPIOC
#define KEY1_PIN		GPIO_Pin_13

#define RCC_KEY2_PORT	RCC_APB2Periph_GPIOD
#define KEY2_PORT		GPIOD
#define KEY2_PIN 		GPIO_Pin_13

#define KEY1		PCin(13)
#define KEY2		PDin(13)

#define KEY1_PRESS		1
#define KEY2_PRESS		2



#define ON 	true
#define OFF	false

#define RET_SUCCESS		0
#define RET_FAILED		1

#define lock_status light_status

/* typedef -------------------------------------------------------------------*/

typedef struct
{
	uint8_t sta_lock	:1;		// 锁的状态
	uint8_t sta_lamp	:1;		// 灯的状态
	uint8_t sta_fan		:1;		// 风扇的状态
	uint8_t sta_buzzer 	:1;		// 蜂鸣器的状态
	uint8_t is_connect_to_server	:1;	// 是否连接上服务器
	
}DEV_STATUS;

typedef enum 
{
	MENU1 = 1,
	MENU2,
	MENU3,
	MENU4,
	MENU5
}MENU_STATUS;


/* variables -----------------------------------------------------------------*/

extern DEV_STATUS device_sta;	// device sta

extern  MENU_STATUS Current_Menu; // 当前的菜单

extern const char light_status[2][5];	

extern uint8_t m3_key_sta;

extern uint8_t data_buf[800];

extern char lcd_show_buf[100];

/* function ------------------------------------------------------------------*/

void M3_key_init(void);

uint8_t M3_key_scan(uint8_t mode);

void __Init_Device(void);

#endif

/***************************************END OF FILE**********************************************/


