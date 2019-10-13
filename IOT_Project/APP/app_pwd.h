
/******************************************************************************
* @file    app/app_pwd.h 
* @author  zhao
* @version V1.0.0
* @date    2018.06.18
* @brief   密码eeprom管理 头文件
******************************************************************************/
#ifndef __APP_PWD_H__
#define __APP_PWD_H__
#include "bsp_flash.h"
#include <stdio.h>
#include <stdlib.h>
#include "delay.h"

//MEMSDataTypedef 参数
#define ADC_CHANNEL_NUM		5

typedef struct
{
	unsigned short theshold;
	unsigned short static_ADCValue[ADC_CHANNEL_NUM];
}MEMSDataTypedef;


extern enum
{
	PWD_ERROR,
	PWD_SUCCESS,
}PWD_STATUS;

extern enum
{
	EEP_SAVE_FLAG = 0,
	EEP_DIGIT_PWD_OFFSET = 7,
	EEP_DIGIT_PWD_START = 8,
	EEP_ID_CARD_PWD_OFFSET = 71,
	EEP_ID_CARD_PWD_START = 72,
	EEP_MEMS_START = 112,
}EEP_MAP;

/**
 * EEPROM start address in flash
 * As for STM32F103VE (512KB flash), 0x0807F000 is the begining of the last 2 pages.
 * Check the *.map file (in listing folder) to find out the used ROM size.
 */
#define EEP_SAVE_FLAG_ADDRESS    		((uint32_t)0x0807D800)
#define EEP_DIGIT_PWD_OFFSET_ADDRESS   	((uint32_t)0x0807E000)
#define EEP_DIGIT_PWD_START_ADDRESS   	((uint32_t)0x0807E800)
#define EEP_ID_CARD_PWD_OFFSET_ADDRESS  ((uint32_t)0x0807F000)
#define EEP_ID_CARD_PWD_START_ADDRESS   ((uint32_t)0x0807F800)
#define EEP_MEMS_START_ADDRESS    		((uint32_t)0x0807D000)


//Digit password
#define MAX_DIGIT_PWD_NUM 5	 //可存储的密码个数
#define DIGIT_PWD_LEN 6		 //数字密码长度

extern unsigned char DigitPwd_Cache[MAX_DIGIT_PWD_NUM][DIGIT_PWD_LEN];	//数字密码缓存
extern unsigned char DigitPwd_Offset;		//密码保存在缓存中的位置
extern unsigned char CurDigitPwd_Cache[DIGIT_PWD_LEN];		//当前输入的密码
extern unsigned char CurDigitPwd_Offset;	//当前输入密码到第几位

//LF id card password
#define MAX_ID_CARD_PWD_NUM 5		//可存储的IDcard密码个数
#define ID_CARD_PWD_LEN 	8		//ID card密码长度

extern unsigned char IDCardPwd_Cache[MAX_ID_CARD_PWD_NUM][ID_CARD_PWD_LEN];	//ID card密码缓存
extern unsigned char IDCardPwd_Offset;		//密码保存在缓存中的位置
extern unsigned char CurIDCardPwd_Cache[ID_CARD_PWD_LEN];	//当前输入的密码

extern MEMSDataTypedef memsData;

void clear_pwdcache(void);	//清空输入密码缓存

int IsRight_Pwd_Digit(unsigned char pwd[],int len);	//判断数字密码是否正确

int IsRight_Pwd_IdCard(unsigned char pwd[],int len); //判断ID卡密码是否正确
	
void param_init(void);

int Save_DigitPwd(unsigned char pwd[],int len);	//保存 Digit passsword

unsigned char **Read_DigitPwd(void);// 读取数字密码

int Save_IDCardPwd(unsigned char pwd[],int len);	//保存 IDCard password

int Delete_IDCardPwd(unsigned char pwd[],int len);	//删除 ID Card passsword

unsigned char **Read_IDCardPwd(void);	// 读 ID Card passsword

void Save_MemsData(void);	//save memdata

void Read_MemsData(void);	//read memdata


#endif //__APP_PWD_H__

