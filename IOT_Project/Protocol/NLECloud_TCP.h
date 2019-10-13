/**********************************************************
*	@file 		NLECloud_TCP.h
*	@author		Panda
*	@version	V0.0.0
*	@date		2019-09
***********************************************************/
#ifndef _NLECLOUD_TCP_H
#define	_NLECLOUD_TCP_H

#include "Init.h"
#include "WiFiToCloud.h"

#include "NLECloud_TCP.h"
#include "lock.h"

typedef struct
{
	char *apitag;
	char up_data_type;	// 上传的数据类型				
	union
	{
		int integer;
		double float_num;
		char string[10];
	}pdata;				// 上传的数据
	
}Data_ReportStruct;




#define	CONN_REQ_STR 		("\"t\":1")
#define CONN_RESP_STR		("\"t\":2")
#define PUSH_DATA_STR		("\"t\":3")
#define PUSH_ACK_STR		("\"t\":4")
#define CMD_REQ_STR			("\"t\":5")
#define CMD_RESP_STR		("\"t\":6")
#define PING_REQ_STR		("\"t\":7")
#define PING_RESP_STR		("\"t\":8")


uint8_t Data_Report_TCP_Number(uint8_t datatype,char *apitag,char *datetime,uint32_t value,uint8_t msgid);

uint8_t Data_Report_TCP_String(uint8_t datatype,char *apitag,char *datetime,uint8_t *pstr,uint8_t msgid);

uint8_t Data_Report_TCP_String(uint8_t datatype,char *apitag,char *datetime,uint8_t *pstr,uint8_t msgid);

int8_t Data_Report_TCP(Data_ReportStruct *pStruct,int data_num,int msgid);
	
void NLECloud_DataAnalysis_TCP(char *RxBuf);

void NewLand_DataAnalysis_USART(unsigned char* buf);

char *GetJsonValue(char *cJson, char *Tag);

char* json_getvalue(char *json_str,char *tag);

double data_type_conversion_S2N(char* str);

uint8_t remove_both_end(char *str);

#endif	//	_NLECLOUD_TCP_H

/*************************END OF FILE**********************/
