/******************************************************************************
*	@file		WIFIToCloud.c
*	@author
*	@version
*	@data		2019-09
*	@brief		WIFI模块的封装函数
*
******************************************************************************/


#include "WiFiToCloud.h"

 
/*******************************************************************
*	@brief		清空AT串口接收缓存相关寄存器
*	@param		None
*	@retval		None
*	@attention	
*******************************************************************/
void ClrAtRxBuf(void)
{
	memset(AT_RX_BUF, 0x00, MAX_AT_RX_LEN);//清空缓存

	AT_RX_STA =0x00000000;
}
/*******************************************************************
*	@brief		通过 USART4 发送AT指令字符串
*	@param		uint8_t *string 待发送的AT指令
*				uint8_t len 长度
*	@retval		None
*	@attention
*******************************************************************/
void SendAtCmd(uint8_t *string, uint8_t len)
{
	UART4_SendStrlen(string, len);
	UART4_SendStrlen((uint8_t *)"\r\n", 2);
}

/*******************************************************************
*	@brief		通过 USART4 发送给定长度字符串
*	@param		uint8_t *string 待发送的字符串
*				uint8_t len 长度
*	@retval		None
*	@attention
*******************************************************************/
void SendStrLen(uint8_t *string, uint8_t len)
{
	UART4_SendStrlen(string, len);
}



/*******************************************************************
*	@function 	int8_t ESP8266_SetStation(void)
*	@brief		ESP8266设置为station模式
*	@param 		None
*	@retval
*		@val	return = 0 ,sucess
*		@val	return < 0 ,error
*	@attention
*******************************************************************/
int8_t ESP8266_SetStation(void)
{
	ClrAtRxBuf();//清空缓存
	SendAtCmd((uint8_t *)AT_CWMODE,strlen(AT_CWMODE));
	delay_ms(100);
	if(strstr((const char *)AT_RX_BUF, (const char *)"OK") == NULL)
	{
		return -1;
	}
	ClrAtRxBuf();
	return 0;
}

/*******************************************************************
*	@function	int8_t ESP8266_SetAP(void)
*	@brief		设置ESP8266要连接的热点名称和密码
*	@param		char *wifi:热点名称 
*				char *pwd:热点密码
*	@retval 
		@val 	return = 0 ,sucess
		@val	return < 0 ,error
*	@attention
*******************************************************************/
int8_t ESP8266_SetAP(char *wifi, char *pwd)
{
	uint8_t AtCwjap[MAX_AT_TX_LEN];
	memset(AtCwjap, 0x00, MAX_AT_TX_LEN);//清空缓存
	ClrAtRxBuf();//清空缓存
	sprintf((char *)AtCwjap,"AT+CWJAP_CUR=\"%s\",\"%s\"",wifi, pwd);
//	printf("%s\r\n",AtCwjap);////////////////////////////////////////////////////////////
	SendAtCmd((uint8_t *)AtCwjap,strlen((const char *)AtCwjap));
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	if(strstr((const char *)AT_RX_BUF, (const char *)"OK") == NULL)
	{
		return -1;
	}
	ClrAtRxBuf();
	return 0;
}

/*******************************************************************
*	@function	int8_t ESP8266_SetStation(void)
*	@brief		ESP8266建立TCP连接
*	@param 		char *IpAddr-IP地址，例如：120.77.58.34
*				uint16_t port-端口号，取值0~65535
*	@retval
*		@val	return = 0 ,sucess
*		@val	return < 0 ,error
*	@attention
*******************************************************************/
int8_t ESP8266_IpStart(char *IpAddr, uint16_t port)
{
	uint8_t IpStart[MAX_AT_TX_LEN];
	memset(IpStart, 0x00, MAX_AT_TX_LEN);//清空缓存
	ClrAtRxBuf();//清空缓存
	sprintf((char *)IpStart,"AT+CIPSTART=\"TCP\",\"%s\",%d",IpAddr, port);
	//printf("%s\r\n",IpStart);////////////////////////////////////////////////////////////
	SendAtCmd((uint8_t *)IpStart,strlen((const char *)IpStart));
	delay_ms(1500);
	if(strstr((const char *)AT_RX_BUF, (const char *)"OK") == NULL)
	{
		return -1;
	}
	ClrAtRxBuf();
	return 0;
}

