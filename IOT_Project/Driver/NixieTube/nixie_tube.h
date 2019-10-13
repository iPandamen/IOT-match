/****************************************************************
*	@file 		nixie_tube.h
*	@author		Panda
*	@version	V0.0.0
*	@date		2019-09
*	@brief		通过 两片74HC595芯片 控制 8 个数码管  
*****************************************************************/


#ifndef _NIXIE_TUBE_H
#define _NIXIE_TUBE_H


#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"

#include "sys.h"
#include "timer.h"

#define RCC_NIXIE_TUBE_PORT 	RCC_APB2Periph_GPIOC

#define NIXIE_TUBE_PORT			GPIOC

#define NIXIE_TUBE_DATA_PIN		GPIO_Pin_8
#define NIXIE_TUBE_SCK_PIN		GPIO_Pin_9
#define NIXIE_TUBE_RCK_PIN		GPIO_Pin_12

#define NIXIE_TUBE_PINS		(NIXIE_TUBE_DATA_PIN | NIXIE_TUBE_SCK_PIN | NIXIE_TUBE_RCK_PIN)

#define NIXIE_TUBE_DATA_H	GPIO_SetBits(NIXIE_TUBE_PORT,NIXIE_TUBE_DATA_PIN);
#define NIXIE_TUBE_DATA_L	GPIO_ResetBits(NIXIE_TUBE_PORT,NIXIE_TUBE_DATA_PIN);

#define NIXIE_TUBE_SCK_H	GPIO_SetBits(NIXIE_TUBE_PORT,NIXIE_TUBE_SCK_PIN);
#define NIXIE_TUBE_SCK_L    GPIO_ResetBits(NIXIE_TUBE_PORT,NIXIE_TUBE_SCK_PIN);

#define NIXIE_TUBE_RCK_H	GPIO_SetBits(NIXIE_TUBE_PORT,NIXIE_TUBE_RCK_PIN);
#define NIXIE_TUBE_RCK_L	GPIO_ResetBits(NIXIE_TUBE_PORT,NIXIE_TUBE_RCK_PIN);


#define NIXIE_TUBE_SUM		8		


/******************* 数码管的编号 ******************/
typedef enum
{
	NIXIE_TUB_NUM_0 = 0x01,
	NIXIE_TUB_NUM_1 = 0x02,
	NIXIE_TUB_NUM_2 = 0x04,
	NIXIE_TUB_NUM_3 = 0x08,
	NIXIE_TUB_NUM_4 = 0x10,
	NIXIE_TUB_NUM_5 = 0x20,
	NIXIE_TUB_NUM_6 = 0x40,
	NIXIE_TUB_NUM_7 = 0x80,
	
}NIXIE_TUBE_NUM;


/******************* 输出字符的编码 ********************/
typedef enum
{
	NIXIE_TUBE_CHAR_0 = 0x00,
	NIXIE_TUBE_CHAR_1 = 0x01,
	NIXIE_TUBE_CHAR_2 = 0x02,
	NIXIE_TUBE_CHAR_3 = 0x03,
	NIXIE_TUBE_CHAR_4 = 0x04,
	NIXIE_TUBE_CHAR_5 = 0x05,
	NIXIE_TUBE_CHAR_6 = 0x06,
	NIXIE_TUBE_CHAR_7 = 0x07,
	NIXIE_TUBE_CHAR_8 = 0x08,
	NIXIE_TUBE_CHAR_9 = 0x09,
	NIXIE_TUBE_CHAR_a = 0x0A,
	NIXIE_TUBE_CHAR_b = 0x0B,
	NIXIE_TUBE_CHAR_c = 0x0C,
	NIXIE_TUBE_CHAR_d = 0x0D,
	NIXIE_TUBE_CHAR_E = 0x0E,
	NIXIE_TUBE_CHAR_F = 0x0F,
	NIXIE_TUBE_CHAR_H = 0x10,
	NIXIE_TUBE_CHAR_h = 0x11,
	NIXIE_TUBE_CHAR_L = 0x12,
	NIXIE_TUBE_CHAR_n = 0x13,
	NIXIE_TUBE_CHAR_N = 0x14,
	NIXIE_TUBE_CHAR_o = 0x15,
	NIXIE_TUBE_CHAR_p = 0x16,
	NIXIE_TUBE_CHAR_q = 0x17,
	NIXIE_TUBE_CHAR_r = 0x18,
	NIXIE_TUBE_CHAR_t = 0x19,
	NIXIE_TUBE_CHAR_U = 0x1A,
	NIXIE_TUBE_CHAR_y = 0x1B,
	NIXIE_TUBE_CHAR__ = 0x1C,
	NIXIE_TUBE_CHAR_SPACE = 0x1D,
	NIXIE_TUBE_CHAR_SUM
}NIXIE_TUBE_CHAR_CODE;

extern uint8_t nixietube_show_buf[NIXIE_TUBE_SUM];
extern const uint8_t NixieTube_CharCodes[];

void NixieTube_Init(void);	//Nixie Tube Init

void NixieTube_74HC595_Set(uint8_t High,uint8_t Low);

void NixieTube_Show(uint8_t num,uint8_t ch_code_num);	//在选定数码管中显示给定他字符

void NixieTube_Show_Integer(uint32_t num,uint8_t prescale,uint8_t block_select);

void NixieTube_Refresh(void); //数码管显示刷新函数


#endif	//_NIXIE_TUBE_H
