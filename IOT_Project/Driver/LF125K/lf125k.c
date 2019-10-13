/*****************************************************************
*	@file	 	lf125k.c
*	@Author	 	Allen
*	@Version 
*	@date		2019-09
*	@brief
*******************************************************************/
/* Includes --------------------*/
#include <string.h>
#include "stm32f10x.h"
#include "delay.h"
#include "lf125k.h"

/* Types ---------------------------------------------*/

/* Constants -----------------------------------------*/

/* Define --------------------------------------------*/

#define REM 		GPIO_Read_LF125K_Data()
#define CLK(n) 		GPIO_SET_LF125K_CLK(n)
#define	M_OK 		0
#define LF_DBR_BASE 
#define LF_1us 		72ul
#define LF_720us 	(720000-LF_1us*720)
#define LF_650us 	(720000-LF_1us*650)
#define LF_Star 	(720000-LF_1us*5000)//5ms
#define LF_T557 	(720000-LF_1us*350)//350us
#define LF_380us 	(720000-LF_1us*380)
#define LF_350us 	(720000-LF_1us*384)//8*48
//#define LF_350us (720000-LF_1us*350)
#define LF_250us 	(720000-LF_1us*250)
#define LF_200us 	(720000-LF_1us*200)
#define LF_100us 	(720000-LF_1us*128)//8*18
//#define LF_100us (720000-LF_1us*100)
#define LF_5ms 		(720000-LF_1us*5000)//5ms

//低频
#define GPIO_LF125K_DATA 		GPIOE
#define GPIO_LF125K_DATA_PIN 	GPIO_Pin_5
#define GPIO_LF125K_CLK 		GPIOE
#define GPIO_LF125K_CLK_PIN 	GPIO_Pin_4

/* Variables -------------------*/
//id card1 卡号
const uint8_t id_card1[8]={0x07,0xA8,0x50,0x26,0x3F,0x41,0xD5,0x84};
//id card3 卡号
const uint8_t id_card3[8]={0x07,0xA8,0x50,0x26,0x3F,0x41,0x69,0x53};


uint8_t lf_page_0_buf[8][4]={0};	// page 0 info

uint8_t lf_page_1_buf[8]={0};	// page 1 info
	
/* Functions prototypes --------*/
/* Functions -------------------*/
/**********************************************************************************************
*	@brief		配置 LF125K GPIO引脚
*	@param		None
*	@retval		None	
***********************************************************************************************/
void GPIO_LF125K_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_LF125K_CLK_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_LF125K_CLK, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_LF125K_DATA_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIO_LF125K_DATA, &GPIO_InitStructure);
}
/****************************************************
*@brief	
*@param	
*@retval	
*****************************************************/	
void GPIO_SET_LF125K_CLK (uint8_t status)
{
	if (status)
		GPIO_WriteBit(GPIO_LF125K_CLK, GPIO_LF125K_CLK_PIN,Bit_SET);
	else
		GPIO_WriteBit(GPIO_LF125K_CLK, GPIO_LF125K_CLK_PIN,Bit_RESET);
}

/****************************************************
*@function		uint8_t GPIO_Read_LF125K_Data (void)
*@brief	
*@param	
*@retval	
*****************************************************/
uint8_t GPIO_Read_LF125K_Data (void)
{
	return GPIO_ReadInputDataBit(GPIO_LF125K_DATA, GPIO_LF125K_DATA_PIN);
}


uint8_t volue;	//volue 为曼侧斯特码译码时的临时运算缓存
uint8_t tap;	//tap 为曼侧斯特码发射时的临时运算缓存
uint8_t data_tap[6];	//等待发射的数据
void _nop_(void)
{
	uint32_t i;
	i=1;
	while(i--);
}


/****************************************************
*@brief			打开天线
*@param	
*@retval	
*****************************************************/
void LF125K_Open_RF(void)
{
	CLK(1);
}
/****************************************************
*@brief			关闭天线
*@param	
*@retval	
*****************************************************/
void LF125K_Close_RF(void)
{
	CLK(0);
}

/****************************************************
*@brief	
*@param	
*@retval	
*****************************************************/
void LF125K_Init(void)
{
	GPIO_LF125K_Configuration();
	LF125K_Open_RF();
}

