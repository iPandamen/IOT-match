
#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"
#include "keyboard.h"
#include "delay.h"
#include "LCD12864.h"
#include "timer.h"


static const uint32_t kbd_columns_port_rcc[KBD_COLUMN_SUM]={RCC_KBD_COLUMN0_PORT,RCC_KBD_COLUMN1_PORT,RCC_KBD_COLUMN2_PORT,RCC_KBD_COLUMN3_PORT,RCC_KBD_COLUMN4_PORT};
static GPIO_TypeDef * kbd_columns_port[KBD_COLUMN_SUM] = {KBD_COLUMN0_PORT,KBD_COLUMN1_PORT,KBD_COLUMN2_PORT,KBD_COLUMN3_PORT,KBD_COLUMN4_PORT};
static const uint16_t kbd_columns_pin[KBD_COLUMN_SUM] = {KBD_COLUMN0_PIN,KBD_COLUMN1_PIN,KBD_COLUMN2_PIN,KBD_COLUMN3_PIN,KBD_COLUMN4_PIN};

static const uint32_t kbd_rows_port_rcc[KBD_ROW_SUM]={RCC_KBD_ROW0_PORT,RCC_KBD_ROW1_PORT,RCC_KBD_ROW2_PORT,RCC_KBD_ROW3_PORT,RCC_KBD_ROW4_PORT};
static GPIO_TypeDef * kbd_rows_port[KBD_ROW_SUM] = {KBD_ROW0_PORT,KBD_ROW1_PORT,KBD_ROW2_PORT,KBD_ROW3_PORT,KBD_ROW4_PORT};
static const uint16_t kbd_rows_pin[KBD_ROW_SUM] = {KBD_ROW0_PIN,KBD_ROW1_PIN,KBD_ROW2_PIN,KBD_ROW3_PIN,KBD_ROW4_PIN};


static unsigned char kbd_status_buf1[KBD_ROW_SUM][KBD_COLUMN_SUM];
static unsigned char kbd_status_buf2[KBD_ROW_SUM][KBD_COLUMN_SUM];
static unsigned char kbd_status[KBD_ROW_SUM][KBD_COLUMN_SUM];
static KEY_EVT_FIFO evt_fifo;


unsigned char key_code_cnvt[KBD_ROW_SUM*KBD_COLUMN_SUM + 1] = 
{
	KEY_INVALID,
	KBD_PROJ5,
	KEY_BACKSPACE,
	KBD_KEY6,
	KEY_ESC,
	KEY_ENTER,
	KBD_PROJ4,
	KBD_KEY3,
	KBD_KEY5,
	KBD_KEY9,
	KEY_HASH_KEY,
	KBD_PROJ3,
	KBD_KEY2,
	KBD_KEY4,
	KBD_KEY8,
	KBD_KEY0,
	KBD_PROJ2,
	KBD_KEY1,
	KBD_RIGHT,
	KBD_KEY7,
	KEY_KPASTERISK,
	KBD_PROJ1,
	KBD_UP,
	KBD_LEFT,
	KBD_DOWN,
	KBD_FUNC,
};

/************************************************************
*	@breef	键盘初始化
*	@param	None
*	@retval	None
*************************************************************/
void kbd_init(void)
{
	//init keyboard data
	memset(kbd_status_buf1, KBD_KEY_UP, sizeof(kbd_status_buf1));
	memset(kbd_status_buf2, KBD_KEY_UP, sizeof(kbd_status_buf2));
	memset(kbd_status, KBD_KEY_UP, sizeof(kbd_status));
	memset((void*)&evt_fifo, 0, sizeof(evt_fifo));

	init_kbd_gpio();
}

/*******************************************************************************
*	@function	int init_kbd_GPIO(void)
*	@brief		初始化键盘模块的 GPIO 引脚
*	@param		None
*	@retval		None
********************************************************************************/
void init_kbd_gpio(void)
{
	u8 i = 0;
	// 将每一行的GPIO引脚设置为上拉输入模式
	for(i=0;i<KBD_ROW_SUM;i++)
	{
		STM32_GPIOx_Init(kbd_rows_port_rcc[i],kbd_rows_port[i],kbd_rows_pin[i],GPIO_Speed_50MHz,GPIO_Mode_IPU);
	}
	// 将每一列的GPIO引脚设置为推挽输出模式
	for(i=0;i<KBD_COLUMN_SUM;i++)
	{
		STM32_GPIOx_Init(kbd_columns_port_rcc[i],kbd_columns_port[i],kbd_columns_pin[i],GPIO_Speed_50MHz,GPIO_Mode_Out_PP);
		//set all column gpios to 0
		GPIO_ResetBits(kbd_columns_port[i], kbd_columns_pin[i]); 
	}
	
}

