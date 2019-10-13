
#ifndef TIMER_H
#define TIMER_H

#include <stdio.h>

#include "stm32f10x.h"
#include "delay.h"
#include "keyboard.h"
#include "usart.h"

/*-------------------------- TIM1 ----------------------------------*/

#define TIM1_IT_EN 0	
#if TIM1_IT_EN
	#define TIM1_IT_UPDATE_EN	1
	#define TIM1_IT_TRIGGER_EN	1
#endif


void TIM1_Init(uint16_t arr,uint16_t pre);

/*-------------------------- TIM2 ----------------------------------*/

#define TIM2_OCx_EN 0
#if	TIM2_OCx_EN
//	#define TIM2_OC1_EN 1
	#define TIM2_OC2_EN 1
//	#define TIM2_OC3_EN 1
//	#define TIM2_OC4_EN 1
#endif /* TIM2_OCx_EN */

#define TIM2_IT_EN 1
#if TIM2_IT_EN
	#define TIM2_IT_UPDATE_EN 1
#endif /* TIM2_IT_EN */

void TIM2_Init(void);

/* TIM3----------------------------------------------------------------*/

void TIM3_Init(u16 arr,u16 pre);

/*-------------------------- TIM7 ----------------------------------*/

void TIM7_Int_Init(u16 arr,u16 psc);

#endif

