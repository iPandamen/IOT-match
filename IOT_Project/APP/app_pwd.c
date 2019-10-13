/******************************************************************************
* @file    app/app_pwd.c 
* @author  zhao
* @version V1.0.0
* @date    2018.06.18
* @brief   密码eeprom管理
******************************************************************************/
#include "app_pwd.h"
#define __DEBUG__  
#ifdef __DEBUG__  
#define DEBUG(format,...) printf("File: "__FILE__", Line: %05d: "format"\r\n", __LINE__, ##__VA_ARGS__)  
#else  
#define DEBUG(format,...)  
#endif  

// Digital passwd
unsigned char DigitPwd_Cache[MAX_DIGIT_PWD_NUM][DIGIT_PWD_LEN];	//数字密码缓存
unsigned char DigitPwd_Offset = 0;		//密码保存在缓存中的位置
unsigned char CurDigitPwd_Cache[DIGIT_PWD_LEN];	//当前输入的密码
unsigned char CurDigitPwd_Offset = 0;	//当前输入密码到第几位

//LF ID card password
unsigned char IDCardPwd_Cache[MAX_ID_CARD_PWD_NUM][ID_CARD_PWD_LEN];	//ID card密码缓存
unsigned char IDCardPwd_Offset = 0;	//密码保存在缓存中的位置
unsigned char CurIDCardPwd_Cache[ID_CARD_PWD_LEN];	//当前输入的密码


MEMSDataTypedef memsData = 
{
	.static_ADCValue[0] = 0,
	.static_ADCValue[1] = 0,
	.static_ADCValue[2] = 0,
};


/*********************************************************
*	@brief 	ClearPwdCache 清空输入密码缓存
*	@param 	None
*	@retur	None
*	@note  	1.参数初始化
*			2.是否初次上电  第一次上电 重置密码 
**********************************************************/
void clear_pwdcache(void)
{
	CurDigitPwd_Offset = 0;
	memset(CurDigitPwd_Cache, 0, DIGIT_PWD_LEN);  // 清空当前输入的数字密码缓存
	memset(CurIDCardPwd_Cache, 0, ID_CARD_PWD_LEN); // 清空 ID Card 的密码缓存
}

/**********************************************************
*	@brief  	IsRightPwd_Digit 判断数字密码是否正确
*	@param  	pwd:需判断的数字密码
*				len:数字密码的长度	
*	@retval 	数字密码是否正确
*		@val	PWD_SUCCESS:数字密码正确
*		@val	PWD_ERROR:数字密码错误	
*	@note  		1.参数初始化
*               2.是否初次上电  第一次上电 重置密码 
***********************************************************/
int IsRight_Pwd_Digit(unsigned char pwd[],int len)
{
	int i;
	if(len != DIGIT_PWD_LEN) //判断长度
	{
		printf("Digit Password error: Please enter %d-digit password!\r\n",DIGIT_PWD_LEN);
		return PWD_ERROR; // end 密码长度不匹配
	}
	for(i=0; i < MAX_DIGIT_PWD_NUM; i++)
	{
		if(memcmp(pwd, DigitPwd_Cache[i], DIGIT_PWD_LEN) == 0) // 在以缓存的密码中进行比对
		{
			break;
		}
	}
	if(i < MAX_DIGIT_PWD_NUM)	// 在缓存的 MAX_DIGIT_PWD_NUM 个密码中，找到了该数字密码
	{
		return PWD_SUCCESS; // 该数字密码正确
	}
	else	// 在缓存的 MAX_DIGIT_PWD_NUM 个密码中，没有找到该数字密码
	{
		printf("Digit Password error: Cannot find the matching password!\r\n");
		return PWD_ERROR;
	}
}

