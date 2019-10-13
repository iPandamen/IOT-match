/*****************************************
*@file		app_lib.c
*@author	Panda
*@version	V0.0.0
*@date		2019-10
*@brief		app 使用的一些函数
******************************************/
#include "app_lib.h"

/*******************************************************************************
*@breief		参数初始化
*@param			address: 初始数据的地址
*				pdata:	数据
*				len: 该类型数据的长度
*				sum: 该类型数据的个数
*@retval		success：
*				failed：0
********************************************************************************/
int32_t app_param_init(uint32_t address,uint8_t *pdata,uint32_t len,uint32_t sum)
{
	uint8_t i=0;
	uint32_t ret = 0;
	for(i=0;i<sum;i++)
		memset(pdata+i,0xFF,len);
	
	bsp_flash_erase(address,len*sum);
	ret = bsp_flash_write(address,pdata,len*sum);
	return ret;
}

/*******************************************************************************
*@breief		写数据
*@param			address: 写入的地址
*				pdata:	写入的数据
*				len: 该类型数据的长度
*				sum: 该类型数据的个数
*@retval		success：
*				failed：0
********************************************************************************/
int32_t app_write_data(uint32_t address,uint8_t *pdata,uint32_t len,uint32_t sum)
{
	int32_t ret = 0;
	bsp_flash_erase(address,len * sum);
	ret = bsp_flash_write(address,pdata,len*sum);
	return ret;
}


/*******************************************************************************
*@breief		读数据
*@param			address: 读取的地址
*				pdata:	读取的数据
*				len: 该类型数据的长度
*				sum: 该类型数据的个数
*@retval		success：
*				
********************************************************************************/
int32_t app_read_data(uint32_t address,uint8_t *pdata,uint32_t len,uint32_t sum)
{
	int32_t ret=0;
	ret = bsp_flash_read(address,pdata,len*sum);
	return ret;
}

/*******************************************************************************
*@brief		寻找数据
*@param		addr: 开始地址
*			pdata:寻找的数据
*			len:  寻找数据的长度
*			sum:  该类型数据的个数
*@retval	成功：返回数据地址
*			失败：返回空
********************************************************************************/
uint32_t *app_find_data(uint32_t address,uint8_t *pdata,uint32_t len,uint32_t sum)
{
	uint32_t *ret=0;
	ret = bsp_flash_find(address,pdata,len,sum);
	return ret;
}

/*******************************************************************************
*@breief		擦除数据
*@param			address: 擦除的地址
*				len: 该类型数据的长度
*				sum: 该类型数据的个数
*@retval		success:
*				failed:
********************************************************************************/
int32_t app_erase_data(uint32_t address,uint32_t len,uint32_t sum)
{
	int32_t ret=0;
	ret = bsp_flash_erase(address,len*sum);
	return ret;
}

/*******************************************************************************
*@breief		添加数据
*@param			address: 存储该数据的基地址地址
*				pdata:	添加的数据
*				len: 该类型数据的长度
*				*pnum: 当前该数据
*				sum: 该类型数据的个数
*@retval		success:
*				failed:
********************************************************************************/
int32_t app_add_data(uint32_t address,uint8_t *pdata,uint8_t *pnew,uint32_t len,uint32_t *pnum,uint32_t sum)
{
	int32_t ret = 0;
	uint32_t *pdest = NULL;
	uint32_t num = *pnum;
	if(num > sum)
		num = 0;
	
	pdest = (uint32_t*)(pdata + num * len);
	memset(pdest,0xFF,len);
	memcpy(pdest,pnew,len);
	num++;
	
	ret = app_erase_data(address,len,sum);
	if(ret != 0)
	{
		DEBUG("Flash erase error");
		ret = -1;
		return ret;
	}
	ret = app_write_data(address,pdata,len,sum);
	if(ret == 0)
	{
		DEBUG("Flash write error!");
		ret = -2;
		return ret;
	}
	*pnum = num;
	return ret; 
}




#define TEST_ADDR 0x08070000

void app_lib_test_dome(void)
{
	uint8_t num = 4;
	
	uint8_t w_buf[10][2]={"ab","cd","ef","gh"};
	uint8_t r_buf[10][2];
	
	
	printf("before: %s\r\n",w_buf[0]);
	app_write_data(0x08070000,w_buf[0],2,10); 
	
	app_read_data(0x08070000,r_buf[0],2,10);
	printf("after: %s\r\n",r_buf[0]);
	
	app_add_data(0x08070000,r_buf[0],(uint8_t*)"cc",2,(uint32_t *)&num,10);
	printf("after: %s\r\n",r_buf[0]);
	printf("num: %d\r\n",num);
//	

}