/***************************************
*	@function	unsigned char is_kbd_fifo_empt(void)
*	@brief 		判断键盘 FIFO 是否为空
*	@param		None
*	@retval		
****************************************/
unsigned char is_kbd_fifo_empt(void)
{
	return evt_fifo.evt_r_indx == evt_fifo.evt_w_indx;
}

/*********************************************************
*@brief		获取KBD_FIFO的长度
*@param		None
*@retval	返回 KBD_FIFO 长度
**********************************************************/
unsigned char kbd_fifo_len(void)
{
	if(evt_fifo.evt_w_indx > evt_fifo.evt_r_indx)
	{
		return evt_fifo.evt_w_indx - evt_fifo.evt_r_indx;
	}
	else
	{
		return evt_fifo.evt_w_indx - evt_fifo.evt_r_indx + KEY_EVENT_BUF_SIZE;
	}
}

/*****************************************************
*@brief		添加按键事件进入FIFO
*@param		evt:要添加的按键事件
*@retval	None
******************************************************/
void put_kbd_fifo(KBD_EVENT* evt)
{
	memcpy((void*)&evt_fifo.key_evt_buf[evt_fifo.evt_w_indx], (void*)evt, sizeof(KBD_EVENT));
	evt_fifo.evt_w_indx = (++evt_fifo.evt_w_indx) % KEY_EVENT_BUF_SIZE;
}

/*****************************************************
*@brief		获取 KBD FIFO 中的按键事件
*@param		evt: 用于存储获取的按键事件
*@retval	0：	success
*			-1: faild
******************************************************/
int get_kbd_fifo(KBD_EVENT* evt)
{
	if(is_kbd_fifo_empt())
		return -1;
	memcpy((void*)evt, (void*)&evt_fifo.key_evt_buf[evt_fifo.evt_r_indx], sizeof(KBD_EVENT));
	evt_fifo.evt_r_indx = (++evt_fifo.evt_r_indx) % KEY_EVENT_BUF_SIZE;
	
	return 0;
}

/*************************************************************
*@brief 	设置每一列的GPIO引脚的输出电平
*@param		val：GPIO引脚状态的二进制数据
*			0~4bit 分别对应 column 0~4 的输出电平
*@retval	None
**************************************************************/
void kbd_column_gpio_set(uint8_t val)
{
	for(uint8_t i=0;i<KBD_COLUMN_SUM;i++)
	{
		if(val & (0x01<<i))
			GPIO_SetBits(kbd_columns_port[i],kbd_columns_pin[i]);
		else 
			GPIO_ResetBits(kbd_columns_port[i],kbd_columns_pin[i]);
	}
}