/***********************************************************
*	@brief  IsRightPwd_IdCard 判断ID卡密码是否正确
*	@param  none * 
*	@return	PWD_SUCCESS  
*			PWD_ERROR
*	@note  1.参数初始化
*	       2.是否初次上电  第一次上电 重置密码 
************************************************************/
int IsRight_Pwd_IdCard(unsigned char pwd[],int len)
{
	int i;
	Read_IDCardPwd();
	if(len != ID_CARD_PWD_LEN)
	{
		printf("ID Card Password error：Please enter %d-digit password!\r\n",ID_CARD_PWD_LEN);	
		return PWD_ERROR;
	}
	for(i=0; i < MAX_ID_CARD_PWD_NUM; i++)
	{
		if(memcmp(pwd, &IDCardPwd_Cache[i][0], ID_CARD_PWD_LEN) == 0)
		{
			break;
		}
	}
	printf("ID PWD's i=%d\r\n",i);
	if(i < MAX_ID_CARD_PWD_NUM)	// 在缓存的 MAX_DIGIT_PWD_NUM 个密码中，找到了该ID Card password
	{	
		return PWD_SUCCESS;
	}
	else
	{
		printf("ID Card Password error: Cannot find the matching password!\r\n");
		return PWD_ERROR;// 在缓存的 MAX_DIGIT_PWD_NUM 个密码中，没有找到该ID Card password
	}
}


/********************************************************
*	@brief  paramInit
*	@param  none
*	@return none
*	@note  	1.参数初始化
*			2.是否初次上电  第一次上电 重置密码 
*		   	3.如果是第一次上电0 byte 写入 0xA3
*********************************************************/
void param_init(void)
{
	unsigned char save_flag;
	unsigned short NumByteToRead = 1;
	unsigned short NumByteToWrite;
	int i;
	
    bsp_flash_read(EEP_SAVE_FLAG_ADDRESS, &save_flag, NumByteToRead);

//	DEBUG("%x", save_flag);
	if(save_flag != 0xA5)
	{
		save_flag = 0xA5;

		// 0byte write 0xA3
		NumByteToWrite = 1;
		
		bsp_flash_erase(EEP_SAVE_FLAG_ADDRESS, NumByteToWrite);
		bsp_flash_write(EEP_SAVE_FLAG_ADDRESS, &save_flag, NumByteToWrite);
		
		//digit pwd Init
		for(i=0; i < MAX_DIGIT_PWD_NUM; i++)
		{
			if(i == 0)
			{
				memset(DigitPwd_Cache[i], 0x00, DIGIT_PWD_LEN);//default pwd 000000
			}
			else
			{
				memset(DigitPwd_Cache[i], 0xEE, DIGIT_PWD_LEN);
			}
		}
		
		// Write DigitPwd_Offset
		NumByteToWrite = 1;
		
		bsp_flash_erase(EEP_DIGIT_PWD_OFFSET_ADDRESS, NumByteToWrite);
		bsp_flash_write(EEP_DIGIT_PWD_OFFSET_ADDRESS, &DigitPwd_Offset, NumByteToWrite);
		// Write DigitPwd_Cache
		NumByteToWrite = MAX_DIGIT_PWD_NUM * DIGIT_PWD_LEN;
		
		bsp_flash_erase(EEP_DIGIT_PWD_START_ADDRESS, NumByteToWrite);
		bsp_flash_write(EEP_DIGIT_PWD_START_ADDRESS, (unsigned char *)DigitPwd_Cache[0], NumByteToWrite);
		
		//id card pwd init
		for(i=0; i < MAX_ID_CARD_PWD_NUM; i++)
		{
		  memset(DigitPwd_Cache[i], 0xEE, ID_CARD_PWD_LEN);
		}
		//Write IDCardPwd_Offset
		NumByteToWrite = 1;
		
		bsp_flash_erase(EEP_ID_CARD_PWD_OFFSET_ADDRESS, NumByteToWrite);
		bsp_flash_write(EEP_ID_CARD_PWD_OFFSET_ADDRESS, &IDCardPwd_Offset, NumByteToWrite);
		//Write IDCardPwd_Cache
		NumByteToWrite = MAX_ID_CARD_PWD_NUM * ID_CARD_PWD_LEN;
		
		bsp_flash_erase(EEP_ID_CARD_PWD_START_ADDRESS, NumByteToWrite);
		bsp_flash_write(EEP_ID_CARD_PWD_START_ADDRESS, (unsigned char *)IDCardPwd_Cache[0], NumByteToWrite);
		
		//mems
		memsData.theshold = 50;
		//memset(&memsData.static_ADCValue[0], 0, sizeof(memsData.static_ADCValue));
		
		NumByteToWrite = sizeof(MEMSDataTypedef);
		
		bsp_flash_erase(EEP_MEMS_START_ADDRESS, NumByteToWrite);
		bsp_flash_write(EEP_MEMS_START_ADDRESS, (unsigned char *)&memsData, NumByteToWrite);
	}
}
/*************************************************************************************
*@brief		保存 Digit passsword
*@param		pwd: 要保存的 Digit password
*			len: Digit password 的长度
*@retval	0:success
**************************************************************************************/
int Save_DigitPwd(unsigned char pwd[],int len)
{	
	unsigned short data_len = 0;
	
	if(DigitPwd_Offset>= MAX_DIGIT_PWD_NUM)
		DigitPwd_Offset = 0;	
	
	memset(DigitPwd_Cache[DigitPwd_Offset], 0xff, DIGIT_PWD_LEN);
	memcpy(DigitPwd_Cache[DigitPwd_Offset], pwd, len);
	
	DigitPwd_Offset++;
	
	// save DigitPwd_Offset
	data_len = 1;
	
	bsp_flash_erase(EEP_DIGIT_PWD_OFFSET_ADDRESS, data_len);
	bsp_flash_write(EEP_DIGIT_PWD_OFFSET_ADDRESS, &DigitPwd_Offset, data_len);
	
	//save DigitPwd_Cached
	data_len = MAX_DIGIT_PWD_NUM * DIGIT_PWD_LEN;
	bsp_flash_erase(EEP_DIGIT_PWD_START_ADDRESS, data_len);
	bsp_flash_write(EEP_DIGIT_PWD_START_ADDRESS,(unsigned char *)&DigitPwd_Cache[0][0], data_len);
	return 0;
}