/*******************************************************************
*	@function	int8_t ESP8266_IpSend(char *IpBuf, uint8_t len)
*	@brief		ESP8266发送数据
*	@param		char *IpBuf-IP数据
*				uint8_t len-数据长度
*	@retval
*		@val	return = 0 ,sucess
*		@val	return < 0 ,error
*	@attention
*******************************************************************/
int8_t ESP8266_IpSend(char *IpBuf, uint8_t len)
{
	uint8_t TryGo = 0;
	int8_t error = 0;
	uint8_t IpSend[MAX_AT_TX_LEN];
	memset(IpSend, 0x00, MAX_AT_TX_LEN);//清空缓存
	ClrAtRxBuf();//清空缓存
	sprintf((char *)IpSend,"AT+CIPSEND=%d",len);
//	printf("%s\r\n",IpSend);////////////////////////////////////////////////////////////
	SendAtCmd((uint8_t *)IpSend,strlen((const char *)IpSend));
	delay_ms(3);
//	printf("nprintf = %s\r\n",AT_RX_BUF);
	if(strstr((const char *)AT_RX_BUF, (const char *)"OK") == NULL)
	{
		return -1;
	}
	ClrAtRxBuf();//清空缓存
	SendStrLen((uint8_t *)IpBuf, len);
//	printf("%s\r\n",IpBuf);////////////////////////////////////////////////////////////
	for(TryGo = 0; TryGo<60; TryGo++)//最多等待时间100*60=6000ms
	{
		if(strstr((const char *)AT_RX_BUF, (const char *)"SEND OK") == NULL)
		{
			error = -2;
		}
		else
		{
			error = 0;
			break;
		}
		delay_ms(100);
	}
//	ClrAtRxBuf();
	return error;
}

/*******************************************************************
*	@function	int8_t ConnectToServer(void)
*	@brief		连接到服务器
*	@param 		None
*	@retval
*		@val	return = 0 ,sucess
*		@val	return < 0 ,error
*	@attention
*******************************************************************/
int8_t ConnectToServer(char *DeviceID, char *SecretKey)
{
	uint8_t TryGo = 0;
	int8_t error = 0;
	uint8_t TxetBuf[MAX_AT_TX_LEN];
	memset(TxetBuf,0x00,MAX_AT_TX_LEN);//清空缓存
	for(TryGo = 0; TryGo<3; TryGo++)
	{
		if(ESP8266_SetStation() == 0)//设置WiFi通讯模块工作模式
		{
			error = 0;
			break;
		}
		else
		{
			error = -1;
		}
	}
	if(error < 0)
	{
		return error;
	}
	for(TryGo = 0; TryGo<3; TryGo++)
	{
		if(ESP8266_SetAP((char *)WIFI_AP, (char *)WIFI_PWD) == 0)//设置热点名称和密码
		{
			error = 0;
			break;
		}
		else
		{
			error = -2;
		}
	}
	if(error < 0)
	{
		return error;
	}
	for(TryGo = 0; TryGo<3; TryGo++)
	{
		if(ESP8266_IpStart((char *)SERVER_IP,SERVER_PORT) == 0)//连接服务器IP地址，端口：120.77.58.34,8600
		{
			error = 0;
			break;
		}
		else
		{
			error = -3;
		}
	}
	if(error < 0)
	{
		return error;
	}
	
	sprintf((char *)TxetBuf,"{\"t\":1,\"device\":\"%s\",\"key\":\"%s\",\"ver\":\"v0.0.0.0\"}",DeviceID,SecretKey);
	//printf("%s\r\n",TxetBuf);////////////////////////////////////////////////////////////
	if(ESP8266_IpSend((char *)TxetBuf, strlen((char *)TxetBuf)) < 0)
	{//发送失败
		error=-4;
	}
	else
	{//发送成功
		for(TryGo = 0; TryGo<50; TryGo++)//最多等待时间50*10=500ms
		{
			if(strstr((const char *)AT_RX_BUF, (const char *)"\"status\":0") == NULL)//检查响应状态是否为握手成功
			{
				error = -5;
			}
			else
			{
				error = 0;
				break;
			}
			delay_ms(10);
		}
	}
	return error;
}

/*******************************************************************
*	@function	int8_t ESP8266_SendSensor(char *SensorTag, char *TimeStr, uint8_t sensor)
*	@brief 		发送传感数据到服务器
*	@param		char *SensorTag	对象标签名称
*				uint8_t sensor	传感数值
*				char *TimeStr	采集数据时的时间，须是yyyy-mm-dd hh:mm:ss格式
*	@retval
*	   	@val	return = 0 ,sucess
*		@val	return < 0 ,error
*	@attention
*******************************************************************/
int8_t ESP8266_SendSensor(char *SensorTag, char *TimeStr, uint8_t sensor)
{
	uint8_t TryGo = 0;
	int8_t error = 0;
	uint8_t TxetBuf[MAX_AT_TX_LEN];
	memset(TxetBuf,0x00,MAX_AT_TX_LEN);//清空缓存
	sprintf((char *)TxetBuf,"{\"t\":3,\"datatype\":2,\"datas\":{\"%s\":{\"%s\":%d}},\"msgid\":001}",SensorTag, TimeStr, sensor);
	//printf("%s\r\n",TxetBuf);////////////////////////////////////////////////////////////
	if(ESP8266_IpSend((char *)TxetBuf, strlen((char *)TxetBuf)) < 0)
	{//发送失败
		error=-1;
	}
	else
	{//发送成功
		for(TryGo = 0; TryGo<50; TryGo++)//最多等待时间50*10=500ms
		{
			if(strstr((const char *)AT_RX_BUF, (const char *)"\"status\":0") == NULL)//检查响应状态是否为"上报成功"
			{
				error = -1;
			}
			else
			{
				error = 0;
				break;
			}
			delay_ms(10);
		}
		ClrAtRxBuf();
	}
	return error;
}

