#include "usart_protocol.h"
#include "usart.h"
#include "Temperature.h"
#include "delay.h"
#include "LCD12864.h"
#include "lf125k.h"
#include "as608.h"


 
/**
* @brief  获取温度响应
* @param  
* @retval 
*/
void respond_get_Illumination()
{
	unsigned char buf_temp[15];
	long data_int = 0;
	short tx_count = 0;

	data_int = LDR_Lux(0)*100;
//	printf("串口响应光照数据 = %.2f\r\n",GetIllumination());
	buf_temp[0] = 0xBB;
	buf_temp[1] = 0x01;
	buf_temp[2] = 0x02;
	buf_temp[3] = 0x05;
	buf_temp[4] = (unsigned char)((data_int/100)>>24);
	buf_temp[5] = (unsigned char)((data_int/100)>>16);
	buf_temp[6] = (unsigned char)((data_int/100)>>8);
	buf_temp[7] = (unsigned char)(data_int/100);
	buf_temp[8] = (unsigned char)(data_int%100);
	buf_temp[9] = (short)(buf_temp[1]+buf_temp[2]+buf_temp[3]+buf_temp[4]+buf_temp[5]+buf_temp[6]+buf_temp[7]+buf_temp[8]);
	buf_temp[10] = 0x7E;
	while(tx_count < 11)
	{
		/* Wait until end of transmit */
		while(!(USART1->SR&(0x0001<<7)));
		/* Write one byte in the USART1 Transmit Data Register */
		USART1->DR = (uint16_t)(0x01FF & buf_temp[tx_count]);
		tx_count++;
	}
#if RESPONSE_FRAME_HEX_OUTPUT_EN
	printf_hex(buf_temp,tx_count);
#endif	/* RESPONSE_FRAME_HEX_OUTPUT_EN */
	beep(100);
}


/**
* @brief  舵机响应
* @param  site  舵机号
		   buf	角度
* @retval 
*/
void respond_steeringengine(uint8_t site,uint16_t buf)
{
	unsigned char buf_temp[15];
	short data_checksum = 0, tx_count = 0;
	
	switch(site)
	{
		case 0x00:		//下舵机
			Servo_Control_X(buf);
		break;
		
		case 0x01:
//			if(site > 130)		//上舵机最多130°
//				site = 130;
			Servo_Control_Y(buf);
		break;
		
		default:
			break;
		beep(100);
	}
	
	buf_temp[0] = 0xBB;
	buf_temp[1] = 0x01;
	buf_temp[2] = 0x08;
	buf_temp[3] = 0x02;
	buf_temp[4] = site;
	buf_temp[5] = buf;
	buf_temp[6] = (short)(buf_temp[1]+buf_temp[2]+buf_temp[3]+buf_temp[4]+buf_temp[5]);
	buf_temp[7] = 0x7E;
	while(tx_count < 8)
	{
		/* Wait until end of transmit */
		while(!(USART1->SR&(0x0001<<7)));
		/* Write one byte in the USART1 Transmit Data Register */
		USART1->DR = (uint16_t)(0x01FF & buf_temp[tx_count]);
		tx_count++;
	}
#if RESPONSE_FRAME_HEX_OUTPUT_EN
	printf_hex(buf_temp,tx_count);
#endif	/* RESPONSE_FRAME_HEX_OUTPUT_EN */
	beep(100);
	
}

/**
* @brief  低频卡初始化响应
* @param  
* @retval 
*/
void respond_lf125k_init()
{
	unsigned char buf_temp[15],lf_buf[8];
	short tx_count = 0;

//	LCD_PRINT(1,3,"0x09   ");
	
	if(LF125K_Card_Init() == 0)	
		buf_temp[4] = 0x00;
	else
		buf_temp[4] = 0xff;
	buf_temp[0] = 0xBB;
	buf_temp[1] = 0x01;
	buf_temp[2] = 0x09;
	buf_temp[3] = 0x01;
	buf_temp[5] = 0x0B + buf_temp[4];
	buf_temp[6] = 0x7E;
	while(tx_count < 7)
	{
		/* Wait until end of transmit */
		while(!(USART1->SR&(0x0001<<7)));
		/* Write one byte in the USART1 Transmit Data Register */
		USART1->DR = (uint16_t)(0x01FF & buf_temp[tx_count]);
		tx_count++;
	}
#if RESPONSE_FRAME_HEX_OUTPUT_EN
	printf_hex(buf_temp,tx_count);
#endif	/* RESPONSE_FRAME_HEX_OUTPUT_EN */
	beep(100);
}


