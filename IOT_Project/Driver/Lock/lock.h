/**********************************************************
*	@file 		door.c
*	@author		Panda
*	@version	V0.0.0
*	@date		2019-09
*	@brief
***********************************************************/
#ifndef __DOOR_H
#define __DOOR_H

#include "stdio.h"
#include "stdlib.h"
#include "stm32f10x.h"

#include "Init.h"
#include "delay.h"

#define RCC_LOCK_OUT_PORT	RCC_APB2Periph_GPIOA	// door 使用IO端口的时钟
#define LOCK_OUT_PORT		GPIOA                   // door 使用的端口
#define LOCK_OUT_PIN 		GPIO_Pin_2              // door 使用的IO引脚

#define LOCK_STA	GPIO_ReadInputDataBit(LOCK_OUT_PORT, LOCK_OUT_PIN)

#define LOCK_ON 	GPIO_SetBits(LOCK_OUT_PORT,LOCK_OUT_PIN)
#define LOCK_OFF	GPIO_ResetBits(LOCK_OUT_PORT,LOCK_OUT_PIN)

void lock_init(void);
bool lock_getsta(void);
void lock_ctrl(bool sta);

void door_getsta(void);
void door_open(uint32_t nms);
	
#endif


/*************************************END OF FILE*****************************************/