/*************************************************************
*@brief 	读取每一行的GPIO引脚的状态
*@param		None
*@retval	返回每一行的GPIO引脚状态
*			0-4bit对应着 row 0-4 引脚的状态		
**************************************************************/
uint8_t kbd_row_gpio_read(void)
{
	uint8_t val=0,temp=0;
	for(uint8_t i=0;i<KBD_COLUMN_SUM;i++)
	{
		temp = GPIO_ReadInputDataBit(kbd_rows_port[i],kbd_rows_pin[i]);
		val |= (temp<<i);
	}
	return val;
}
/****************************************************
*	@function	void kbd_scan(void)
*	@brief		按键扫描
*	@param		None
*	@retval		None
*****************************************************/
void kbd_scan(void)
{
	int i;
	int j;
	static unsigned char(*kbd_now)[KBD_COLUMN_SUM] = kbd_status_buf1;
	static unsigned char(*kbd_prev)[KBD_COLUMN_SUM] = kbd_status_buf2;
	static unsigned char(*p_tmp)[KBD_COLUMN_SUM];
	static unsigned char kbd_same_cnt[KBD_ROW_SUM] [KBD_COLUMN_SUM] = {{0}};
	static unsigned char key_evt_flag = 0;
	static unsigned char act_flag = 0;
	KBD_EVENT evt;
	unsigned char chg_flag = 0;
	unsigned long kbd_data;

	#ifdef KBD_HOLD_KEY_SCAN_ON
	static unsigned char hold_key_cnt = KBD_SAMPLE_COUNT_FOR_HOLD_KEY;
	#endif
	
	if (key_evt_flag == 0 && act_flag == 0) 
	{
		//set all column gpios to 0
//		GPIO_ResetBits(KBD_COLUMN_PORT, KBD_COLUMN_PINS);
		kbd_column_gpio_set(0x00);
		
//		if ((GPIO_ReadInputData(KBD_ROW_PORT) & 0x1f) == 0x1f) 
		if((kbd_row_gpio_read() & 0x1f) == 0x1f)
		{
			//no key dow
			return;
		}//end no key input 	
	}

	//read the now status
	for(i = 0; i < KBD_ROW_SUM; i++) 
	{
		//set all columns to 1
//		GPIO_SetBits(KBD_COLUMN_PORT, KBD_COLUMN_PINS);
		kbd_column_gpio_set(0x1f);
		
		//set column(0~4) data
//		GPIO_ResetBits(kbd_columns_port[i], kbd_columns_pin[i]);
		kbd_column_gpio_set(~(0x01<<i));
		
		//delay 1 us, make the signal stable
		delay_us(1);

//		kbd_data = GPIO_ReadInputData(KBD_ROWS_PORT);
		kbd_data = kbd_row_gpio_read();
		for(j = 0; j < KBD_COLUMN_SUM; j++) 
		{
			kbd_now[i][j] = (kbd_data >> j) & 0x01;
		}
	}

	for(i = 0; i < KBD_ROW_SUM; i++) 
	{
		for(j = 0; j < KBD_COLUMN_SUM; j++) 
		{

			//now status is same with previously status, then calculate the same times
			if (kbd_now[i][j] == kbd_prev[i][j]) 
			{
				//keyboard status is not changed, set the same count to MAX
				if ( ++kbd_same_cnt[i][j] > KBD_SAMPLE_COUNT_FOR_HOLD_KEY ) 
				{
					kbd_same_cnt[i][j] = KBD_SAMPLE_COUNT_FOR_HOLD_KEY;
				}
				//the same count must greater than KBD_SAMPLE_COUNT to guarantee the status is effective	防抖动
				if ( (kbd_same_cnt[i][j] >= KBD_SAMPLE_COUNT) && (kbd_now[i][j] != kbd_status[i][j]) ) 
				{
					//keep the real status
					kbd_status[i][j] = kbd_now[i][j];

					//keep key event cout
					if (kbd_status[i][j] == KBD_KEY_DONW) 
					{
						key_evt_flag++;
					} 
					else 
					{
						key_evt_flag--;
						#ifdef KBD_HOLD_KEY_SCAN_ON
						hold_key_cnt = KBD_SAMPLE_COUNT_FOR_HOLD_KEY;
						#endif
					}

					evt.key_num = i*KBD_COLUMN_SUM + j + 1;
					evt.key_event = kbd_status[i][j];

					//put the keyboard evnet to buffer
					evt.key_num = key_code_cnvt[evt.key_num];
					put_kbd_fifo(&evt);
					
					//printf("keyboard event key:%d event: %d\n", evt.key_num, evt.key_event);

				}
				#ifdef KBD_HOLD_KEY_SCAN_ON
				else if ( (kbd_same_cnt[i][j] >= hold_key_cnt) && (kbd_now[i][j] == KBD_KEY_DONW) )
				{
					evt.key_num = i*KBD_COLUMN_SUM + j + 1;
					evt.key_event = KBD_KEY_HOLD;

					//put the keyboard evnet to buffer
					evt.key_num = key_code_cnvt[evt.key_num];
					put_kbd_fifo(&evt);

					//keyboard status is changed, then count the new status
					kbd_same_cnt[i][j] = 1;
					hold_key_cnt = KBD_SAMPLE_COUNT * 2;
					//printf("keyboard event key:%d event: %d\r\n", evt.key_num, evt.key_event);
				}
				#endif
			} 
			else 
			{
				//keyboard status is changed, then count the new status
				kbd_same_cnt[i][j] = 1;
				chg_flag = 1;
			}
		}
	}
	if (chg_flag) 
	{
		//keep the now status
		p_tmp = kbd_now;
		kbd_now = kbd_prev;
		kbd_prev = p_tmp;
		
		//keyboard has action
		act_flag = 1;
	} 
	else
	{
		//keyboard not has action
		act_flag = 0;
	}
}

