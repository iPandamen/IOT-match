
#include "queuing_machine.h"

#define CARD_ID(id)	"卡号为: %02X %02X %02X %02X %02X %02X %02X %02X",id[0],id[1],id[2],id[3],id[4],id[5],id[6],id[7]

#define CARD_STR_LEN	8	
#define CARD_STR_SUM	64	
uint8_t number_fifo_buffer[CARD_STR_LEN * CARD_STR_SUM]="Hello World";
kfifo_t number_fifo={"number fifo",CARD_STR_LEN * CARD_STR_SUM,number_fifo_buffer,0,0};


#define FLASH_SAVE_START_ADDR 0x08070000
#define ID_CARD_OFFSET_ADDR	0x08070000
#define ID_CARD_ADDR	ID_CARD_OFFSET_ADDR+4
uint8_t id_card_offset=0;
uint8_t id_card[CARD_STR_SUM][CARD_STR_LEN]={0};

uint8_t number_up = 0;
uint8_t number_down = 0;

uint8_t broadcast_buf[100]={0};

uint8_t swipe_flag = 0;

static void DataAnalysis_TCP(char *RxBuf)
{
	char *cmdid = NULL;
	char *data = NULL;
	double value;
	uint8_t TxetBuf[128];
	if(strstr((const char *)RxBuf, (const char *)PING_REQ) != NULL)//心跳请求？
	{
		if(ESP8266_IpSend((char *)PING_RSP, strlen((const char *)PING_RSP)) < 0)//响应心跳
		{
			//发送失败
			printf("发送心跳包失败！\r\n");
		}
		else
		{
			printf("心跳包！\r\n");
		}
	}
	else if(strstr((const char *)RxBuf, (const char *)"\"t\":5") != NULL)//命令请求？
	{
		if(strstr((const char *)RxBuf, (const char *)"\"apitag\":\"number_down\"") != NULL)//开/关 灯请求
		{
			memset(TxetBuf,0x00,128);//清空缓存
			
			data = json_getvalue(RxBuf,"data");
			remove_both_end(data);
			printf("data: %s\r\n",data);
			value = data_type_conversion_S2N(data);
			printf("value: %f\r\n",value);
			;//...
			;//...
			;//...
//			cmdid = GetJsonValue((char *)RxBuf, (char *)"cmdid");
//			sprintf((char *)TxetBuf,"{\"t\":6,\"cmdid\":%s,\"status\":0,\"data\":0}",cmdid);
////			printf("%s\r\n",TxetBuf);
//			if(ESP8266_IpSend((char *)TxetBuf, strlen((char *)TxetBuf)) < 0)
//			{
//				//发送失败
//				printf("发送响应失败！\r\n");
//			}
			
		}
		if(strstr((const char *)RxBuf, (const char *)"\"apitag\":\"string_play\"") != NULL)//开/关 灯请求
		{
			memset(TxetBuf,0x00,128);//清空缓存
			data = json_getvalue(RxBuf,"data");
			remove_both_end(data);
			printf("data:%s\r\n",data);
			Xfs_VoiceBroadcastStart((unsigned char*)data,XFS_FORMAT_GB2312,1);
			while(Xfs_GetStatus() != XFS_ACK_IDEL);

			
		}
		if(strstr((const char *)RxBuf, (const char *)"\"apitag\":\"bool_work\"") != NULL)// 开始/暂停刷卡
		{
			memset(TxetBuf,0x00,128);//清空缓存
			if((strstr((const char *)RxBuf, (const char *)"\"data\":1") != NULL))// 开始取号
			{		
				printf("开始取号！\r\n");
				Xfs_VoiceBroadcastStart((unsigned char*)"开始取号",XFS_FORMAT_GB2312,1);
				while(Xfs_GetStatus() != XFS_ACK_IDEL);
				swipe_flag = 1;	
				printf("swipe_flag: %d\r\n",swipe_flag);
				;//...
				;//...
				;//...

			}
			else if((strstr((const char *)RxBuf, (const char *)"\"data\":0") != NULL))// 暂停取号
			{
				printf("暂停取号！\r\n");
				Xfs_VoiceBroadcastStart((unsigned char*)"暂停取号",XFS_FORMAT_GB2312,1);
				while(Xfs_GetStatus() != XFS_ACK_IDEL);
				swipe_flag = 0;	
				printf("swipe_flag: %d\r\n",swipe_flag);
	
			}
		}
	}
}






void Q_SwipeCard(void)
{
	int8_t temp;
	
	Xfs_VoiceBroadcastStart((unsigned char*)"刷卡成功",XFS_FORMAT_GBK,1);
	while(Xfs_GetStatus() != XFS_ACK_IDEL);
	
	swipe_flag = 0;
	
	kfifo_push_in(&number_fifo,lf_page_1_buf,CARD_STR_LEN);
	number_up = number_fifo.write_index / 8;
	number_down = number_fifo.read_index / 8;
	printf("number_up: %d\r\n",number_up);
	printf("number_down: %d\r\n",number_down);
	
	sprintf((char*)broadcast_buf,CARD_ID(lf_page_1_buf));
	printf("%s\r\n",broadcast_buf);
	Xfs_VoiceBroadcastStart((unsigned char*)broadcast_buf,XFS_FORMAT_GBK,1);
	while(Xfs_GetStatus() != XFS_ACK_IDEL);
	
	sprintf((char*)broadcast_buf,"你的号码为：%d",number_up);
	Xfs_VoiceBroadcastStart((unsigned char*)broadcast_buf,XFS_FORMAT_GBK,1);
	while(Xfs_GetStatus() != XFS_ACK_IDEL);
	
	// 数据添加
	temp = app_add_data(ID_CARD_ADDR,id_card[0],lf_page_1_buf,8,(uint32_t*)&id_card_offset,CARD_STR_SUM);
	if(temp == 0)
	{
		Xfs_VoiceBroadcastStart((unsigned char*)"存储成功",XFS_FORMAT_GBK,1);
		while(Xfs_GetStatus() != XFS_ACK_IDEL);
	}			
}