/****************************************************
*	@brief		Manchester process.
*	@param		
*	@retval		0:success
*				1:failse.
*****************************************************/
int8_t REM_Processing(uint8_t *buf)
{
	uint32_t i,j,delay;
	long tus;
	for(i=0;i<50;i++)//find the first cync head
	{	
		volue=0;
		//while(REM==0);
		delay = 7200;
		while(REM == 0 && delay)  //wait high level
			delay --;
		if (delay == 0) 
		{
			return 1;//timeout
		}
		SYSTICK_ClearSystickVal();
		while(REM == 1)
		{
			if(SYSTICK_GetVal() > LF_720us)
			{
				
			}
			else
			{
				i = 50;
				break;
			}
		}
   	}
	delay = 7200;
	while(REM == 1 && delay)//wait low level
		delay --;
	if (delay == 0) 
	{
		return 1;
	}
	delay = 72000;
	while(REM == 0 && delay)//wait high level
		delay --;
	if (delay == 0) 
	{
		return 1;
	}
	SYSTICK_ClearSystickVal();
	tus = LF_650us;
	for(i=0;i<8;i++)
	{
		for(j=0;j<8;j++)
		{
			while(SYSTICK_GetVal() > tus);
			if(REM==1)
			{
				delay = 7200;
				while(REM==1 && delay)
				{
					delay --;
				}
				if(delay == 0)
					return 1;
				volue>>=1;
				volue|=0x80;
				tus = LF_T557;
				SYSTICK_ClearSystickVal();
			}
			else
			{
				delay = 7200;
				while(REM==0 && delay)
				{
					delay --;
				}
				if(delay == 0)
					return 1;
				volue>>=1;
				volue|=0x00;
				tus = LF_T557;
				SYSTICK_ClearSystickVal();
			}
		}
		buf[i]=volue;
		volue=0x00;
	}	   
	return 0;
}

