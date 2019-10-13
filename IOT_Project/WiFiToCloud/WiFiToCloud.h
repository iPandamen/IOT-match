/******************************************************************************
*	@file		WIFIToCloud.h
*	@author
*	@version
*	@date		2019-09
*	@brief		
*
******************************************************************************/

#ifndef _WIFITOCLOUD_H
#define _WIFITOCLOUD_H

/* includes------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdint.h>

#include "Init.h"
#include "delay.h"

#include "CloudReference.h"

/* defines------------------------------------------------------------------------*/
//宏定义
#define AT_CWMODE	"AT+CWMODE_CUR=1" //设置为“station”模式

#define PING_REQ "$#AT#\r"		//心跳请求
#define PING_RSP "$OK##\r"		//心跳响应

#define MAX_AT_RX_LEN	255			//最大接收长度
#define MAX_AT_TX_LEN	128			//最大发送长度
#define AT_RX_BUF 		UART4_RX_BUF		//AT串口接收缓存，它是一个数组
#define AT_RX_STA		UART4_RX_STA	



/* function------------------------------------------------------------------------*/
/*******************************************************************
*函数：void ClrAtRxBuf(void)
*功能：清空AT串口接收缓存相关寄存器
*输入：无
*输出：无
*特殊说明：无
*******************************************************************/
void ClrAtRxBuf(void);

/*******************************************************************
*函数：void SendAtCmd(uint8_t *string)
*功能：发送AT指令字符串
*输入：
		uint8_t *string 待发送的AT指令
		uint8_t len 长度
*输出：无
*特殊说明：无
*******************************************************************/
void SendAtCmd(uint8_t *string, uint8_t len);

/*******************************************************************
*函数：void SendStrLen(uint8_t *string, uint8_t len)
*功能：发送字符串到AT指令串口
*输入：
		uint8_t *string 待发送的字符串
		uint8_t len 长度

*输出：无
*特殊说明：无
*******************************************************************/
void SendStrLen(uint8_t *string, uint8_t len);


/*******************************************************************
*函数：int8_t ESP8266_SetStation(void)
*功能：ESP8266设置为station模式
*输入：无
*输出：
		return = 0 ,sucess
		return < 0 ,error
*特殊说明：
*******************************************************************/
int8_t ESP8266_SetStation(void);

/*******************************************************************
*函数：int8_t ESP8266_SetAP(void)
*功能：设置ESP8266要连接的热点名称和密码
*输入：char *wifi-热点名称 char *pwd-热点密码
*输出：
		return = 0 ,sucess
		return < 0 ,error
*特殊说明：
*******************************************************************/
int8_t ESP8266_SetAP(char *wifi, char *pwd);

/*******************************************************************
*函数：int8_t ESP8266_SetStation(void)
*功能：ESP8266建立TCP连接
*输入：
	char *IpAddr-IP地址，例如：120.77.58.34
	uint16_t port-端口号，取值0~65535
*输出：
		return = 0 ,sucess
		return < 0 ,error
*特殊说明：
*******************************************************************/
int8_t ESP8266_IpStart(char *IpAddr, uint16_t port);

/*******************************************************************
*函数：int8_t ESP8266_IpSend(char *IpBuf, uint8_t len)
*功能：ESP8266发送数据
*输入：
		char *IpBuf-IP数据
		uint8_t len-数据长度
*输出：
		return = 0 ,sucess
		return < 0 ,error
*特殊说明：
*******************************************************************/
int8_t ESP8266_IpSend(char *IpBuf, uint8_t len);

/*******************************************************************
*函数：int8_t ConnectToServer(void)
*功能：连接到服务器
*输入：无
*输出：
		return = 0 ,sucess
		return < 0 ,error
*特殊说明：
*******************************************************************/
int8_t ConnectToServer(char *DeviceID, char *SecretKey);

/*******************************************************************
*函数：int8_t ESP8266_SendSensor(char *SensorTag, char *TimeStr, uint8_t sensor)
*功能：发送传感数据到服务器
*输入：
	char *SensorTag, 对象标签名称
	uint8_t sensor, 传感数值
	char *TimeStr,采集数据时的时间，须是yyyy-mm-dd hh:mm:ss格式
*输出：
		return = 0 ,sucess
		return < 0 ,error
*特殊说明：
*******************************************************************/
int8_t ESP8266_SendSensor(char *SensorTag, char *TimeStr, uint8_t sensor);

/*******************************************************************
*函数：uint8_t ESP8266_GetIpData(uint8_t *AtRxBuf, char *GetIpData)
*功能：解析服务器数据
*输入：
		uint8_t *AtRxBuf ，原始AT串口缓存
		char *GetIpData ，截取出来的Itcp/ip数据
*输出：返回收到的IP数据长度
*特殊说明：
	AT+CIPSEND=76
	{"t":3,"datatype":2,"datas":{"alarm":{"2018-06-19 18:15:02":0}},"msgid":001}
	+IPD,29:{"msgid":1,"status":0,"t":4}
*******************************************************************/
uint8_t ESP8266_GetIpData(uint8_t *AtRxBuf, char *GetIpData);
	
/*******************************************************************
*函数：void ESP8266_DataAnalysisProcess(char *RxBuf)
*功能：解析服务器数据
*输入：char *RxBuf 服务器下发数据
*输出：
*特殊说明：
*******************************************************************/
void ESP8266_DataAnalysisProcess(char *RxBuf);

int8_t ESP8266_SendPing(void);

void ConnectToServer_times(void);

#endif /* _WIFITOCLOUD_H */

/******************************************* END OF FILE ********************************************/