/***************************************************************************
*	@brief  Read_DigitPwd 读取数字密码
*	@param  none
*	@note  	1.参数初始化
*	       	2.分配内存 calloc(128,sizeof(char))
*	       	3.获取当前缓存位置
*	       	4.DigitPwd_Cache缓存写FF
*	       	5.将digitPwd缓存写入 debug_buff  打印 密码  strcat 串联字符串
*	       	6.free 释放内存
*	@return:(unsigned char **)DigitPwd 指针地址
****************************************************************************/
unsigned char **Read_DigitPwd(void)
{	
	int i,j;
	unsigned short data_len = 0;
	char *debug_buf = calloc(128, sizeof(char));
	char temp_buf[8];
	// read DigitPwd_Offset
	data_len = 1;
	
	bsp_flash_read(EEP_DIGIT_PWD_OFFSET_ADDRESS, &DigitPwd_Offset, data_len);
	//erase DigitPwd_Cache
	memset(DigitPwd_Cache[0], 0xff, MAX_DIGIT_PWD_NUM * DIGIT_PWD_LEN);
	// read DigitPwd_Cache
	data_len = MAX_DIGIT_PWD_NUM * DIGIT_PWD_LEN;
	
	bsp_flash_read(EEP_DIGIT_PWD_START_ADDRESS, DigitPwd_Cache[0], data_len);

	for(i=0; i < MAX_DIGIT_PWD_NUM; i++)
	{
		for(j=0; j < DIGIT_PWD_LEN; j++)
		{
			sprintf(temp_buf, "%x ", DigitPwd_Cache[i][j]);
			strcat(debug_buf, temp_buf);
		}
		strcat(debug_buf, "\r\n");
	}
	DEBUG("DigitPwd_Cache: %s", debug_buf);
	delay_ms(5);
	free(debug_buf);
	return (unsigned char **)DigitPwd_Cache;
}

/***************************************************
*@breif		保存 IDCard password
*@param		pwd:要保存的 IDCard password
*			len:要保存的 IDCard password 的长度
*@retval 	0: success
****************************************************/
int Save_IDCardPwd(unsigned char pwd[],int len)
{	
	unsigned short data_len = 0;
	//static unsigned char pwd_offset = 0;
	if(IDCardPwd_Offset>= MAX_ID_CARD_PWD_NUM)
		IDCardPwd_Offset = 0;
	
	memset(IDCardPwd_Cache[IDCardPwd_Offset], 0xff, ID_CARD_PWD_LEN);
	memcpy(IDCardPwd_Cache[IDCardPwd_Offset], pwd, len);
	IDCardPwd_Offset++;
	
	//save IDCardPwd_Offset
	data_len = 1;
	bsp_flash_erase(EEP_ID_CARD_PWD_OFFSET_ADDRESS, data_len);
	bsp_flash_write(EEP_ID_CARD_PWD_OFFSET_ADDRESS,&IDCardPwd_Offset, data_len);
	
	//save IDCardPwd_Cache
	data_len = MAX_ID_CARD_PWD_NUM * ID_CARD_PWD_LEN;
	bsp_flash_erase(EEP_ID_CARD_PWD_START_ADDRESS, data_len);
	bsp_flash_write(EEP_ID_CARD_PWD_START_ADDRESS,(unsigned char *)&IDCardPwd_Cache[0][0], data_len);
	return 0;
}