/****************************************************
*	@brief		
*	@param		
*	@retval		
*				
*****************************************************/
void Data_Processing(void)
{
	if(tap==0x01)
	{
		SYSTICK_ClearSystickVal();
		while(SYSTICK_GetVal()>LF_350us);
		CLK(0);
	 }
	else
	{
		SYSTICK_ClearSystickVal();
		while(SYSTICK_GetVal()>LF_100us);
		CLK(0);
	 }
	SYSTICK_ClearSystickVal();
	while(SYSTICK_GetVal()>LF_250us);
	CLK(1);
}
/****************************************************
*	@brief		
*	@param		
*	@retval		
*				
*****************************************************/
void write_od(uint8_t voled)
{
	SYSTICK_ClearSystickVal();
	while(SYSTICK_GetVal()>LF_Star);
	CLK(0);
	SYSTICK_ClearSystickVal();
	while(SYSTICK_GetVal()>LF_350us);
	CLK(1);
	tap=voled;
	tap&=0x10;
	if(tap==0x10)
	{
		tap=0x01;
		Data_Processing();//
	}
	else
	{
		tap=0x00;
		Data_Processing();//
	}
	tap=voled;
	tap&=0x01;
	Data_Processing();//	
}
/****************************************************
*	@brief		
*	@param		
*	@retval		
*				
*****************************************************/
void  write_lock(uint8_t voled)//
{
	tap=voled;
	tap&=0x01;
	Data_Processing();
}
/****************************************************
*	@brief		
*	@param		
*	@retval		
*				
*****************************************************/
void write_data()
{
	uint8_t i,j,voled_1;
	for(j=0;j<4;j++)
	{
		voled_1=data_tap[j];
		for(i=0;i<8;i++)
		{ 
			tap=voled_1;
			tap&=0x01;
			Data_Processing();
			voled_1>>=1;
		}
	}
}
/****************************************************
*	@brief		
*	@param		
*	@retval		
*				
*****************************************************/
void write_add(uint8_t voled)
{
	tap=voled;
	tap&=0x04;
	if(tap==0x04)
	{
		tap=0x01;
		Data_Processing();//
	}
	else 
	{
		tap=0x00;
		Data_Processing();//
	}
	tap=voled;
	tap&=0x02;
	if(tap==0x02)
	{
		tap=0x01;
		Data_Processing();//
	}
	else 
	{
		tap=0x00;
		Data_Processing();//
	}
	tap=voled;
	tap&=0x01;
	Data_Processing();//	
}
/****************************************************
*	@brief		读 LF Card 的 page 1
*	@param		buf: 读出的数据
*	@retval		0: 成功
*				1: 失败
*				
*****************************************************/
int8_t LF125K_Read_1_Page(uint8_t* buf)
{
	uint8_t buf1[10];
	write_od(0x11);
//	SYSTICK_Delay10ms(1);
	delay_ms(10);
	if(REM_Processing(buf))
		return 1;
//	SYSTICK_Delay10ms(5);
	delay_ms(50);
	write_od(0x11);
//	SYSTICK_Delay10ms(1);
	delay_ms(10);
	if(REM_Processing(buf1))
		return 1;
	if (memcmp(buf1,buf,8) != 0)
	{
		return 1;
	}
	return 0;
}
/****************************************************
*	@brief		读 LF Card 的 Page 0
*	@param		adr: 地址
*				buf: 读出的数据
*	@retval		0: 成功
*				1: 失败
*				
*****************************************************/
int8_t LF125K_Read_0_Page(uint8_t adr,uint8_t* buf)
{
	uint8_t  buf1[10];
	adr &= 0x7;
	write_od(0x10);
	//write_lock(RI_Buffer[3]);
	write_lock(0x00);
	write_add(adr);
	SYSTICK_Delay10ms(2);
	REM_Processing(buf1);
	SYSTICK_Delay10ms(2);
	write_od(0x10);
	//write_lock(RI_Buffer[3]);
	write_lock(0x00);
	write_add(adr);
	SYSTICK_Delay10ms(2);
	REM_Processing(buf);
	if(memcmp(buf1,buf,4) != 0)	
   		return M_OK+1;
	return M_OK;
}
/****************************************************
*	@brief		向 LF Card 的 page 0  写数据
*	@param		adr: 写人的位置
*				buf: 写入的数据
*	@retval		
*				
*****************************************************/
int8_t LF125K_Write_0_Page(unsigned char adr,unsigned char *buf) 
{
	uint8_t  buf1[10];
	uint8_t lock=0;
	if((adr&0x80) == 0x80)
		lock = 1;
	adr &=0x07;
	if(adr==0x00)//块0 配置区 防止误操作
		return M_OK+1;
	
	write_od(0x10);
	//write_lock(RI_Buffer[7]);
	write_lock(lock);//写1固化
	data_tap[0]=buf[0];
	data_tap[1]=buf[1];
	data_tap[2]=buf[2];
	data_tap[3]=buf[3];
	write_data();
	write_add(adr);
	/*SYSTICK_Delay10ms(2);	
	REM_Processing(buf1);*/
	SYSTICK_Delay10ms(2);
	
	write_od(0x10);
	write_lock(0x00);
	write_add(adr);
	SYSTICK_Delay10ms(2);
	REM_Processing(buf1);
	if(memcmp(buf1,buf,4)!=0)	
   		return M_OK+1;
	return M_OK;
}

/****************************************************
*	@brief		固化页0某块数据，固化后不能修改.
*	@param		
*	@retval		
*				
*****************************************************/
int8_t LF125K_Write_0_Page_die(char adr,char* buf) 
{
	return 1;
}

/****************************************************
*	@brief		带密码,写 page 0 的块数据.
*	@param		adr: 写入的位置
*				buf: 写入的数据
*				pwd: 密码
*	@retval		0: 成功
*				1: 失败
*****************************************************/
int8_t LF125K_Write_0_Page_Protect(uint8_t adr,uint8_t *buf,uint8_t *pwd)
{
	uint8_t lock=0;
	if ((adr&0x80) == 0x80) 
	{
		lock = 1;
	}
	write_od(0x10);
	data_tap[0]=pwd[0];
	data_tap[1]=pwd[1];
	data_tap[2]=pwd[2];
	data_tap[3]=pwd[3];
	write_data();
	write_lock(lock);
	
	data_tap[0]=buf[0];
	data_tap[1]=buf[1];
	data_tap[2]=buf[2];
	data_tap[3]=buf[3];
	write_data();
	write_add(adr);
	SYSTICK_Delay10ms(1);
	if(REM_Processing(buf))
		return 1;	
	return 0;
}

