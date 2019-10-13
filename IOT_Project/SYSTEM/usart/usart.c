/********************************************************************
*	@file 		usart.h
*	@author		Panda
*	@version	V0.0.0
*	@date		2019-09
*********************************************************************/

#include "usart.h"	

//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif


//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
//重定义fgetc函数
int fgetc(FILE *f)
{
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
	{
	}
	return (int)USART_ReceiveData(USART1);
}

#endif

/*------------------------------ USART1 ------------------------------*/

#if USART1_RX_EN   //如果使能了接收
	
u8 USART1_RX_BUF[USART1_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.

/**
*接收状态
*		bit15，		接收完成标志
*		bit14，		接收到0x0d
*		bit13~0，	接收到的有效字节数目
*/
u16 USART1_RX_STA=0;       //接收状态标记	

#endif	/* USART1_RX_EN */

/**************************************************************
*@brief 	USART1 初始化
*@param 	bound:USART1 波特率
*@retval 	None
***************************************************************/
void USART1_Init(u32 bound)
{
   //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//使能USART1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	// 使能AFIO时钟
	
	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART1, &USART_InitStructure); //初始化串口1
	
	USART_Cmd(USART1, ENABLE);  //使能串口1 
	
	USART_ClearFlag(USART1, USART_FLAG_TC);
	
#if USART1_RX_EN	

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启相关中断

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

#endif
	
}


void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	u8 Res;
#if SYSTEM_SUPPORT_OS  //使用UCOS操作系统
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
//		USART_SendData(USART3,Res);
		if((USART1_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART1_RX_STA&0x4000)//接收到了0x0d
			{
				if(Res!=0x0A)
					USART1_RX_STA=0;//接收错误,重新开始
				else 
					USART1_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(Res==0x0D)
					USART1_RX_STA|=0x4000;
				else
				{
					USART1_RX_BUF[USART1_RX_STA&0X3FFF]=Res ;
					USART1_RX_STA++;
					if(USART1_RX_STA>(USART1_REC_LEN-1))	//接收数据错误,重新开始接收	 
						USART1_RX_STA=0; 
				}		 
			}
		}
  } 
 #if SYSTEM_SUPPORT_OS  
	OSIntExit();    	//退出中断
#endif
}