/*******************************************************************
*	@function	uint8_t ESP8266_GetIpData(uint8_t *AtRxBuf, char *GetIpData)
*	@brief		解析服务器数据
*	@param		uint8_t *AtRxBuf ，原始AT串口缓存
*		 		char *GetIpData ，截取出来的Itcp/ip数据
*	@retval 	返回收到的IP数据长度
*	@attention
*		AT+CIPSEND=76
*		{"t":3,"datatype":2,"datas":{"alarm":{"2018-06-19 18:15:02":0}},"msgid":001}
*		+IPD,29:{"msgid":1,"status":0,"t":4}
*******************************************************************/
uint8_t ESP8266_GetIpData(uint8_t *AtRxBuf, char *GetIpData)
{
	char *Point = NULL;
	uint8_t len = 0;
	
	Point = strstr((const char *)AtRxBuf, (const char *)"+IPD,");
	if(Point != NULL)//检查模块收到TCP/IP数据包？
	{
		len=(Point[5]-'0')*10+Point[6]-'0';//这里IP数据长度不会超过100，故可以按“个位”和“十位”计算收到的IP数据长度
		Point = strstr((const char *)AtRxBuf, (const char *)":")+1;
		memcpy(GetIpData, Point, len);
		GetIpData[len]='\0';
		printf("收到IP数据：%s\r\n",GetIpData);	
		printf("收到IP数据长度=%d\r\n",len);
	}
	
	return (len);
}


int8_t ESP8266_SendPing(void)
{
	int8_t error=0;
	uint8_t TryGo = 0;
	if(ESP8266_IpSend((char *)PING_REQ, strlen((const char *)PING_REQ)< 0))
	{
		//发送失败
		error=-1;
	}
	else
	{
		//发送成功
		for(TryGo = 0; TryGo<50; TryGo++)//最多等待时间50*10=500ms
		{
			if(strstr((const char *)AT_RX_BUF, (const char *)PING_RSP) == NULL)//检查响应状态是否为"上报成功"
			{
				error = -1;
			}
			else
			{
				error = 0;
				break;
			}
			delay_ms(10);
		}
		ClrAtRxBuf();
	}
	return error;
}




void ConnectToServer_times(void)
{
	int8_t temp=0;
	//连接服务器
	for(u8 TryCount=0; TryCount<3; TryCount++)
	{
		LCD_PRINT(0,0,lcd_show_buf,"Connect to sever");
		LCD_PRINT(1,0,lcd_show_buf,"Try %d times",TryCount);
		LCD_PRINT(2,0,lcd_show_buf,"... ...");
		temp=ConnectToServer((char *)MY_DEVICE_ID, (char *)MA_SECRET_KEY);
		if(temp != 0)
		{
			printf("Connect To Server ERROR=%d\r\n",temp);
		}
		else
		{
			printf("Connection successful!\r\n");
			device_sta.is_connect_to_server = ON;
			break; // end Server connected successfully
		}
	}
	if(temp!=0)
	{
		printf("Connection failed!\r\n");
		device_sta.is_connect_to_server = OFF;
	}
	lcd_clr();
}
	


/*******************************************************************
*	@function		void ESP8266_DataAnalysisProcess(char *RxBuf)
*	@brief			解析服务器数据
*	@param			char *RxBuf 服务器下发数据
*	@retval			None
*	@attention		用户可以在此基础上改造和扩展该函数，这里只是个简单的DEMO
*******************************************************************/
void ESP8266_DataAnalysisProcess(char *RxBuf)
{
	if(strstr((const char *)RxBuf, (const char *)PING_REQ) != NULL)//心跳请求？
	{
		ESP8266_IpSend((char *)PING_RSP, strlen((const char *)PING_RSP));//响应心跳
	}
	else if(strstr((const char *)RxBuf, (const char *)CMD_REQ_STR) != NULL)//命令请求？
	{
		if(strstr((const char *)RxBuf, (const char *)"\"apitag\":\"ctrl\"") != NULL)//开锁请求
		{
			if((strstr((const char *)RxBuf, (const char *)"\"data\":1") != NULL))//开锁
			{
				ESP8266_IpSend((char *)PING_RSP, strlen((const char *)PING_RSP));//响应心跳
			}
		}
		else if(strstr((const char *)RxBuf, (const char *)"\"apitag\":\"defense\"") != NULL)//布防/撤防请求
		{
			if((strstr((const char *)RxBuf, (const char *)"\"data\":1") != NULL))//布防
			{
				printf("布防！\r\n");
				;//...
				;//...
				;//...
			}
			else if((strstr((const char *)RxBuf, (const char *)"\"data\":0") != NULL))//撤防
			{
				printf("撤防！\r\n");
				;//...
				;//...
				;//...
			}
		}
	}
}

/******************************************* END OF FILE ********************************************/