/**********************************************************************************************
*	@brief		带密码读Page 0 的块数据.
*	@param		adr: 读取数据的地址
*				buf: 读出的数据
*				pwd: 密码
*	@retval		0: 成功
*				1: 失败
***********************************************************************************************/
int8_t LF125K_Read_0_Page_Protect(uint8_t adr,uint8_t *buf,uint8_t *pwd)
{
/*	write_od(0x10);
	data_tap[0]=pwd[0];
	data_tap[1]=pwd[1];
	data_tap[2]=pwd[2];
	data_tap[3]=pwd[3];
	write_data();
	SYSTICK_Delay10ms(1);
*/
	write_od(0x10);
	data_tap[0]=pwd[0];
	data_tap[1]=pwd[1];
	data_tap[2]=pwd[2];
	data_tap[3]=pwd[3];
	write_data();
	write_lock(0x00);
	write_add(adr);
	SYSTICK_Delay10ms(1);
	if(REM_Processing(buf))
		return 1;	
	return 0;
}

//无密码
/**********************************************************************************************
*	@brief		初始化 LF Card	
*	@param		
*	@retval		
*				
***********************************************************************************************/
int8_t Card_Initialization(void)
{
	uint8_t  buf[10];
	write_od(0x10);
	write_lock(0x00);
	data_tap[0]=0x00;
	data_tap[1]=0x28;//0x28;
	data_tap[2]=0x01;
	data_tap[3]=0x17;
	write_data();
	write_add(0x00);
	SYSTICK_Delay10ms(100);
	REM_Processing(buf);
	return (LF125K_Read_1_Page(buf));
}
/**********************************************************************************************
*	@brief		
*	@param		
*	@retval						
***********************************************************************************************/
void LF125K_card_init1(void)
{
	write_od(0x10);
	write_lock(0x00);
	data_tap[0]=0x00;
	data_tap[1]=0x28;//数据率
	data_tap[2]=0x01;
	data_tap[3]=0x17;
	write_data();
	write_add(0x00);
}

/**********************************************************************************************
*	@brief		卡片初始化.
*	@param		
*	@retval		0:success
*				1:false.			
***********************************************************************************************/
int8_t LF125K_Card_Init(void)
{
	uint8_t  buf[10];
	write_od(0x10);
	write_lock(0x00);
	data_tap[0]=0x00;
	data_tap[1]=0x28;//0x28;
	data_tap[2]=0x01;
	data_tap[3]=0x17;
	write_data();
	write_add(0x00);
	SYSTICK_Delay10ms(100);
	REM_Processing(buf);
	return (LF125K_Read_1_Page(buf));
}

/**********************************************************************************************
*	@function	void LF125K_Set_Password (void)
*	@brief		加密卡片
*	@param		
*	@retval		
*						
***********************************************************************************************/
void LF125K_Set_Password (void)
{
	write_od(0x10);
	write_lock(0x00);
	data_tap[0]=0x00;
	data_tap[1]=0x28;
	data_tap[2]=0x41;
	data_tap[3]=0x1f;
	write_data();
	write_add(0x00);
}
/**********************************************************************************************
*	@function	int8_t LF125K_Set_PWD (uint8_t *pwd)
*	@brief		
*	@param		
*	@retval		
*						
***********************************************************************************************/
int8_t LF125K_Set_PWD (uint8_t *pwd)
{
	if(LF125K_Write_0_Page(7,pwd) != 0)
		return 1;
	write_od(0x10);
	write_lock(0x00);
	data_tap[0]=0x00;
	data_tap[1]=0x28;
	data_tap[2]=0x41;
	data_tap[3]=0x1f;
	write_data();
	write_add(0x00);
	return 0;
}


/**********************************************************************************************
*	@function	int8_t LF125K_Clear_PWD (uint8_t *pwd)
*	@brief		清除卡片密码.
*	@param		
*	@retval		
*						
***********************************************************************************************/
int8_t LF125K_Clear_PWD (uint8_t *pwd)
{
	uint8_t buf[10];
	write_od(0x10);
	data_tap[0]=pwd[0];
	data_tap[1]=pwd[1];
	data_tap[2]=pwd[2];
	data_tap[3]=pwd[3];
	write_data();
	write_lock(0x00);
	data_tap[0]=0x00;
	data_tap[1]=0x28;
	data_tap[2]=0x01;
	data_tap[3]=0x17;
	write_data();
	write_add(0);
	SYSTICK_Delay10ms(1);
	if(REM_Processing(buf))
		return 1;	
	return 0;
}
/**********************************************************************************************
*	@brief		获取 ID Card 卡号
*	@param		buf: 获取 卡号 的数据
*	@retval		0:
*				1:					
***********************************************************************************************/
int8_t LF125K_Get_CardID (uint8_t *buf)
{
	uint8_t tmp_buf[10];
	if(LF125K_Read_1_Page(tmp_buf))
	{
		buf[0] = tmp_buf[4];
		buf[1] = tmp_buf[5];
		buf[2] = tmp_buf[6];
		buf[3] = tmp_buf[7];
	}
	return 0;
}

