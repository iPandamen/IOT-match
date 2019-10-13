/**********************************************************
*	@file 		fan.h
*	@author		Panda
*	@version	V0.0.0
*	@date		2019-09
*	@brief
***********************************************************/
#ifndef _FAN_H
#define _FAN_H

#define RCC_FAN_PORT	RCC_APB2Periph_GPIOA	// fan 使用IO端口的时钟
#define FAN_PORT		GPIOA				 	// fan 使用的端口
#define FAN_PIN			GPIO_Pin_0			 	// fan 使用的IO引脚

#define	FAN_STA		GPIO_ReadInputDataBit(FAN_PORT,FAN_PIN)

#define FAN_ON		GPIO_SetBits(FAN_PORT,FAN_PIN)
#define FAN_OFF		GPIO_ResetBits(FAN_PORT,FAN_PIN)


#include "Init.h"
#include "stm32f10x.h"

void fan_init(void);
void fan_ctrl(bool sta);
bool fan_getsta(void);

#endif


/***************************************END OF FILE**********************************************/