void USART1_DataAnalysis(void)
{
	u8 temp=0;
	u16 waittime=0;
	if(USART1_RX_STA & 0x8000)
	{
		if(strstr((const char*)USART1_RX_BUF,"unlock")!=NULL)
		{
			lock_ctrl(1);
		}
		else if(strstr((const char*)USART1_RX_BUF,"lock")!=NULL)
		{
			lock_ctrl(0);
		}
		else if(strstr((const char*)USART1_RX_BUF,"light on")!=NULL)
		{
			lamp_ctrl(1);
		}
		else if(strstr((const char*)USART1_RX_BUF,"light off")!=NULL)
		{
			lamp_ctrl(0);
		}
		else if(strstr((const char*)USART1_RX_BUF,"buzzer on")!=NULL)
		{
			buzzer_ctrl(1);
		}else if(strstr((const char*)USART1_RX_BUF,"buzzer off")!=NULL)
		{
			buzzer_ctrl(0);
		}
		else if(strstr((const char*)USART1_RX_BUF,"get id")!=NULL)
		{
			LF125K_Read_1_Page(data_buf);
			data_buf[8]= '\0' ;
			DEBUG("ID Card:%x,%x,%x,%x,%x,%x,%x,%x",data_buf[0],data_buf[1],data_buf[2],data_buf[3],data_buf[4],data_buf[5],data_buf[6],data_buf[7]);
		}
		else if(strstr((const char*)USART1_RX_BUF,"read page 0:0")!=NULL)
		{
			LF125K_Read_0_Page(0,data_buf);
			data_buf[4]='\0';
			DEBUG("addr:00 content:%x,%x,%x,%x",data_buf[0],data_buf[1],data_buf[2],data_buf[3]);
		}
		else if(strstr((const char*)USART1_RX_BUF,"read page 0:1")!=NULL)
		{
			LF125K_Read_0_Page(1,data_buf);
			data_buf[4]='\0';
			DEBUG("addr:01 content:%x,%x,%x,%x",data_buf[0],data_buf[1],data_buf[2],data_buf[3]);
		}
		else if(strstr((const char*)USART1_RX_BUF,"read page 0:2")!=NULL)
		{
			
			LF125K_Read_0_Page(2,data_buf);
			data_buf[4]='\0';
			DEBUG("addr:02 content:%x,%x,%x,%x",data_buf[0],data_buf[1],data_buf[2],data_buf[3]);
		}
		else if(strstr((const char*)USART1_RX_BUF,"read page 0:3")!=NULL)
		{
			LF125K_Read_0_Page(3,data_buf);
			data_buf[4]='\0';
			DEBUG("addr:03 content:%x,%x,%x,%x",data_buf[0],data_buf[1],data_buf[2],data_buf[3]);
		}
		else if(strstr((const char*)USART1_RX_BUF,"read page 0:4")!=NULL)
		{
			LF125K_Read_0_Page(4,data_buf);
			data_buf[4]='\0';
			DEBUG("addr:04 content:%x,%x,%x,%x",data_buf[0],data_buf[1],data_buf[2],data_buf[3]);
		}
		else if(strstr((const char*)USART1_RX_BUF,"read page 0:5")!=NULL)
		{
			LF125K_Read_0_Page(5,data_buf);
			data_buf[4]='\0';
			DEBUG("addr:05 content:%x,%x,%x,%x",data_buf[0],data_buf[1],data_buf[2],data_buf[3]);
		}
		else if(strstr((const char*)USART1_RX_BUF,"read page 0:6")!=NULL)
		{
			LF125K_Read_0_Page(6,data_buf);
			data_buf[4]='\0';
			DEBUG("addr:06 content:%x,%x,%x,%x",data_buf[0],data_buf[1],data_buf[2],data_buf[3]);
		}
		else if(strstr((const char*)USART1_RX_BUF,"read page 0:7")!=NULL)
		{
			LF125K_Read_0_Page(7,data_buf);
			data_buf[4]='\0';
			DEBUG("addr:07 content:%x,%x,%x,%x",data_buf[0],data_buf[1],data_buf[2],data_buf[3]);
		}
		else if(strstr((const char*)USART1_RX_BUF,"swipe lf")!=NULL)
		{
			waittime=1000;
			while(waittime--)
			{
				delay_ms(10);
				temp = LF125K_SwipeCard();
				if(temp == ID_CARD1)
				{
					door_open(500);
					break;
				}
				else if(temp ==ID_CARD3)
				{
					door_open(500);
					break;
				}
			}
			if(waittime==0)
				DEBUG("Swipe Card failed!");		
		}
		else if(strstr((const char*)USART1_RX_BUF,"add fingerprint")!=NULL)
		{
			printf("Add fingerprint\r\n");
			Add_FR();
		}
		else if(strstr((const char*)USART1_RX_BUF,"del fingerprint")!=NULL)
		{
			printf("Del fingerprint!\r\n");
			Del_FR();
		}
		else if(strstr((const char*)USART1_RX_BUF,"press fingerprint")!=NULL)
		{
			printf("Swiping fingerprint... ...\r\n");
			waittime = 1000;
			while(waittime--)
			{
				if(!press_FR())
				{
					printf("Swipe fingerprint Successfully!\r\n");
					break;
				}
			}
			if(waittime == 0)
				printf("Swipe fingerprint Out Time\r\n");
			
		}
		else if(strstr((const char*)USART1_RX_BUF,"up char")!=NULL)
		{
			printf("Uping char... ...\r\n");
			waittime = 1000;
			while(waittime--)
			{
				if(AS608_UpChar(CharBuffer1))
				{
					;
					break;
				}
			}
			if(waittime == 0)
				printf("Swipe fingerprint Out Time\r\n");
			
		}
//		printf("\r\n%s\r\n",USART1_RX_BUF);
		memset(USART1_RX_BUF,0x00,sizeof(USART1_RX_BUF));
		USART1_RX_STA = 0;
	}
}






/*------------------------------ USART2 ------------------------------*/

//串口接收缓存区 	
u8 USART2_RX_BUF[USART2_RECV_LEN]; 				//接收缓冲,最大USART2_MAX_RECV_LEN个字节.

u8 USART2_TX_BUF[USART2_SEND_LEN]; 			  //发送缓冲,最大USART2_MAX_SEND_LEN字节

//通过判断接收连续2个字符之间的时间差不大于10ms来决定是不是一次连续的数据.
//如果2个字符接收间隔超过10ms,则认为不是1次连续数据.也就是超过10ms没有接收到
//任何数据,则表示此次接收完毕.
//接收到的数据状态
//[15]:0,没有接收到数据;1,接收到了一批数据.
//[14:0]:接收到的数据长度
vu16 USART2_RX_STA=0;   	


