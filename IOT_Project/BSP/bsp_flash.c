
/******************************************************************************
  * @file    bsp/bsp_flash.c 
  * @author  
  * @version V1.0.0
  * @date    2018.06.18
  * @brief   flash板级管理包
******************************************************************************/
#include "bsp_flash.h"


FLASH_Status FLASHStatus = FLASH_COMPLETE;


/********************************************************************
*@breif		将数据写入flash
*@param		addr: 地址
*			pbuf: 写入的数据
*			len : 写入数据的长度
*@retva		成功: 返回写入数据的长度
*********************************************************************/
int32_t bsp_flash_write(uint32_t addr,uint8_t* pbuf,int32_t len)
{
    uint32_t DATA_32 = 0;
    uint32_t i = 0;
    FLASH_Unlock();	// 解锁
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 

    while (i < len)
    {
        if(len>(i+4)) 
		{ 
			memcpy((void*)&DATA_32,(void*)&pbuf[i],sizeof(uint32_t)); 
		}
        else
		{ 
			memcpy((void*)&DATA_32,(void*)&pbuf[i],(len-i) ); 
		}
        if (FLASH_ProgramWord(addr+i, DATA_32) == FLASH_COMPLETE)
        {
			i+=4;
        }
        else
        { 
          /* Error occurred while writing data in Flash memory. User can add here some code to deal with this error */
            DEBUG("bsp_flash_write error:0x%x ",addr+i);
			return 0;
//            while (1);
        }
    }

    /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    FLASH_Lock(); 
    return len;
}

/*****************************************************************************************************************
*@brief		添加新的数据，并写入 flash 中
*@param		addr: 写入的地址
*			pdata: 原始数据
*			pnew: 新的数据
*			len: 该类型数据的长度
*			num：原始数据中数据的个数
*			sum: 该类数据最大的个数
*@retval	success: 返回写入的长度
*			failed：返回0
******************************************************************************************************************/
int32_t bsp_flash_write_num(uint32_t addr,uint8_t *pdata,uint8_t *pnew,int32_t len,int32_t *pnum,int32_t sum)
{
	int32_t num = *pnum;
	int32_t *pdest = (int32_t *)(pdata+num*len);
	int32_t ret=0;
	
	if( num >= sum)
		num = 0;
	
	memset(pdest,0xFF,len);
	memcpy(pdest,pnew,len);
	num++;
	
	bsp_flash_erase(addr,len*sum);
	ret = bsp_flash_write(addr,pdata,len*sum);
	
	*pnum = num;
	return ret;
}



int32_t bsp_flash_read (uint32_t addr,uint8_t* p_dest,int32_t len)
{
    memcpy((void*)p_dest, (void *)addr, len);
    return len;
}

/**********************************************************************************************
*@brief		读取 flash 中某一个的数据
*@param		addr: 读取数据的地址
*			pdata: 读取的数据
*			len: 读取数据的长度
*			num: flash 中第 num 个数据
*			sum：flash中 连续地址中 该类型数据的个数
*@retval	success：返回读取的 Byte 数
*			failed： 返回0
***********************************************************************************************/
int32_t bsp_flash_read_num(uint32_t addr,uint8_t *pdata,int32_t len,int32_t num,int32_t sum)
{
	uint32_t source = 0;
	int32_t ret = 0;
	if(num > sum)
	{
		printf("Please enter the true value!\r\n");
		return NULL;
	}
	source = addr + len *(num - 1);
	ret = bsp_flash_read(source,pdata,len);
	return ret;
}	


int32_t bsp_flash_erase	(uint32_t address,uint32_t len)
{
    uint32_t EraseCounter = 0;
    uint32_t NbrOfPage = 0;
    
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
    
    NbrOfPage = (len) / MCU_FLASH_PAGE_SIZE;
    if(len&(MCU_FLASH_PAGE_SIZE-1))	/*如果字节数大于当前块数*/
	{
		NbrOfPage++;
	}
    for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
    {
        if (FLASH_ErasePage(address + (MCU_FLASH_PAGE_SIZE * EraseCounter))!= FLASH_COMPLETE)
        {
         /* Error occurred while sector erase. User can add here some code to deal with this error  */
			DEBUG("bsp_flash_erase error : address:0x%x ",address + (MCU_FLASH_PAGE_SIZE * EraseCounter));
			return 1;
//            while (1);
        }
    }
    FLASH_Lock(); 
    return 0;
}

