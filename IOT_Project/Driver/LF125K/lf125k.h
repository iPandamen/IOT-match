#ifndef __LF125K_H
#define __LF125K_H
/* Includes --------------------*/
#include "stm32f10x.h"
#include "usart.h"
/* Typdefs -----------------------*/

typedef enum 
{
	PAGE_0_BLOCK_0 = 0,
	PAGE_0_BLOCK_1,
	PAGE_0_BLOCK_2,
	PAGE_0_BLOCK_3,
	PAGE_0_BLOCK_4,
	PAGE_0_BLOCK_5,
	PAGE_0_BLOCK_6,
	PAGE_0_BLOCK_7,
	PAGE_1,
	PAGE_ALL
}Card_Infomation;


/* Constants -------------------*/
/* Define ----------------------*/

#define ID_CARD1	1
#define ID_CARD2	2
#define ID_CARD3	3

/* Variables -------------------*/

extern const uint8_t id_card1[8];//id card1 ¿¨ºÅ
extern const uint8_t id_card3[8];//id card3 ¿¨ºÅ

extern uint8_t lf_page_0_buf[8][4];	// page 0 info
extern uint8_t lf_page_1_buf[8];	// page 1 info

/* Functions prototypes --------*/

void LF125K_Init(void);

//adr(0~7), buf(return 4 bytes)
int8_t LF125K_Read_0_Page(uint8_t adr,uint8_t *buf);

//adr(0~7), buf(4 bytes)
int8_t LF125K_Write_0_Page(uint8_t adr,uint8_t *buf);

//return 8 bytes
int8_t LF125K_Read_1_Page(uint8_t *buf);

int8_t LF125K_Card_Init(void);
int8_t LF125K_Set_PWD (uint8_t *pwd);
int8_t LF125K_Clear_PWD (uint8_t *pwd);
int8_t LF125K_Write_0_Page_Protect(uint8_t adr,uint8_t *buf,uint8_t *pwd);
int8_t LF125K_Read_0_Page_Protect(uint8_t adr,uint8_t *buf,uint8_t *pwd);

int8_t LF125K_Get_CardID (uint8_t *buf);

int8_t LF125K_SwipeCard(void);

int8_t LF125K_Get_CardInfo(uint8_t addr);

void LF125K_Show_CardInfo(uint8_t addr);
	
#endif