void USART2_IRQHandler(void)
{
#if	SYSTEM_SUPPORT_OS
	OSIntEnter();
#endif
	uint8_t res;	  
	uint32_t temp;
//	if(!(USART2->CR1 & (0x01<<4)))
//		USART2->CR1 |= (0x01<<4);
	if(USART_GetITStatus(USART2,USART_IT_RXNE) != RESET)
	{
		res = USART_ReceiveData(USART2);
//		USART_SendData(USART1,res);
		if(!(USART2_RX_STA & 0x8000))
		{	
//			USART_SendData(USART1,res);
			USART2_RX_BUF[USART2_RX_STA++]=res;
		}
	}
	if(USART_GetITStatus(USART2,USART_IT_IDLE) != RESET)
	{
		temp=USART2->SR;
		temp=USART2->DR;
		USART2_RX_STA |= 0x8000;
	}
		

    
//	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//接收到数据
//	{	 
//		res =USART_ReceiveData(USART2);		 
//		if((USART2_RX_STA&(1<<15))==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
//		{ 
//			if(USART2_RX_STA<USART2_RECV_LEN)	//还可以接收数据
//			{
//				TIM_SetCounter(TIM7,0);//计数器清空          				//计数器清空
//				if(USART2_RX_STA==0) 				//使能定时器7的中断 
//				{
//					TIM_Cmd(TIM7,ENABLE);//使能定时器7
//				}
//				USART2_RX_BUF[USART2_RX_STA++]=res;	//记录接收到的值	 
//			}
//			else 
//			{
//				USART2_RX_STA|=1<<15;				//强制标记接收完成
//			} 
//		}
//	}  	
#if	SYSTEM_SUPPORT_OS
	OSIntExit();
#endif

}   


//初始化IO 串口2
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率	  
void USART2_Init(u32 bound)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //串口2时钟使能

 	USART_DeInit(USART2);  //复位串口2
	//USART2_TX   PA2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA2
   
    //USART2_RX	  PA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA3
	
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  
	USART_Init(USART2, &USART_InitStructure); //初始化串口2
  

	USART_Cmd(USART2, ENABLE);                    //使能串口 
	
	//使能接收中断
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断   
	USART_ITConfig(USART2,USART_IT_IDLE,ENABLE);
	
	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
//	TIM7_Int_Init(99,7199);		//10ms中断
	USART2_RX_STA=0;		//清零
//	TIM_Cmd(TIM7,DISABLE);			//关闭定时器7

}

//串口发送一个字节
void USART2_SendData(u8 data)
{
	while((USART2->SR&0X40)==0); 
	USART2->DR = data;
}

//串口2,printf 函数
//确保一次发送数据不超过USART2_MAX_SEND_LEN字节
void usart2_printf(char* fmt,...)  
{  
	u16 i,j; 
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)USART2_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART2_TX_BUF);		//此次发送数据的长度
	for(j=0;j<i;j++)							//循环发送数据
	{
	  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
		USART_SendData(USART2,USART2_TX_BUF[j]); 
	} 
}


/*------------------------------ USART3 ------------------------------*/
#if USART3_IT_EN   //如果使能了接收	

//u8  USART3_RX_BUF[USART3_RECV_LEN]; //接收缓冲,最大USART_REC_LEN个字节.
//u16 USART3_RX_STA=0;       //接收状态标记	  

void USART3_IRQHandler(void)
{
	uint8_t recv_char;
	uint32_t temp=0;
	
#if	SYSTEM_SUPPORT_OS
	OSIntEnter();
#endif	/* SYSTEM_SUPPORT_OS */

#if USART3_IT_RX_EN	
	if(USART_GetITStatus(USART3,USART_IT_RXNE) != RESET)  //接收到数据
	{
		recv_char=USART_ReceiveData(USART3);
//		USART_SendData(USART1,recv_char);
//		USART3_RX_BUF[USART3_RX_STA++]=recv_char;
//		if(USART3_RX_STA > USART3_RECV_LEN)
//			USART3_RX_STA = 0;
        Xfs_AckReceive(recv_char);
    }
#endif /* USART3_IT_RX_EN */
#if USART3_IT_IDLE_EN	
	if(USART_GetITStatus(USART3,USART_IT_IDLE) != RESET)
	{
		temp = USART3->SR;
		temp = USART3->DR;
	}
#endif /* USART3_IT_IDLE_EN */	
}
#if	SYSTEM_SUPPORT_OS
	OSIntEnter();