void Q_Call_number(void)
{
	uint8_t read_buf[8]={0};
	
	kfifo_pull_out(&number_fifo,read_buf,8);
	number_up = number_fifo.write_index / 8;
	number_down = number_fifo.read_index / 8;
	printf("number_up: %d\r\n",number_up);
	printf("number_down: %d\r\n",number_down);
	
	sprintf((char *)broadcast_buf,"请 %d 号来到窗口",number_down);
	Xfs_VoiceBroadcastStart((unsigned char*)broadcast_buf,XFS_FORMAT_GBK,1);
	while(Xfs_GetStatus() != XFS_ACK_IDEL);
}




void Queue_machine_init(void)
{
	STM_Clock_Init(9);	// 系统时钟初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	SYSTICK_Init(); 
	
	USART1_Init(115200);
	
	UART4_Init(115200);
	
	RTC_Init();
	
	Xfs5152ce_Init();
	
//	Xfs_VoiceBroadcastStart((unsigned char *)"欢迎使用智能排队机", XFS_FORMAT_GBK, 1);
//	while(Xfs_GetStatus() != XFS_ACK_IDEL);
	
	lcd_init();
	lcd_clr();
	
	LF125K_Init();
	
	M3_key_init();
	buzzer_init();
	
	app_write_data(ID_CARD_OFFSET_ADDR,(uint8_t*)"\0",1,1);
	app_read_data(ID_CARD_OFFSET_ADDR,&id_card_offset,1,1);
//	printf("id_card_offset: %d\r\n",id_card_offset);
	app_param_init(ID_CARD_ADDR,id_card[0],CARD_STR_LEN,CARD_STR_SUM);
	
	NixieTube_Init();
	
//	SI4332A_Init();
//	SI4332B_Init();
	
}

void Queue_main_task(void)
{
	uint32_t adr = 0;
	int16_t i=0,sel=0;
	int8_t temp=0;
	uint16_t cmdid=0;
	uint8_t read_buf[50];
	uint8_t broadcast_buf[50]={0};
	
	Data_ReportStruct test[]={{"number_up",1,number_up},{"number_down",1,number_down}};
	
	//连接服务器
	ConnectToServer_times();
	if(device_sta.is_connect_to_server == ON)
	{
		Xfs_VoiceBroadcastStart((unsigned char *)"连接成功", XFS_FORMAT_GBK, 1);
		while(Xfs_GetStatus() != XFS_ACK_IDEL);
	}
	lcd_clr();
//	app_lib_test_dome();
//	kfifo_test_demo();
	
	while(1)
	{
		i++;
		m3_key_sta = M3_key_scan(0); 
		if(m3_key_sta == KEY1_PRESS)
		{
			swipe_flag = 1;
			Xfs_VoiceBroadcastStart((unsigned char*)"请刷卡",XFS_FORMAT_GBK,1);
			while(Xfs_GetStatus() != XFS_ACK_IDEL);
			DEBUG("KEY1 Press");
		}
		else if(m3_key_sta == KEY2_PRESS)
		{
			Q_Call_number();
		}
		
		if(swipe_flag == 1)
		{
			temp = LF125K_Get_CardInfo(PAGE_1);
			LF125K_Show_CardInfo(PAGE_1);
			DEBUG("temp= %d\r\n",temp);
			if(temp == 0)
			{	
//				adr = (uint32_t)app_find_data(ID_CARD_ADDR,lf_page_1_buf,CARD_STR_LEN,CARD_STR_SUM);
//				if(adr)
//				{	
//					sprintf((char*)broadcast_buf,"你已经完成取号,你的号码为 %d,请不要重复刷卡",(adr-ID_CARD_ADDR)/8);
//					Xfs_VoiceBroadcastStart((unsigned char*)broadcast_buf,XFS_FORMAT_GBK,1);
//					while(Xfs_GetStatus() != XFS_ACK_IDEL);
//				}
//				else
					Q_SwipeCard();
//				swipe_flag = 0;
			}	
		}
		NixieTube_Show_Integer(number_down,1,1);
		
		delay_ms(10);
		if(i%200 == 0)
		{
//			if(cmdid % 2 == 0)
//				Data_Report_TCP_Number(2,"number_up",(char*)current_time_buf,number_up,cmdid++);
//			else
//				Data_Report_TCP_Number(2,"number_down",(char*)current_time_buf,number_down,cmdid++);
			
			Data_Report_TCP(test,2,cmdid++);
			
//			if( ESP8266_SendPing() != 0)
//				device_sta.is_connect_to_server = OFF;
//			printf("status: %d\r\n",device_sta.is_connect_to_server);
			printf("write_index: %d\r\n",number_fifo.write_index);
			printf("read_index: %d\r\n",number_fifo.read_index);
			printf("number_up: %d\r\n",number_up);
			printf("number_down: %d\r\n",number_down);
		}
		
//		if(device_sta.is_connect_to_server == OFF)
//			ConnectToServer_times();
		
		number_up = number_fifo.write_index / 8;
		number_down = number_fifo.read_index / 8;
		
		memset(data_buf,0x00,sizeof(data_buf));
		if(IS_UART4_IN_RX)
			while(IS_UART4_IN_RX);
		ESP8266_GetIpData(AT_RX_BUF,(char*)data_buf); 	// 从 ESP8266 获取数据
		ClrAtRxBuf();
		DataAnalysis_TCP((char*)data_buf);
		
	}
}