/*********************************************
*@brief		打印 按键 消息
*@param		key: 打印按键的事件信息	
*@retval	None	
**********************************************/
void print_key_info(KBD_EVENT* key)
{
	char str[LCD_COLUMN_NUM+1];

	memset(str, 0, sizeof(str));
	lcd_clr_row(0);
	
	if(key->key_num <= 9)
	{
		if(key->key_event == KBD_KEY_DONW)
		{
			sprintf(str, "Key %d down", key->key_num);
		}
		else
		{
			sprintf(str, "Key %d up", key->key_num);
		}
		lcd_write(0, 0, str, strlen(str));
	}
	else
	{
		switch(key->key_num)
		{
		case KBD_UP :
			if(key->key_event == KBD_KEY_DONW)
			{
				sprintf(str, "UP KEY down");
			}
			else
			{
				sprintf(str, "UP KEY up");
			}
			break;
		case KBD_DOWN :
			if(key->key_event == KBD_KEY_DONW)
			{
				sprintf(str, "DOWN KEY down");
			}
			else
			{
				sprintf(str, "DOWN KEY up");
			}
			break;
		case KBD_LEFT :
			if(key->key_event == KBD_KEY_DONW)
			{
				sprintf(str, "LEFT KEY down");
			}
			else
			{
				sprintf(str, "LEFT KEY up");
			}
			break;
		case KBD_RIGHT :
			if(key->key_event == KBD_KEY_DONW)
			{
				sprintf(str, "RIGHT KEY down");
			}
			else
			{
				sprintf(str, "RIGHT KEY up");
			}
			break;
		case KBD_PROJ1 :
			if(key->key_event == KBD_KEY_DONW)
			{
				sprintf(str, "PROJ1 KEY down");
			}
			else
			{
				sprintf(str, "PROJ1 KEY up");
			}
			break;
		case KBD_PROJ2 :
			if(key->key_event == KBD_KEY_DONW)
			{
				sprintf(str, "PROJ2 KEY down");
			}
			else
			{	
				sprintf(str, "PROJ2 KEY up");
			}
			break;
		case KBD_PROJ3 :
			if(key->key_event == KBD_KEY_DONW)
			{
				sprintf(str, "PROJ3 KEY down");
			}
			else
			{
				sprintf(str, "PROJ3 KEY up");
			}
			break;
		case KBD_PROJ4 :
			if(key->key_event == KBD_KEY_DONW)
			{
				sprintf(str, "PROJ4 KEY down");
			}
			else
			{
				sprintf(str, "PROJ4 KEY up");
			}
			break;
		case KBD_PROJ5 :
			if(key->key_event == KBD_KEY_DONW)
			{
				sprintf(str, "PROJ5 KEY down");
			}
			else
			{
				sprintf(str, "PROJ5 KEY up");
			}
			break;
		case KBD_FUNC :
			if(key->key_event == KBD_KEY_DONW)
			{
				sprintf(str, "FUNC KEY down");
			}
			else
			{
				sprintf(str, "FUNC KEY up");
			}
			break;
		case KEY_HASH_KEY :
			if(key->key_event == KBD_KEY_DONW)
			{
				sprintf(str, "HASH KEY down");
			}
			else
			{
				sprintf(str, "HASH KEY up");
			}
			break;
		case KEY_KPASTERISK :
			if(key->key_event == KBD_KEY_DONW)
			{
				sprintf(str, "KPASTERISK KEY down");
			}
			else
			{
				sprintf(str, "KPASTERISK KEY up");
			}
			break;
		case KEY_BACKSPACE :
			if(key->key_event == KBD_KEY_DONW)
			{
				sprintf(str, "BACKSPACE KEY down");
			}
			else
			{
				sprintf(str, "BACKSPACE KEY up");
			}
			break;
		case KEY_ENTER :
			if(key->key_event == KBD_KEY_DONW)
			{
				sprintf(str, "ENTER KEY down");
			}
			else
			{
				sprintf(str, "ENTER KEY up");
			}
			break;
		case KEY_ESC :
			if(key->key_event == KBD_KEY_DONW)
			{
				sprintf(str, "ESC KEY down");
			}
			else
			{
				sprintf(str, "ESC KEY up");
			}
			break;
		default:
			break;
		}
		lcd_write(0, 0, str, strlen(str));
		printf("%s\r\n",str);
	}
}