/**********************************************************
*@brief		判断两个字符串是否一样
*@param		a: 字符串A
*			b: 字符串B
*@return 	0: 相等
*			1: 不相等
***********************************************************/
static uint8_t Is_Equal(const char *a,const char *b)
{
	u8 i=0;
	for(i=0;i<8;i++)
	{
		if(a[i] != b[i])
		{
			return 1;
		}
	}
	return 0;
}

/*******************************************************
*@brief 
*@param
*@retval 
********************************************************/
int8_t LF125K_SwipeCard(void)
{
	uint8_t temp_buf[8]={0};
	if(LF125K_Read_1_Page(temp_buf) == 0)
	{
		printf("%s\r\n",temp_buf);
//		if(strcmp((const char*)temp_buf,(const char*)id_card1) == 0)
		if(!Is_Equal((const char*)temp_buf,(const char*)id_card1))
			return ID_CARD1;
		else if(!Is_Equal((const char*)temp_buf,(const char*)id_card3))
			return ID_CARD3;
	}
	return 0;
}

/*****************************************************
*@brief		获取 LF Card 中的信息
*@param		None
*@retval	None
******************************************************/
int8_t LF125K_Get_CardInfo(uint8_t addr)
{
	int8_t ret = 0;
	
	// 清除缓存
	memset(lf_page_0_buf,0x00,sizeof(lf_page_0_buf));
	memset(lf_page_1_buf,0x00,sizeof(lf_page_1_buf));
	
	switch(addr)
	{
		case PAGE_0_BLOCK_0:
			// Read page 0 block 0
			ret = LF125K_Read_0_Page(0,lf_page_0_buf[0]);
			if(ret != 0)
			{
				ret = -1;
				return ret;
			}
			break;
		case PAGE_0_BLOCK_1:
			// Read page 0 block 1
			ret = LF125K_Read_0_Page(1,lf_page_0_buf[1]);
			if(ret != 0)
			{
				ret = -2;
				return ret;
			}
			break;
		case PAGE_0_BLOCK_2:
			// Read page 0 block 2
			ret = LF125K_Read_0_Page(2,lf_page_0_buf[2]);
			if(ret != 0)
			{
				ret = -3;
				return ret;
			}
			break;
		case PAGE_0_BLOCK_3:
			// Read page 0 block 3
			ret = LF125K_Read_0_Page(3,lf_page_0_buf[3]);
			if(ret != 0)
			{
				ret = -4;
				return ret;
			}
			break;
		case PAGE_0_BLOCK_4:
			// Read page 0 block 4
			ret = LF125K_Read_0_Page(4,lf_page_0_buf[3]);
			if(ret != 0)
			{
				ret = -5;
				return ret;
			}
			break;
		case PAGE_0_BLOCK_5:
			// Read page 0 block 5
			ret = LF125K_Read_0_Page(5,lf_page_0_buf[3]);
			if(ret != 0)
			{
				ret = -6;
				return ret;
			}
			break;
		case PAGE_0_BLOCK_6:
			// Read page 0 block 6
			ret = LF125K_Read_0_Page(6,lf_page_0_buf[3]);
			if(ret != 0)
			{
				ret = -7;
				return ret;
			}
			break;
		case PAGE_0_BLOCK_7:
			// Read page 0 block 7
			ret = LF125K_Read_0_Page(7,lf_page_0_buf[3]);
			if(ret != 0)
			{
				ret = -8;
				return ret;
			}
			break;
		case PAGE_1:
			// Read page 1
			ret = LF125K_Read_1_Page(lf_page_1_buf);
			if(ret != 0)
			{
				ret = -9;
				return ret;
			}
			break;
		case PAGE_ALL:
			// Read page all
			ret = LF125K_Read_0_Page(0,lf_page_0_buf[3]);
			if(ret != 0)
			{
				ret = -1;
				return ret;
			}
			ret = LF125K_Read_0_Page(1,lf_page_0_buf[3]);
			if(ret != 0)
			{
				ret = -2;
				return ret;
			}
			ret = LF125K_Read_0_Page(2,lf_page_0_buf[3]);
			if(ret != 0)
			{
				ret = -3;
				return ret;
			}
			ret = LF125K_Read_0_Page(3,lf_page_0_buf[3]);
			if(ret != 0)
			{
				ret = -4;
				return ret;
			}
			ret = LF125K_Read_0_Page(4,lf_page_0_buf[4]);
			if(ret != 0)
			{
				ret = -5;
				return ret;
			}
			ret = LF125K_Read_0_Page(5,lf_page_0_buf[5]);
			if(ret != 0)
			{
				ret = -6;
				return ret;
			}
			ret = LF125K_Read_0_Page(6,lf_page_0_buf[6]);
			if(ret != 0)
			{
				ret = -7;
				return ret;
			}
			ret = LF125K_Read_0_Page(7,lf_page_0_buf[7]);
			if(ret != 0)
			{
				ret = -8;
				return ret;
			}
			
			// Read page 1
			ret = LF125K_Read_1_Page(lf_page_1_buf);
			if(ret != 0)
			{
				ret = -9;
				return ret;
			}
			
		
		break;
	}
	
	return ret;
	
}


