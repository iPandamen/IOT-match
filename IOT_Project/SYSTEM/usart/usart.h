/********************************************************************
*	@file 		usart4.h
*	@author		Panda
*	@version	V0.0.0
*	@date		2019-09
*********************************************************************/

#ifndef __USART_H
#define __USART_H

#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	 

#include "sys.h" 
#include "Init.h"
#include "delay.h"
#include "timer.h"

#include "userapp.h"

typedef struct
{
	uint8_t *data_buf;
	uint16_t lenth :15;
	uint16_t finish_flag:1;
}RX_BUF;

typedef struct
{
	uint8_t *data_buf;
	uint16_t lenth :15;
	uint16_t finish_flag:1;
}TX_BUF;


/******************************* USART1 *******************************/

#define USART1_REC_LEN   200  	//定义最大接收字节数 200
#define USART1_RX_EN 	 1		//使能（1）/禁止（0）串口1接收
	  	
extern u8  USART1_RX_BUF[USART1_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART1_RX_STA;         		//接收状态标记	

//如果想串口中断接收，请不要注释以下宏定义
void USART1_Init(u32 bound);

void USART1_DataAnalysis(void);

/******************************* USART2 *******************************/

#define USART2_RECV_LEN		600					//最大接收缓存字节数
#define USART2_SEND_LEN		600					//最大发送缓存字节数
#define USART2_RX_EN 		1					//0,不接收;1,接收.

extern u8  USART2_RX_BUF[USART2_RECV_LEN]; 		//接收缓冲,最大USART2_MAX_RECV_LEN字节
extern u8  USART2_TX_BUF[USART2_SEND_LEN]; 		//发送缓冲,最大USART2_MAX_SEND_LEN字节
extern vu16 USART2_RX_STA;   						//接收数据状态
extern uint8_t  IS_UART4_IN_RX;

void USART2_Init(u32 bound);		//串口2初始化 
void usart2_printf(char* fmt,...);
void USART2_SendData(u8 data);		//串口发送一个字节

/******************************* USART3 *******************************/

#define USART3_IT_EN	1
#if USART3_IT_EN
	#define USART3_IT_RX_EN		1
	#define USART3_IT_IDLE_EN		1
#endif	/* USART3_IT_EN */

#define USART3_RX_EN 			1		//使能（1）/禁止（0）串口1接收

#define USART3_RECV_LEN		200  	//定义最大接收字节数 200
#define USART3_SNED_LEN 	100

//extern u8  USART3_RX_BUF[USART3_RECV_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
//extern u16 USART3_RX_STA;         		//接收状态标记	

void USART3_Init(u32 bound);
void USART3_SendStrlen(uint8_t *Data, int len);
   


/******************************* USART4 *******************************/

/*===========================================================================*/
#ifdef __cplusplus  /* C++支持 */
	extern "C"{
#endif
/*===========================================================================*/

#define UART4_PRE_PRIORITY	1
#define UART4_SUB_PRIORITY	0	

#define UART4_REC_MAX_LEN	511		//定义最大接收字节数
#define UART4_RX_EN		1	//串口接收使能。
							//1：允许接收
							//0：禁止接收


/******************************************************************************
                              外部调用功能函数
******************************************************************************/

#if UART4_RX_EN	//使能了串口接收
	
	extern uint8_t  UART4_RX_BUF[UART4_REC_MAX_LEN];	//接收缓冲，最大UART4_REC_MAX_LEN个字节，末字节为换行符 
	extern uint32_t UART4_RX_STA ;		// bit31:接收状态标记,1:UART4接收完成，0：UART4接收未完成
	                                    // bit30:串口接收正在进行中标记,1:进行中，0：暂停或结束
	extern uint8_t UART4_RX_TIMEOUT_COUNT;			//串口接收超时计数。串口超过3ms未接收到数据，视为超时。单位：ms

#endif

void UART4_Init(uint32_t bound);	//初始化IO 串口1
void UART4_SendData(uint16_t Data);
void UART4_SendStr(uint8_t *Data);
void UART4_SendStrlen(uint8_t *Data, uint8_t len);

/*===========================================================================*/
#ifdef __cplusplus  /* C++支持 */
	}
#endif
/*===========================================================================*/


/******************************* USART5 *******************************/

#define UART5_MAX_RECV_LEN		800					//最大接收缓存字节数
#define UART5_MAX_SEND_LEN		800					//最大发送缓存字节数
#define UART5_RX_EN 			1				    //0,不接收;1,接收.

extern u8  UART5_RX_BUF[UART5_MAX_RECV_LEN]; 		//接收缓冲,最大USART2_MAX_RECV_LEN字节
extern u8  UART5_TX_BUF[UART5_MAX_SEND_LEN]; 		//发送缓冲,最大USART2_MAX_SEND_LEN字节
extern vu16 UART5_RX_STA;   						//接收数据状态

void UART5_Init(u32 bound);
void uart5_printf(char* fmt,...);


#endif

/*********************************END OF FILE*********************************/

