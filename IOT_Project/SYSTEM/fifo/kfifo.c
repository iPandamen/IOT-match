#include "kfifo.h"
//#include "debug_printf.h"
#define KFIFO_MIN(a,b) ((a)<(b)?(a):(b))
#define KFIFO_MAX(a,b) ((a)>(b)?(a):(b))


static uint32_t calc_real_fifo_len(uint16_t len)
{
    uint16_t i = 0;
    
    for(i=0; len>0; i++)
    {
        len = len >> 1;
    }
    return (0x1<<(i-1));
}



int32_t kfifo_init(char* name,kfifo_t* pfifo,uint8_t* pbuf,uint32_t size)
{
    pfifo->fifo_name    = name;
    pfifo->p_buf        = pbuf;
    pfifo->fifo_size    = calc_real_fifo_len(size);  // 2^(size) - 1  
    pfifo->write_index  = 0;
    pfifo->read_index   = 0;
    //pfifo->lock         = 0;
    return 1;
}



void kfifo_reset(kfifo_t* pfifo)
{
	pfifo->write_index  = 0;
    pfifo->read_index   = 0;
	memset(pfifo->p_buf,0,pfifo->fifo_size);
//	DBG_B_INFO("kfifo reset");
}


/*
      avaiable                   avaiable
    |*******|-----------------|************|
       |     \___             |        |
      PART_B     |            |      PART_A
                 |            |
                 |             \
               read_index       write_index
*/
uint32_t kfifo_push_in(kfifo_t* pfifo,uint8_t* pbuf, uint32_t len)
{   
	unsigned int l;   
//    DBG_B_INFO("len:%ld",len);
	len = KFIFO_MIN(len, pfifo->fifo_size - pfifo->write_index + pfifo->read_index);   
	SYS_ENTRY_CRITICAL   
	
//    DBG_B_INFO("len:%ld  kfifo_rest_len:%ld ",len,pfifo->fifo_size - pfifo->write_index + pfifo->read_index);
	l = KFIFO_MIN(len, pfifo->fifo_size - (pfifo->write_index & (pfifo->fifo_size - 1)));   

//	DBG_B_INFO("l:%d  kfifo_get_w_index:%ld ",l, pfifo->write_index & (pfifo->fifo_size - 1));
  
	memcpy(pfifo->p_buf + (pfifo->write_index & (pfifo->fifo_size - 1)), pbuf, l);   
	
	memcpy(pfifo->p_buf, pbuf + l, len - l);   

    pfifo->write_index += len;  

    SYS_EXIT_CRITICAL  
    
    return len;   
}  
/*
      avaiable                   avaiable
    |*******|-----------------|************|
       |     \___             |        |
      PART_B     |            |      PART_A
                 |            |
                 |             \
               write_index       read_index
*/ 
uint32_t kfifo_pull_out(kfifo_t* pfifo,uint8_t* pbuf, uint32_t len)
{   
    unsigned int l;   
    
    if(pbuf!=NULL)
    {
		len = KFIFO_MIN(len, pfifo->write_index - pfifo->read_index);   
		
		SYS_ENTRY_CRITICAL   
		
		l = KFIFO_MIN(len, pfifo->fifo_size - (pfifo->read_index & (pfifo->fifo_size - 1)));   
		
		memcpy(pbuf, pfifo->p_buf + (pfifo->read_index & (pfifo->fifo_size - 1)), l);   
		
		memcpy(pbuf + l, pfifo->p_buf, len - l);   
    }

    pfifo->read_index += len;   
  
    SYS_EXIT_CRITICAL 

    return len;   
}   

uint32_t kfifo_have_len(kfifo_t* pfifo)
{
    return (pfifo->write_index - pfifo->read_index);
}

uint32_t kfifo_rest_len(kfifo_t* pfifo)
{
	return pfifo->fifo_size - kfifo_have_len(pfifo);
}

uint32_t kfifo_get_w_index(kfifo_t* pfifo)
{
    return (pfifo->write_index & (pfifo->fifo_size - 1));
}

uint32_t kfifo_get_r_index(kfifo_t* pfifo)
{
    return (pfifo->read_index & (pfifo->fifo_size - 1));
}



void kfifo_test_demo(void)
{
	uint8_t out_buf[20]={0};
	uint8_t test_fifo_buffer[100]={0};
	kfifo_t test_fifo={"test fifo",10*10,test_fifo_buffer,0,0};
		
	printf("name: %s\r\n",test_fifo.fifo_name);
	printf("size: %d\r\n",test_fifo.fifo_size);
	printf("buf:%s\r\n",test_fifo.p_buf);
	printf("read_index: %d\r\n",test_fifo.read_index);
	printf("write_index: %d\r\n",test_fifo.write_index);
	printf("have len: %d\r\n",kfifo_have_len(&test_fifo));
	printf("\r\n");
	
	kfifo_push_in(&test_fifo,(uint8_t*)"abcdef",6);
	printf("buf: %s\r\n",test_fifo.p_buf);
	printf("read_index: %d\r\n",test_fifo.read_index);
	printf("write_index: %d\r\n",test_fifo.write_index);
	printf("have len: %d\r\n",kfifo_have_len(&test_fifo));
	printf("\r\n");
	
	
	kfifo_pull_out(&test_fifo,out_buf,3);
	printf("buf: %s\r\n",test_fifo.p_buf);
	printf("read_index: %d\r\n",test_fifo.read_index);
	printf("write_index: %d\r\n",test_fifo.write_index);
	printf("have len: %d\r\n",kfifo_have_len(&test_fifo));
	printf("\r\n");
	
}