/*************************************************************************************
*@brief		删除 ID Card passsword
*@param		pwd: 要保存的 ID Card passsword
*			len: ID Card passsword 的长度
*@retval	0:success
**************************************************************************************/
int Delete_IDCardPwd(unsigned char pwd[],int len)
{	
	int i;
	unsigned short data_len;
	if(len != ID_CARD_PWD_LEN)
	{
		return PWD_ERROR;
	}
	for(i=0; i < MAX_ID_CARD_PWD_NUM; i++)
	{
		if(memcmp(pwd, IDCardPwd_Cache[i], ID_CARD_PWD_LEN) == 0)
		{
			// 在 IDCardPwd_Cache 中找到了 pwd,删除密码
			memset(IDCardPwd_Cache[i], 0xff, ID_CARD_PWD_LEN);
			break;
		}
	}
	if(i < MAX_ID_CARD_PWD_NUM) // 
	{
		// 如果在 IDCardPwd_Cache 中找到了 pwd,保存修改
		data_len = MAX_ID_CARD_PWD_NUM * ID_CARD_PWD_LEN;
		
		bsp_flash_erase(EEP_ID_CARD_PWD_START_ADDRESS, data_len);
	    bsp_flash_write(EEP_ID_CARD_PWD_START_ADDRESS,(unsigned char *)&IDCardPwd_Cache[0][0], data_len);
	}
	return 0;
}

/*************************************************************************************
*@brief		读 ID Card passsword
*@param		None
*@retval	(unsigned char **)IDCardPwd_Cache -> 指针地址
**************************************************************************************/
unsigned char **Read_IDCardPwd(void)
{	
	unsigned short data_len = 0;
	// read IDCardPwd_Offset
	data_len = 1;
	
	bsp_flash_read(EEP_ID_CARD_PWD_OFFSET_ADDRESS, &IDCardPwd_Offset, data_len);
	// erase IDCardPwd_Cache
	memset(IDCardPwd_Cache[0], 0xff, MAX_ID_CARD_PWD_NUM * ID_CARD_PWD_LEN);
	// read IDCardPwd_Cache
	data_len = MAX_ID_CARD_PWD_NUM * ID_CARD_PWD_LEN;
	
	bsp_flash_read(EEP_ID_CARD_PWD_START_ADDRESS, IDCardPwd_Cache[0], data_len);
	return (unsigned char **)IDCardPwd_Cache;
}
/*************************************************************************************
*@brief		save memdata
*@param		None
*@retval	None
**************************************************************************************/
void Save_MemsData(void)
{
	unsigned short NumByteToWrite;	// 写的字节数
	
	NumByteToWrite = sizeof(MEMSDataTypedef);
	//擦除
	bsp_flash_erase(EEP_MEMS_START_ADDRESS, NumByteToWrite);
    //写入数据
	bsp_flash_write(EEP_MEMS_START_ADDRESS,(unsigned char *)&memsData, NumByteToWrite);
	
	DEBUG("MEMS theshold %d, ADC Val:%d %d %d",memsData.theshold, memsData.static_ADCValue[0], memsData.static_ADCValue[1], memsData.static_ADCValue[2]);
}
/*************************************************************************************
*@brief		read memdata
*@param		None
*@retval	None
**************************************************************************************/
void Read_MemsData(void)
{
	unsigned short data_len;
	data_len = sizeof(MEMSDataTypedef);
	bsp_flash_read(EEP_MEMS_START_ADDRESS, (unsigned char *)&memsData, data_len);
	DEBUG("MEMS theshold %d, ADC Val:%d %d %d",memsData.theshold, memsData.static_ADCValue[0], memsData.static_ADCValue[1], memsData.static_ADCValue[2]);
}


