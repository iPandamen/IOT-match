/**********************************************************
*	@file 		buzzer.h
*	@author		Panda
*	@version	V0.0.0
*	@date		2019-09
*	@brief
***********************************************************/
#ifndef __BUZZER_H
#define __BUZZER_H

#include "stm32f10x.h"
#include "sys.h"
#include "delay.h"
#include "timer.h"

#define RCC_BUZZER_PORT	RCC_APB2Periph_GPIOA
#define BUZZER_PORT		GPIOA
#define BUZZER_PIN		GPIO_Pin_8


void buzzer_init(void);		
void buzzer_ctrl(bool sta);
bool buzzer_getsta(void);
void beep(u16 tms);

#endif

/**************************** END OF FILE**************************/