/*****************************************************
*@brief		显示LF Card中的信息
*@param		addr：要显示的LF Card 信息的位置
*	@arg	PAGE_0_BLOCK_0:	显示 PAGE 0 的 BLOCK 0 中的信息
*	@arg	PAGE_0_BLOCK_1: 显示 PAGE 0 的 BLOCK 1 中的信息
*	@arg	PAGE_0_BLOCK_2: 显示 PAGE 0 的 BLOCK 2 中的信息
*	@arg	PAGE_0_BLOCK_3: 显示 PAGE 0 的 BLOCK 3 中的信息
*	@arg	PAGE_0_BLOCK_4: 显示 PAGE 0 的 BLOCK 4 中的信息
*	@arg	PAGE_0_BLOCK_5: 显示 PAGE 0 的 BLOCK 5 中的信息
*	@arg	PAGE_0_BLOCK_6: 显示 PAGE 0 的 BLOCK 6 中的信息
*	@arg	PAGE_0_BLOCK_7: 显示 PAGE 0 的 BLOCK 7 中的信息
*	@arg	PAGE_1: 显示 PAGE 1 中的信息
*	@arg	PAGE_ALL: 显示 LF CARD 中的所有信息
*@retval	None
*@attention	一定要在读取卡的信息后才使用此函数
******************************************************/
void LF125K_Show_CardInfo(uint8_t addr)
{
	uint8_t i=0;
	switch(addr)
	{
		case PAGE_0_BLOCK_0:
			printf("Page 0 block 0:");
			printf_hex(lf_page_0_buf[0],4);
			printf("\r\n");
			break;
		case PAGE_0_BLOCK_1:
			printf("Page 0 block 1:");
			printf_hex(lf_page_0_buf[1],4);
			printf("\r\n");
			break;
		case PAGE_0_BLOCK_2:
			printf("Page 0 block 2:");
			printf_hex(lf_page_0_buf[2],4);
			printf("\r\n");
			break;
		case PAGE_0_BLOCK_3:
			printf("Page 0 block 3:");
			printf_hex(lf_page_0_buf[3],4);
			printf("\r\n");
			break;
		case PAGE_0_BLOCK_4:
			printf("Page 0 block 4:");
			printf_hex(lf_page_0_buf[4],4);
			printf("\r\n");
			break;
		case PAGE_0_BLOCK_5:
			printf("Page 0 block 5:");
			printf_hex(lf_page_0_buf[5],4);
			printf("\r\n");
			break;
		case PAGE_0_BLOCK_6:
			printf("Page 0 block 6:");
			printf_hex(lf_page_0_buf[6],4);
			printf("\r\n");
			break;
		case PAGE_0_BLOCK_7:
			printf("Page 0 block 7:");
			printf_hex(lf_page_0_buf[7],4);
			printf("\r\n");
			break;
		case PAGE_1:
			printf("Page 1:");
			printf_hex(lf_page_1_buf,8);
			break;
		case PAGE_ALL:
			// print page 0 info
			for(i=0;i<8;i++)
			{	
				printf("Page 0 block %d:",i);
				printf_hex(lf_page_0_buf[i],4);
				printf("\r\n");
			}
			// print page 1 info
			printf("Page 1:");
			printf_hex(lf_page_1_buf,8);
			break;
	}
	
}