#endif	/* SYSTEM_SUPPORT_OS */

#endif	/* USART3_IT_EN */

//APB1为36Mhz
void USART3_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	// GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE); //串口2时钟使能

 	USART_DeInit(USART3);  //复位串口2
	//USART3_TX   PB10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 		//PB10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure); 	//初始化PB10
   
    //USART3_RX	  PB11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);  	//初始化PB11
	
	USART_DeInit(USART3);
	USART_InitStructure.USART_BaudRate = bound;		//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;			//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;			//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;		//收发模式
	USART_Init(USART3, &USART_InitStructure); 	//初始化串口3
  
	USART_Cmd(USART3, ENABLE);                    //使能串口 
	
#if USART3_IT_RX_EN
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); //使能接收中断
#endif /* USART3_IT_RX_EN */

#if USART3_IT_IDLE_EN
	USART_ITConfig(USART3,USART_IT_IDLE,ENABLE);	//使能空闲中断  
#endif /* USART3_IT_IDLE_EN */

#if USART3_IT_EN		
	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
#endif /* USART3_IT_EN */
}

/******************************************************************************
* Function Name --> 串口3发送一个字节数据
* Description   --> nonevoid USART3_SendData(uint16_t Data)
* Input         --> bound：波特率	
* Output        --> none
* Reaturn       --> none 
******************************************************************************/
void USART3_SendData(uint8_t Data)
{
	while((USART3->SR & 0x40) == 0);	//等待发送完毕
	USART3->DR = Data;      	//写DR,串口3将发送数据
}


void USART3_SendStrlen(uint8_t *Data, int len)
{
	while(len--)
	{
		USART3_SendData((uint16_t )(*Data));
		Data++;
	}
}
	








/*------------------------------ USART4 ------------------------------*/

/******************************************************************************
* @brief	初始化IO 串口4
* @param	bound：波特率	
* @retval	None
******************************************************************************/
void UART4_Init(uint32_t bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
#ifdef UART4_RX_EN	//如果使能了接收中断
	
	UART4->CR1 |= 1 << 8;	//PE中断使能
	UART4->CR1 |= 1 << 5;	//接收缓冲区非空中断使能
	
	MY_NVIC_Init(0, 1, UART4_IRQn, NVIC_PriorityGroup_2);	//中断分组2
#endif

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);	//使能UART4，GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	USART_DeInit(UART4);  //复位串口4
	
	//UART4_TX   PC.10 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure); //初始化PC10
   
	//UART4_RX	  PC.11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOC, &GPIO_InitStructure);  //初始化PC11

	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;	//设置波特率，一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;	//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	
	USART_Init(UART4, &USART_InitStructure); //初始化串口
	
	USART_ClearITPendingBit(UART4,USART_IT_RXNE);
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//开启接收中断
	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);//开启空闲中断
	USART_Cmd(UART4, ENABLE);                    //使能串口 
}

/******************************************************************************
* @brief 	串口4发送一个字节数据
* @param 	data：要发送的数据	
* @retval 	None
******************************************************************************/
void UART4_SendData(uint16_t Data)
{
	while((UART4->SR & 0x40) == 0);	//等待发送完毕
	USART_SendData(UART4, Data);
}

/******************************************************************************
* @brief 	串口4发送一串数据
* @param	*Data: 要发送的字符串	
* @retval	None
******************************************************************************/
void UART4_SendStr(uint8_t *Data)
{
	while(*Data!='\0')
	{
		UART4_SendData((uint16_t )(*Data));
		Data++;
	}
}

/******************************************************************************
* @brief	串口4发送一串数据，指定数据长度
* @param	*Data:字符串，len长度	
* @retval	None 
******************************************************************************/
void UART4_SendStrlen(uint8_t *Data, uint8_t len)
{
	while(len--)
	{
		UART4_SendData((uint16_t )(*Data));
		Data++;
	}
}


 #ifdef UART4_RX_EN   //如果使能了接收

//串口4中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误 

