/*****************************************
*@file		app_lib.h
*@author	Panda
*@version	V0.0.0
*@date		2019-10
*@brief		app 使用的一些函数
******************************************/
#ifndef _APP_LIB_H
#define _APP_LIB_H

#include "Init.h"
#include "bsp_flash.h"



int32_t app_param_init(uint32_t address,uint8_t *pdata,uint32_t len,uint32_t sum);

int32_t app_write_data(uint32_t address,uint8_t *pdata,uint32_t len,uint32_t sum);

int32_t app_read_data(uint32_t address,uint8_t *pdata,uint32_t len,uint32_t sum);

uint32_t *app_find_data(uint32_t address,uint8_t *pdata,uint32_t len,uint32_t sum);

int32_t app_erase_data(uint32_t address,uint32_t len,uint32_t sum);

int32_t app_add_data(uint32_t address,uint8_t *pdata,uint8_t *pnew,uint32_t len,uint32_t *pnum,uint32_t sum);




void app_lib_test_dome(void);


#endif	/* _APP_LIB_H */










