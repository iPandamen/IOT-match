
#include "nixie_tube.h"


/* [0-7]对应数码管的编号和位选码 */
const uint8_t NixieTube_Nums[NIXIE_TUBE_SUM] = { NIXIE_TUB_NUM_0,NIXIE_TUB_NUM_1,NIXIE_TUB_NUM_2,NIXIE_TUB_NUM_3, 
                                                 NIXIE_TUB_NUM_4,NIXIE_TUB_NUM_5,NIXIE_TUB_NUM_6,NIXIE_TUB_NUM_7};
const uint8_t NixieTube_CharCodes[]  =
{
        0xC0,/*0*/
        0xF9,/*1*/
        0xA4,/*2*/
        0xB0,/*3*/
        0x99,/*4*/
        0x92,/*5*/
        0x82,/*6*/
        0xF8,/*7*/
        0x80,/*8*/
        0x90,/*9*/
		0x7f,/*dot*/
        0x88,/*A*/
        0x83,/*b*/
        0xA7,/*c*/
        0xA1,/*d*/
        0x86,/*E*/
        0x8E,/*F*/
        0x89,/*H 10*/
        0x8B,/*h 11*/
        0xC7,/*L 12*/
        0xAB,/*n 13*/
        0xC8,/*N 14*/
        0xA3,/*o 15*/
        0x8C,/*P 16*/
        0x98,/*q 17*/
        0xAF,/*r 18*/
        0x87,/*t 19 */
        0xC1,/*U 1a*/
        0x91,/*y 1b*/
        0xBF,/*- 1c*/
        0xFF, /*  1D*/
        0x00, /*  1F*/

};

/* [0-7]:对应数码管的编号和数码管显示字符编码 */
uint8_t nixietube_show_buf[NIXIE_TUBE_SUM] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};


/*********************************************************************
* @brief	Nixie Tube Init
* @param	None
* @retval	None
**********************************************************************/
void NixieTube_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_NIXIE_TUBE_PORT, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = NIXIE_TUBE_PINS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(NIXIE_TUBE_PORT, &GPIO_InitStructure);
	
}

/********************************************************************
*@brief		通过 2片 74HC595芯片，控制 8个 数码管
*@param		High: 选中数码管
*				  NIXIE_TUB_NUM_0、NIXIE_TUB_NUM_1、NIXIE_TUB_NUM_2、NIXIE_TUB_NUM_3、
*           	  NIXIE_TUB_NUM_4、NIXIE_TUB_NUM_5、NIXIE_TUB_NUM_6、NIXIE_TUB_NUM_7
*			Low: 字符编码
*@retval	None
*********************************************************************/
void NixieTube_74HC595_Set(uint8_t High,uint8_t Low)
{
	uint8_t i = 0;
	uint16_t dataout;
	dataout = (High<<8) | Low;
	// SCK = 0
	NIXIE_TUBE_SCK_L;
	// RCK = 0
	NIXIE_TUBE_RCK_L;
	for(i=0;i<16;i++)
	{
		// SCK = 0
		NIXIE_TUBE_SCK_L;
		__ASM("NOP");
		if( dataout & 0x8000)
		{
			// DATA = 1
			NIXIE_TUBE_DATA_H;
		}
		else
		{
			// DATA = 0
			NIXIE_TUBE_DATA_L;
		}		
		__ASM("NOP");
		// SCK = 1
		NIXIE_TUBE_SCK_H;
		__ASM("NOP");
		dataout <<= 1;
	}
	// RCK = 0
	NIXIE_TUBE_RCK_L;
	// RCK = 1
	NIXIE_TUBE_RCK_H;
	__ASM("NOP");
	__ASM("NOP");
	// RCK = 0
	NIXIE_TUBE_RCK_L;
}
 

/************************************************************
*@brief		在选定数码管中显示给定的字符
*@param		num: 数码管编号
*			ch_code_num: 字符编码编号
*@retval	None
*************************************************************/
void NixieTube_Show(uint8_t num,uint8_t ch_code_num)
{
	if(num >= NIXIE_TUBE_SUM)
	{
		printf("Printf enter the true number of segled!\r\n");
		return ;
	}
	if(ch_code_num >= NIXIE_TUBE_CHAR_SUM)
	{
		printf("Printf enter the true number of char code num!\r\n");
		return ;
	}
	nixietube_show_buf[num] = NixieTube_CharCodes[ch_code_num];
	NixieTube_74HC595_Set(NixieTube_Nums[num],NixieTube_CharCodes[ch_code_num]);
}

/********************************************************************************
*@brieff	显示整数
*@param		num：要显示的整数
*			prescale: 是否将 8 个数码管分成左右两块
*			block_select:  当 preascale == 1 时，有效； block_select=0: 选中左边的块，block_select=1: 选中右边的块
*@retval	None
*********************************************************************************/
void NixieTube_Show_Integer(uint32_t num,uint8_t prescale,uint8_t block_select)
{
	uint8_t i=0;
	uint8_t show_buf[8]={0};
	for(i=8;i>0;i--)
	{
		show_buf[i-1] = num % 10;
		if(num!=0)
			num /= 10;
		else 
			break;
	}
//	DEBUG("%d,%d,%d,%d,%d,%d,%d,%d",show_buf[0],show_buf[1],show_buf[2],show_buf[3],show_buf[4],show_buf[5],show_buf[6],show_buf[7]);
	if(prescale == 1)
	{
		if(block_select == 0)
		{
			for(i=0;i<4;i++)
			{
				NixieTube_Show(i,show_buf[i+4]);
			}
		}
		else if(block_select == 1)
		{
			for(i=4;i<8;i++)
			{
				NixieTube_Show(i,show_buf[i]);
			}
		}
	}
	else 
	{
		for(i=0;i<8;i++)
		{
			NixieTube_Show(i,show_buf[i]);
		}
	}
}


/************************************************************
*@brief		数码管显示刷新函数
*@param		None
*@retval	None
*************************************************************/
void NixieTube_Refresh(void)
{
	static uint8_t nixietube_num = 0;
	nixietube_num++;
	nixietube_num &= 7;
	NixieTube_74HC595_Set( NixieTube_Nums[nixietube_num], nixietube_show_buf[nixietube_num]);
}