uint8_t UART4_RX_BUF[UART4_REC_MAX_LEN];	//接收缓冲,最大UART4_REC_LEN个字节
uint32_t UART4_RX_STA = 0;	
uint8_t  IS_UART4_IN_RX = 0 ;
/******************************************************************************
* @brief	串口4接收中断服务程序
* @param	None
* @retval	None
*******************************************************************************/
void UART4_IRQHandler(void)
{
#if	SYSTEM_SUPPORT_OS
	OSIntEnter();
#endif	/* SYSTEM_SUPPORT_OS */
	uint32_t temp=0;
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)  //接收到数据
	{
		IS_UART4_IN_RX = 1;
		UART4_RX_BUF[UART4_RX_STA++] = USART_ReceiveData(UART4);	//读取接收到的数据
//		USART_SendData(USART1,UART4_RX_BUF[UART4_RX_STA-1]);
		if(UART4_RX_STA>UART4_REC_MAX_LEN) //缓存区溢出
		{
			UART4_RX_STA = 0 ;
		}
//		printf("%s\r\n",UART4_RX_BUF);
		//end 接收未完成   	
	}	//end 接收到数据
	if(USART_GetITStatus(UART4,USART_IT_IDLE)  != RESET)
	{
		temp = UART4->SR;
		temp = UART4->DR;
		IS_UART4_IN_RX = 0;
	}
	
	
	
#if	SYSTEM_SUPPORT_OS
	OSIntExit();
#endif	/* SYSTEM_SUPPORT_OS */	
}

#endif	/* UART4_RX_EN */

//=========================================================


/*------------------------------ UART5 ------------------------------*/

//串口接收缓存区 	
u8 UART5_RX_BUF[UART5_MAX_RECV_LEN]; 				//接收缓冲,最大UART5_MAX_RECV_LEN个字节.
u8 UART5_TX_BUF[UART5_MAX_SEND_LEN]; 			  //发送缓冲,最大UART5_MAX_SEND_LEN字节

/****************************************************************************************
*	通过判断接收连续2个字符之间的时间差不大于10ms来决定是不是一次连续的数据.
*	如果2个字符接收间隔超过10ms,则认为不是1次连续数据.也就是超过10ms没有接收到
*	任何数据,则表示此次接收完毕.
*	接收到的数据状态
*	[15]:0,没有接收到数据;1,接收到了一批数据.
*	[14:0]:接收到的数据长度
*****************************************************************************************/
vu16 UART5_RX_STA=0;   	


void UART5_IRQHandler(void)
{
#if	SYSTEM_SUPPORT_OS
	OSIntEnter();
#endif	/* SYSTEM_SUPPORT_OS */
	u8 res;	      
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)//接收到数据
	{	 
		res =USART_ReceiveData(UART5);		 
		if((UART5_RX_STA&(1<<15))==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
		{ 
			if(UART5_RX_STA<UART5_MAX_RECV_LEN)	//还可以接收数据
			{
				TIM_SetCounter(TIM7,0);//计数器清空          				//计数器清空
				if(UART5_RX_STA==0) 				//使能定时器7的中断 
				{
					TIM_Cmd(TIM7,ENABLE);//使能定时器7
				}
				UART5_RX_BUF[UART5_RX_STA++]=res;	//记录接收到的值	 
			}
			else 
			{
				UART5_RX_STA|=1<<15;				//强制标记接收完成
			} 
		}
	}  		
#if	SYSTEM_SUPPORT_OS
	OSIntExit();
#endif	/* SYSTEM_SUPPORT_OS */	
}   

/**********************************************************
*@brief		初始化IO 串口2
*@param		bound:波特率	  
*@retval	None
***********************************************************/
void UART5_Init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD, ENABLE);	// GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5,ENABLE); //串口2时钟使能

 	USART_DeInit(UART5);  //复位串口2
	//UART5_TX   PC12
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //PC12
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure); //初始化PC12
	
	//UART5_RX	  PD2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOD, &GPIO_InitStructure);  //初始化PA3
	
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  
	USART_Init(UART5, &USART_InitStructure); //初始化串口2
  

	USART_Cmd(UART5, ENABLE);                    //使能串口 
	
	//使能接收中断
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//开启中断   
	
	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	
	TIM7_Int_Init(99,7199);		//10ms中断
	UART5_RX_STA=0;		//清零
	TIM_Cmd(TIM7,DISABLE);			//关闭定时器7

}

//串口2,printf 函数
//确保一次发送数据不超过UART5_MAX_SEND_LEN字节
void uart5_printf(char* fmt,...)  
{  
	u16 i,j; 
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)UART5_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)UART5_TX_BUF);		//此次发送数据的长度
	for(j=0;j<i;j++)							//循环发送数据
	{
	  while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
		USART_SendData(UART5,UART5_TX_BUF[j]); 
	} 
}





/********************************************END OF FILE**************************************************/