/*******************************************************************************
*@brief		在flash中寻找 数据
*@param		addr:开始地址
*			pdata:寻找的数据
*			len: 寻找数据的长度
*			sum: falsh 中该类型数据的个数
*@retval	成功：返回数据地址
*			失败：返回空
********************************************************************************/
uint32_t *bsp_flash_find(uint32_t addr,uint8_t *pdata,int32_t len,int32_t sum)
{
	uint8_t i = 0;
	uint32_t dest = addr; 
	for(i=0;i<sum;i++)
	{
		if(memcmp(pdata,(const uint8_t*)dest,len) == 0)
			break;
		dest += len;
	}
	if(i < sum)
	{
		return (uint32_t*)(dest);
	}
	else
		return NULL;
}



void bsp_flash_test_demo(void)
{
	uint8_t id_card[20] = {0};
	uint8_t buf[10][2] = {"ab","cd","ef","gh"};
	uint8_t num=4;
	
	bsp_flash_erase(0X08070000,8);
	bsp_flash_write(0X08070000,(uint8_t*)"abcdefgh",8);
	printf("addr： %#X\r\n",bsp_flash_find(0X08070000,"ab",2,4));
	printf("addr： %#X\r\n",bsp_flash_find(0X08070000,"cd",2,4));
	printf("addr： %#X\r\n",bsp_flash_find(0X08070000,"ef",2,4));
	printf("addr： %#X\r\n",bsp_flash_find(0X08070000,"gh",2,4));
	
	bsp_flash_read_num(0X08070000,id_card,2,1,4);
	printf("id card: %s\r\n",id_card);
	
	bsp_flash_write_num(0X08070000,(uint8_t *)buf[0],(uint8_t*)"jk",2,(int32_t *)&num,10);
	printf("id card: %s\r\n",buf[0]);
}






#define FLASH_SAVE_ADDR 0X08070000 
uint8_t test_buffer[]={"Stm32f103 bsp_flash"};


/**********************
//设置 FLASH 保存地址(必须为偶数，且其值要大于本代码所占用 FLASH 的大小+0X08000000)
//本项目 的MCU     falsh 总大小有 512k  
//address 0x08000000
//address 0x08005c00 已用到  23k
//address 0x08070000 开辟的save_addr 地址
//address 0x08080000 end save_addr 地址
***********************/
int32_t demo_flash(void)
{
//	uint8_t read_buffer[sizeof(test_buffer)];
	uint16_t i;
	uint8_t test[1024];
	uint8_t test_read[1024];
    bsp_flash_erase(FLASH_SAVE_ADDR, 1);
    bsp_flash_write(FLASH_SAVE_ADDR,test_buffer,sizeof(test_buffer));
//    bsp_flash_read(FLASH_SAVE_ADDR,read_buffer,sizeof(test_buffer));
//    if(strcmp((const char*)test_buffer,(const char*)read_buffer) == 0)
//    {
//	   printf("二者相等 %.*s\n", sizeof(test_buffer), read_buffer); 
//	   return -1;
//    }
//	else
//	{
//		printf("二者不相等%.*s\n", sizeof(test_buffer), read_buffer); 
//		return 0;
//	}
//	for (i = 1; i < 33; ++i)
//	{
//	  bsp_flash_erase(FLASH_SAVE_ADDR+(2024*i),i);
//      bsp_flash_write(FLASH_SAVE_ADDR+(2024*i),&i,1);
//	}
//	for (i = 1; i < 33; ++i)
//	{
//	  bsp_flash_read(FLASH_SAVE_ADDR+(2024*i),&test,1024);
//	  
//	  printf("%d=[%d]\r\n", i,test); 
//	}
  for (i = 0; i < 1024; ++i)
  {
	test[i]=i;
  }
	bsp_flash_erase(FLASH_SAVE_ADDR, 1);
	bsp_flash_write(FLASH_SAVE_ADDR,test,1024);
	bsp_flash_read(FLASH_SAVE_ADDR,test_read,1024);
  for (i = 0; i < 1024; ++i)
  {
  	printf("%d=[%d]\r\n", i,test_read[i]); 
  }
}