/**
* @brief  读低频卡号响应
* @param  
* @retval 
*/
void respond_read_lf125k_ID()
{
	unsigned char i,buf_temp[15],lf_buf[8];
	short tx_count = 0;

	for(i=0;i<3;i++)
	{
		if(LF125K_Read_1_Page(lf_buf) == 0)
		{
//			LCD_PRINT(1,3,"0x0A   ");	
			buf_temp[0] = 0xBB;
			buf_temp[1] = 0x01;
			buf_temp[2] = 0x0A;
			buf_temp[3] = 0x08;
			buf_temp[4] = lf_buf[0];
			buf_temp[5] = lf_buf[1];
			buf_temp[6] = lf_buf[2];
			buf_temp[7] = lf_buf[3];
			buf_temp[8] = lf_buf[4];
			buf_temp[9] = lf_buf[5];
			buf_temp[10] = lf_buf[6];
			buf_temp[11] = lf_buf[7];
			buf_temp[12] = (short)(buf_temp[1]+buf_temp[2]+buf_temp[3]+buf_temp[4]+buf_temp[5]+buf_temp[6]+buf_temp[7]+buf_temp[8]+buf_temp[9]+buf_temp[10]+buf_temp[11]);
			buf_temp[13] = 0x7E;
			while(tx_count < 14)
			{
				/* Wait until end of transmit */
				while(!(USART1->SR&(0x0001<<7)));
				/* Write one byte in the USART1 Transmit Data Register */
				USART1->DR = (uint16_t)(0x01FF & buf_temp[tx_count]);
				tx_count++;
			}
#if RESPONSE_FRAME_HEX_OUTPUT_EN
			printf_hex(buf_temp,tx_count);
#endif	/* RESPONSE_FRAME_HEX_OUTPUT_EN */
			beep(100);

			break;							
		}
	}
}



/**
* @brief  读低频卡数据响应
* @param  uint8_t block		数据块

* @retval 
*/
void respond_read_lf125k_data(uint8_t block)
{
	unsigned char i,buf_temp[15],lf_buf[4];
	short tx_count = 0;
	uint8_t a = block;	
	
	if(LF125K_Read_0_Page(a,lf_buf) == 0)
	{
//		LCD_PRINT(1,3,"%d     ",block);
		buf_temp[0] = 0xBB;
		buf_temp[1] = 0x00;
		buf_temp[2] = 0x0B;
		buf_temp[3] = 0x06;
		buf_temp[4] = 0x00;
		buf_temp[5] = block;					//Block页不用改
		buf_temp[6] = lf_buf[0];
		buf_temp[7] = lf_buf[1];
		buf_temp[8] = lf_buf[2];
		buf_temp[9] = lf_buf[3];
		buf_temp[10] = (short)(buf_temp[1]+buf_temp[2]+buf_temp[3]+buf_temp[4]+buf_temp[5]+buf_temp[6]+buf_temp[7]+buf_temp[8]+buf_temp[9]);
		buf_temp[11] = 0x7E;
		while(tx_count < 12)
		{
			/* Wait until end of transmit */
			while(!(USART1->SR&(0x0001<<7)));
			/* Write one byte in the USART1 Transmit Data Register */
			USART1->DR = (uint16_t)(0x01FF & buf_temp[tx_count]);
			tx_count++;
		}
#if RESPONSE_FRAME_HEX_OUTPUT_EN
		printf_hex(buf_temp,tx_count);
#endif	/* RESPONSE_FRAME_HEX_OUTPUT_EN */		
		beep(100);
	}
}

/**
* @brief  读低频卡化响应
* @param  *buf		命令协议
* @retval 
*/
void respond_write_lf125k_data(unsigned char *buf) 
{
	unsigned char i,a,buf_temp[15],lf_buf[4];
	short tx_count = 0;

//	LCD_PRINT(2,0,lcd_show_buf,"0x0C   ");	
	lf_buf[0] = buf[6];
	lf_buf[1] = buf[7];
	lf_buf[2] = buf[8];
	lf_buf[3] = buf[9];		
	if(LF125K_Write_0_Page(buf[5],lf_buf) == 0)
	{
		buf_temp[4] = 0x00;
	}
	else
		buf_temp[4] = 0x01;
	delay_ms(10);
	buf_temp[0] = 0xBB;
	buf_temp[1] = 0x01;
	buf_temp[2] = 0x0C;
	buf_temp[3] = 0x01;
	buf_temp[5] = 0x0E + buf_temp[4];					
	buf_temp[6] = 0x7E;
	while(tx_count < 7)
	{
		/* Wait until end of transmit */
		while(!(USART1->SR&(0x0001<<7)));
		/* Write one byte in the USART1 Transmit Data Register */
		USART1->DR = (uint16_t)(0x01FF & buf_temp[tx_count]);
		tx_count++;
	}
#if RESPONSE_FRAME_HEX_OUTPUT_EN
	printf_hex(buf_temp,tx_count);
#endif	/* RESPONSE_FRAME_HEX_OUTPUT_EN */
	beep(100);
}


