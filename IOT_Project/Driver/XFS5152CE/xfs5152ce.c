/***********************************************************
*@file		xfs5152ce.c
*@author
*@verdion
*@date		2019-09
*@brief
************************************************************/

#include "xfs5152ce.h"

//全局变量，装载语音芯片的返回值
unsigned char xfs_data_buf[80];

//控制语音芯片的接收帧
unsigned char xfs_state = 0;
volatile unsigned char xfs_receive_ok = 0;	// 完成标志
unsigned char xfs_protocol_state = 0;
unsigned char xfs_protocol_dat_len = 0;		// 数据长度
unsigned char xfs_protocol_dat_flag = 0;	// 数据类型标志
unsigned char xfs_protocol_dat_cnt = 0;		// 接收数据计数

//控制接收时间，防止接收突然中断，程序进入死循环
unsigned char xfs_timer_flag1 = 0;      //接收命令的计时标志
unsigned char xfs_timer_flag2 = 0;      //接收数据的计时标志
unsigned char xfs_timeout_cnt = 0;
unsigned char xfs_time_uplimit = 0;

//存储编码数据，大概10s左右
unsigned char xfs_encode_buf[30000];

//调用colloc函数需要
void _ttywrch(int ch)
{
    ch = ch;
}

//xfs5152的命令枚举
typedef enum 
{
	XFS_CMD_STATUS = 0x21,			//查询状态
	XFS_CMD_SLEEP = 0x88,			//进入低功耗模式
	XFS_CMD_WAKE_UP = 0xFF,			//从低功耗返回正常模式
	XFS_CMD_VOICE_START = 0x01,		//开始语音合成
	XFS_CMD_VOICE_STOP = 0x02,		//停止语音合成
	XFS_CMD_VOICE_PAUSE = 0x03,		//暂停语音合成
	XFS_CMD_VOICE_RESTORE = 0x04,	//恢复语音合成
	XFS_CMD_SEND_TO_CACHE = 0x31,	//发送文本到缓存区
	XFS_CMD_PLAY_FROM_CACHE = 0x32,	//从缓存区播放文本
	XFS_CMD_RECOGNIZE_START = 0x10,	//开始语言识别
	XFS_CMD_RECOGNIZE_STOP = 0x1F,	//停止语言识别
    XFS_CMD_ENCODE_START = 0x41,	//开始编码
    XFS_CMD_CODE_START = 0x42,		//开始解码
    XFS_CMD_ENCODEDATA_SEND = 0x43,	//发送编码数据到芯片
    XFS_CMD_EN_CODE_STOP = 0x44		//停止编码/解码
}XFS_CommandEnum;

//xfs5152ce的编解码采样率枚举
typedef enum 
{
	XFS_SAMPRATE_8K = 0,
	XFS_SAMPRATE_16K = 1
}XFS_SampRate;

//xfs5152编解码的比特率等级，比特率等级越大，声音越清晰
typedef enum 
{
	XFS_BAUD_LEVEL_0 = 0x00,
	XFS_BAUD_LEVEL_1 = 0x01,
	XFS_BAUD_LEVEL_2 = 0x02,
	XFS_BAUD_LEVEL_3 = 0x03,
	XFS_BAUD_LEVEL_4 = 0x04,
	XFS_BAUD_LEVEL_5 = 0x05,
}XFS_BaudRate;

//xfs5152声音编码音量等级
typedef enum 
{
	XFS_VOLUME_LEVEL_0 = 0x00,
	XFS_VOLUME_LEVEL_1 = 0x01,
	XFS_VOLUME_LEVEL_2 = 0x02,
	XFS_VOLUME_LEVEL_3 = 0x03,
	XFS_VOLUME_LEVEL_4 = 0x04,
	XFS_VOLUME_LEVEL_5 = 0x05,
	XFS_VOLUME_LEVEL_6 = 0x06,
	XFS_VOLUME_LEVEL_7 = 0x07,
	XFS_VOLUME_AUTO = 0x08,
}XFS_VolumeLevel;

//xfs5152响应延时
#define XFS_RESPONSE_TIMEOUT 100

// 语音识别命令词 
const char *cmd_word_list[] = 
{
	"我在吃饭",     	//0
	"我在修车",     	//1
	"我在加油",     	//2
	"正在休息",     	//3
	"同意",         	//4
	"不同意",         	//5
	"我去",           	//6
	"现在几点",        	//7 
	"今天几号",        	//8 
	"读信息",          	//9
	"开始读",          	//10
	"这是哪儿",        	//11 
	"打开广播",        	//12
	"关掉广播",        	//13
	"打开音乐",        	//14
	"关掉音乐",        	//15
	"再听一次",        	//16
	"再读一遍",        	//17
	"大声点",          	//18
	"小声点",          	//19
	"读短信",          	//20
	"读预警信息",      	//21 
	"明天天气怎样",    	//22 
	"紧急预警信息",    	//23 
	"开始",            	//24
	"停止",            	//25
	"暂停",            	//26
	"继续读",          	//27 
	"确定开始",        	//28
	"取消"             	//29
};

/**************************************************
*@brief		xfs5152端口初始化
*@param		None
*@retval	None
*@attenton
***************************************************/
void Xfs5152ce_Init(void)
{
    USART3_Init(115200);
}

/*******************************************************************
*@breef		等待XFS5152CE响应
*@param		uint8_t result：等待响应的命令 
* 			uint16_t waittime ：最长等待时间，单位ms
*@retval	1: success
*			0:
*@attention
*******************************************************************/
unsigned char Xfs_WaitResponse(uint8_t result, uint16_t waittime)
{
    while (--waittime)
	{
		delay_ms(1);
        if(xfs_receive_ok)
        {
            xfs_receive_ok = 0;
            if(result == xfs_state)
            {
                xfs_state = 0;
                return 1;
            }
        }
    }
	return 0;
}

/*********************************************************
*@brief		发送命令
*@param		cmd:要发送的指令码
*@retval	Xfs5152ce 的响应
**********************************************************/
unsigned char Xfs_SendCmd(uint8_t cmd)
{
	unsigned  char frame_info[8]; //定义的文本长度
	uint16_t frame_len = 0;
	 
/*****************帧固定配置信息**************************************/           
	frame_info[frame_len++] = 0xFD ; 	//构造帧头FD
	frame_info[frame_len++] = 0x00 ; 	//构造数据区长度的高字节
	frame_info[frame_len++] = 0x01; 	//构造数据区长度的低字节
	frame_info[frame_len++] = cmd ; 	//构造命令字 
 
/*******************发送帧信息***************************************/
    
    USART3_SendStrlen(frame_info,frame_len);
	return Xfs_WaitResponse(XFS_ACK_CMD_RIGHT, XFS_RESPONSE_TIMEOUT);
}


/*******************************************************************
*@brief		获得XFS5152CE的当前状态
*@param		None
*@retval	返回XFS5152CE的状态码
*@attention
*******************************************************************/
unsigned char Xfs_GetStatus(void)
{
	unsigned char rec_buf[8];
	unsigned  char frame_info[8]; //定义的文本长度
	uint16_t frame_len = 0;
	int timeout = XFS_RESPONSE_TIMEOUT;
	
	memset(rec_buf, 0, sizeof(rec_buf));
	
	Xfs_SendCmd(XFS_CMD_STATUS);		//构造命令字 
	
	while ((!xfs_receive_ok) && (timeout-- > 0))
		delay_ms(1);
    xfs_receive_ok = 0;
	return xfs_state;
}

/*******************************************************************
*@brief		使XFS5152CE进入睡眠状态
*@param		None
*@retval	1:
*			0:
*@attention
*******************************************************************/
unsigned char Xfs_Sleep(void)
{
	return Xfs_SendCmd(XFS_CMD_SLEEP);
}

/*******************************************************************
*@brief		唤醒XFS5152CE
*@param		None
*@retval	1:
*			0:
*@attention
*			唤醒后需等待20ms后再进行发指令，使其工作
*******************************************************************/
unsigned char Xfs_WakeUp(void)
{
	return Xfs_SendCmd(XFS_CMD_WAKE_UP);
}

/*******************************************************************
*@brief:	开始语音播报
*@param:	unsigned char *data： 需要播放的语言文本
*        	unsigned char format：语音文本的编码格式
*@retval	-1：参数错误
*         	1：语言开始播放
*         	0：语音播放失败
*@attentino
*******************************************************************/
signed char Xfs_VoiceBroadcastStart(unsigned char *data, unsigned char format, int wait_finish)
{
   	unsigned  char *frame_info;
	unsigned  int  data_length, frame_offset=0;  
	int ack = 0;
	
	if(data == NULL)
		return -1;
	if(format > XFS_FORMAT_UNICODE)
		return -1;

	//if(wait_finish)
	//	while(xfs_get_status() != XFS_ACK_IDEL)
	//		delay_ms(10);
		
	data_length =strlen((char*)data); 			//需要发送文本的长度
	frame_info = (unsigned char *)calloc((unsigned int)data_length, sizeof(unsigned char));
	if(frame_info == NULL)
		return -1;

/*****************帧固定配置信息**************************************/           
	frame_info[frame_offset++] = 0xFD ; 			//构造帧头FD
	frame_info[frame_offset++] = (data_length>>8) & 0xFF; 			//构造数据区长度的高字节
	frame_info[frame_offset++] = (data_length & 0xFF) + 2; 		//构造数据区长度的低字节
	frame_info[frame_offset++] = XFS_CMD_VOICE_START ; 			//构造命令字：合成播放命令		 		 
	frame_info[frame_offset++] = format;       //文本编码格式：GBK 
 
/*******************发送帧信息***************************************/		  
	memcpy(&frame_info[5], data, data_length);
	frame_offset += data_length;
    USART3_SendStrlen(frame_info,frame_offset);
    //while(!F_USART3_RX_FINISH);
	ack = Xfs_WaitResponse(0x41,10);
	if(frame_info)
		free(frame_info);
	
	return ack; 
}

/*******************************************************************
*@brief		停止语音播报
*@param		None
*@retval	1：停止成功
*			0：停止失败
*@attention
*******************************************************************/
unsigned char Xfs_VoiceBroadcastStop(void)
{
	return Xfs_SendCmd(XFS_CMD_VOICE_STOP);
}

/*******************************************************************
*@brief		暂停语音播报
*@param		None
*@retval	1：暂停成功，
*			0：暂停失败
*@attention
*******************************************************************/
unsigned char Xfs_VoiceBroadcastPause(void)
{
	return Xfs_SendCmd(XFS_CMD_VOICE_PAUSE);
}

/*******************************************************************
*@brief		继续语音播报
*@param		None
*@retval	1：继续成功，
*			0：继续失败
*@attention
*******************************************************************/
unsigned char Xfs_VoiceBroadcastRestore(void)
{
	return Xfs_SendCmd(XFS_CMD_VOICE_RESTORE);
}

/*******************************************************************
*@brief		发送文本到缓存区
*@param		data：文本数据
*@param		cache_num (0~15)
*@retval	1：成功
*@retval	0：失败
*@attention	缓存空间总共4K,共分16区,每个区256字节
*			设本次起始缓存区段=X(0<=X<=15),则本次发送的文本长度不能大于(16-X)*256字节，多余的文本将丢弃
*			用户在播放缓存文本前，可多次发送缓存命令，但切记后来发送的文本不能部分覆盖或全部覆盖之前的文本，
*			否则不保证播放的正确性
*******************************************************************/
signed char Xfs_SendTextToCache(unsigned char *data, unsigned char cache_num)
{
	unsigned char *frame_info = NULL; //定义的文本长度
	unsigned short data_length, frame_offset = 0;  
	unsigned char ack = 0;
	
	if(data == NULL)
		return -1;
	if(cache_num >= 16)
		return -1;
	data_length = strlen((char*)data); 			//需要发送文本的长度
	if(data_length > (16-cache_num)*256)
		return -1;
	frame_info = (unsigned char *)calloc((unsigned int)data_length, sizeof(unsigned char));
	if(frame_info == NULL)
		return -1;
/*****************帧固定配置信息**************************************/           
	frame_info[frame_offset++] = 0xFD ; 			//构造帧头FD
	frame_info[frame_offset++] = (data_length>>8) & 0xFF; 			//构造数据区长度的高字节
	frame_info[frame_offset++] = (data_length & 0xFF) + 2; 		//构造数据区长度的低字节
	frame_info[frame_offset++] = XFS_CMD_SEND_TO_CACHE ; 			//构造命令字	 
	frame_info[frame_offset++] = cache_num;       //文本编码格式：GBK 
 
/*******************发送帧信息***************************************/		  
	memcpy(&frame_info[frame_offset], data, data_length);
	frame_offset += data_length;
	USART3_SendStrlen(frame_info,frame_offset);//发送帧配置
	
	ack = Xfs_WaitResponse(XFS_ACK_CMD_RIGHT, XFS_RESPONSE_TIMEOUT);
	
	if(frame_info)
		free(frame_info);
	return ack;

}

/*******************************************************************
*@brief		播放缓存内容
*@param		num(1~15)
*			format:0-GB2312,1-GBK,2-BIG,3-UNICODE
*@retval	1：成功
*			0：失败
*@attention
*******************************************************************/
unsigned char Xfs_PalyFromCache(unsigned char num, unsigned char format)
{
	unsigned  char frame_info[8];
	unsigned short frame_len = 0;
	
	if(format > XFS_FORMAT_UNICODE || num > 15)
	return 0;

/*****************帧固定配置信息**************************************/           
	frame_info[frame_len++] = 0xFD ; 		//构造帧头FD
	frame_info[frame_len++] = 0x00 ; 		//构造数据区长度的高字节
	frame_info[frame_len++] = 0x02; 		//构造数据区长度的低字节
	frame_info[frame_len++] = XFS_CMD_PLAY_FROM_CACHE ; 			//构造命令字 
 	frame_info[frame_len++] = (num << 4 & 0xF0) | (format & 0x0F); 			//参数：高四位-重复播放次数，低四位-文本编码格式

/*******************发送帧信息***************************************/
	USART3_SendStrlen(frame_info, frame_len);
    
	return Xfs_WaitResponse(XFS_ACK_CMD_RIGHT, XFS_RESPONSE_TIMEOUT);

}

/*******************************************************************
*@brief		语音识别
*@param		result[out]: 识别结果
*			waittime[in]: 超时时间
*@retval	>0：成功
*			0：失败
*******************************************************************/
unsigned char Xfs_VoiceRecognize(unsigned char *result, signed int waittime)
{
	int err = 1;
	unsigned char n = 0;
	if(result == NULL)
	{
		err = 0;
		return err;
	}
    beep(100);
	printf("recognize start...\r\n");
	while(Xfs_SendCmd(XFS_CMD_RECOGNIZE_START) == 0)
	{
		err = 0;
		printf("Error: XFS_CMD_RECOGNIZE_START,%d\r\n",++n);
		Xfs_SendCmd(XFS_CMD_RECOGNIZE_STOP);
        delay_ms(100);
	}
	
	while(--waittime)
	{
		delay_ms(1);
		if(xfs_receive_ok)
        {
            xfs_receive_ok = 0;
			break;
        }
	}
	
//	while(xfs_receive_ok != 0)
//	{
//		if(waittime--)
//		{
//			
//		}
//	}
//	xfs_receive_ok = 0;
	
	printf("xfs_data_buf:");
	printf_hex(xfs_data_buf,20);
	if(waittime > 0)
	{
		switch (xfs_data_buf[3])
		{
			case 0x01:
				err = 0x01;
				*result = xfs_data_buf[4];
				printf("the result:%s\r\n",cmd_word_list[*result]);
				printf("语音结果：%d\r\n",xfs_data_buf[4]);
				break;
			case 0x02:
				err = 0x02;
				printf("Error: Time out\r\n");
				break;
			case 0x03:
				err = 0x03;
				printf("Error: Refuse\r\n");
				break;
			case 0x04:
				err = 0x04;
				printf("Error: Internal error\r\n");
				break;					
			default:
				err = 0;
				printf("Error: Unknow cmd\r\n");
				break;
		}
	}
	if(waittime == 0)
    {
		err = 0;
        printf("Error: Please speek the words in the time\r\n");
    }
	
	//stop receive
	Xfs_SendCmd(XFS_CMD_RECOGNIZE_STOP);
	return err;
}

/*******************************************************************
*@brief		开始语音编码
*@param		samp: 采样率
*			baud: 比特率
*           volum：音量
*@retval	>0：成功
*			0：失败
*			<0：参数错误
*******************************************************************/
signed char Xfs_VoiceEncodeStart(unsigned char samp,unsigned char baud,unsigned char volum)
{
    unsigned  char frame_info[7];
	unsigned short frame_len = 0;
    unsigned int data_cnt = 0;
    unsigned char datlen = 0;
    if(samp > 1)
        return -1;
    if(baud > 5)
        return -1;
    if(volum > 8)
        return -1;
/*****************帧固定配置信息**************************************/           
	frame_info[frame_len++] = 0xFD ; 			//构造帧头FD
	frame_info[frame_len++] = 0x00 ; 			//构造数据区长度的高字节
	frame_info[frame_len++] = 0x04; 		//构造数据区长度的低字节
    frame_info[frame_len++] = XFS_CMD_ENCODE_START ;      //编码
    
 	frame_info[frame_len++] = samp;             //参数1:设置采样率
    frame_info[frame_len++] = baud;             //参数2:设置比特率
    frame_info[frame_len++] = volum;            //参数3:设置音量

/*******************发送帧信息***************************************/
	USART3_SendStrlen(frame_info, frame_len);
    if(Xfs_WaitResponse(XFS_ACK_CMD_RIGHT, XFS_RESPONSE_TIMEOUT))
    {
        
        for(data_cnt = 0; data_cnt < 29000;data_cnt+=datlen)
        {
            while(!xfs_receive_ok);
            xfs_receive_ok = 0;
            if(xfs_data_buf[0] == 0xFC && xfs_data_buf[3] == 0x20){
                datlen = (((int)xfs_data_buf[1] << 8) | (xfs_data_buf[2])) + 4;
                memcpy(&xfs_encode_buf[data_cnt],xfs_data_buf,datlen);
            }
        }
        Xfs_VoiceCODECStop();
        if(!(xfs_data_buf[0] == 0xFC && xfs_data_buf[3] == 0x22))
            return 0;
        
        xfs_timer_flag1 = 0;
        xfs_timer_flag2 = 0;
        xfs_timeout_cnt = 0;
        xfs_protocol_state = 0;
        xfs_protocol_dat_len = 0;
        xfs_protocol_dat_flag = 0;
        delay_ms(1000);
        printf("Code Start...\r\n");
        Xfs_VoiceDecodeStart(0,3,7,xfs_encode_buf);
        return 1;
    }
	else
        return 0;
        
}

/*******************************************************************
*@brief		语音编码传输
*@param		*dat：带有枕头的编码数据
*@retval	>0：成功
*			0：失败
*******************************************************************/
signed char Xfs_VoiceCodeTransmit(u8 *dat)
{
    u8 datlen = 0;
    unsigned int cnt = 0;
    dat[cnt] = 0xFD;
    dat[cnt+3] = 0x43;
    datlen = dat[cnt + 1] << 8 | dat[cnt + 2] + 4;
    USART3_SendStrlen(&dat[cnt],datlen);
    for(cnt = datlen;cnt < 29000;cnt += datlen){
    
        while(xfs_receive_ok == 0)
        {
            
        }
        xfs_receive_ok = 0;
        if(xfs_data_buf[0] == 0xFC && xfs_data_buf[3] == 0x23)
        {  
            if(!Xfs_WaitResponse(XFS_ACK_CMD_RIGHT, XFS_RESPONSE_TIMEOUT))
            {
                Xfs_VoiceCODECStop();
                return 0;
            }

            while(!(dat[cnt] == 0xFC && dat[cnt+3] == 0x20))
            {
                cnt ++;
                if(cnt > 29000)
                {
                    Xfs_VoiceCODECStop();
                    return 0;
                }
            }
            dat[cnt] = 0xFD;
            dat[cnt+3] = 0x43;
            datlen = dat[cnt + 1] << 8 | dat[cnt + 2] + 4;
            USART3_SendStrlen(&dat[cnt],datlen);
        }
    }
	return 1; 
}

/*******************************************************************
*@brief		开始语音解码，配合编码传输函数使用
*@param		samp: 采样率
*           baud: 比特率
*           volum：音量
*           *data：编码数据
*@retval	>0：成功
*			0：失败
*			<0：参数错误
*******************************************************************/
signed char Xfs_VoiceDecodeStart(unsigned char samp,unsigned char baud,unsigned char volum,u8 *data)
{
    unsigned  char frame_info[7];
	unsigned short frame_len = 0;

    if(samp > 1)
        return -1;
    if(baud > 5)
        return -1;
    if(volum > 8)
        return -1;
/*****************帧固定配置信息**************************************/           
	frame_info[frame_len++] = 0xFD ; 			//构造帧头FD
	frame_info[frame_len++] = 0x00 ; 			//构造数据区长度的高字节
	frame_info[frame_len++] = 0x04; 		//构造数据区长度的低字节
    frame_info[frame_len++] = 0x42 ;      //解码
    
 	frame_info[frame_len++] = samp;             //参数1:设置采样率
    frame_info[frame_len++] = baud;             //参数2:设置比特率
    frame_info[frame_len++] = volum;            //参数3:设置音量

/*******************发送帧信息***************************************/
	USART3_SendStrlen(frame_info, frame_len);
    if(!Xfs_WaitResponse(XFS_ACK_CMD_RIGHT, XFS_RESPONSE_TIMEOUT))
    {
        Xfs_VoiceCODECStop();
        return 0;
    }
    memset(xfs_data_buf,0x00,80);
    delay_ms(20);
    if(Xfs_VoiceCodeTransmit(data))
    {
        Xfs_VoiceCODECStop();
        printf("Code End...\r\n");
        return 1;
    }
    else{
        printf("Code End...\r\n");
        return 0;
    }
}

/*******************************************************************
*@brief		停止编解码
*@param		None
*@retval	>0：成功
*			0：失败
*@attention
*******************************************************************/
unsigned char Xfs_VoiceCODECStop(void)
{
    unsigned  char frame_info[8]; //定义的文本长度
	uint16_t frame_len = 0;
	 
/*****************帧固定配置信息**************************************/           
	frame_info[frame_len++] = 0xFD ; 			//构造帧头FD
	frame_info[frame_len++] = 0x00 ; 			//构造数据区长度的高字节
	frame_info[frame_len++] = 0x01; 		//构造数据区长度的低字节
	frame_info[frame_len++] = XFS_CMD_EN_CODE_STOP ; 			//构造命令字 
 
/*******************发送帧信息***************************************/
    
    USART3_SendStrlen(frame_info,frame_len);
    if(Xfs_WaitResponse(XFS_ACK_CMD_RIGHT, XFS_RESPONSE_TIMEOUT))
        return 1;
    else
        return 0;
}

/*******************************************************************
*@brief 	完成XFS5152CE的数据接收
*@param		None
*@retval	None
*******************************************************************/
void Xfs_AckReceive(uint8_t dat)
{
	if(dat == 0xFC && xfs_protocol_dat_flag == 0)
	{
		xfs_timeout_cnt = 0;
		xfs_protocol_dat_cnt = 0;	// 接收数据计数
		xfs_data_buf[xfs_protocol_dat_cnt++] = dat;
		xfs_timer_flag1 = 1;
		xfs_protocol_dat_flag =  1;		
		xfs_receive_ok = 0;
		xfs_protocol_state = 0;
		xfs_protocol_dat_len = 0;	// 数据长度
    }
	else if(xfs_protocol_dat_flag == 0)
	{
        switch(dat)
        {
            case XFS_ACK_INIT_SUCCESS:
            case XFS_ACK_CMD_RIGHT:
            case XFS_ACK_CMD_ERROR:
            case XFS_ACK_BUSY:
            case XFS_ACK_IDEL:
                xfs_state = dat;
                xfs_receive_ok = 1;
            break;
        }
    }
	else if(xfs_protocol_dat_flag == 1)
    {
        if(xfs_protocol_dat_flag == 1)
        {
            switch(xfs_protocol_state)
            {
                case 0:
                    xfs_protocol_dat_len = (u16)dat << 8;
                    xfs_data_buf[xfs_protocol_dat_cnt++] = dat;
                    xfs_protocol_state = 1;
                    break;
                case 1:
                    xfs_protocol_dat_len |= dat;
                    if(xfs_protocol_dat_len == 1)
                        xfs_protocol_dat_len = 0;
                    xfs_data_buf[xfs_protocol_dat_cnt++] = dat;
                    xfs_protocol_state = 2;
                    if(xfs_protocol_dat_len > 42)
                        printf("datas is too long,%d\r\n",xfs_protocol_dat_len);
                    break;
                case 2:
                    if(xfs_protocol_dat_cnt <=  xfs_protocol_dat_len + 3)
                    {
                        xfs_data_buf[xfs_protocol_dat_cnt++] = dat;
                    }
                    if(xfs_protocol_dat_len > 2)
                        xfs_timer_flag2 = 1;
                    else{
                        xfs_timer_flag1 = 0;
                    }
                    break;
                default:
                    break;
            }
            if((xfs_protocol_state == 2)&& (xfs_protocol_dat_cnt == xfs_protocol_dat_len + 4))
            {
                xfs_receive_ok = 1;
                xfs_timer_flag1 = 0;
                xfs_timer_flag2 = 0;
                xfs_timeout_cnt = 0;
                xfs_protocol_state = 0;
                xfs_protocol_dat_len = 0;
                xfs_protocol_dat_flag = 0;
            }
        }
    }
}


/*******************************************************************
*@brief		防止接收中断的限时函数
*@param		None
*@retval	None
*******************************************************************/
void Xfs_ReceiveTimeLimt(void)
{
    if(xfs_timer_flag1 || xfs_timer_flag2)//正在接收串口数据
    {
        if(xfs_timer_flag1)
            xfs_time_uplimit = 10;
        if(xfs_timer_flag2)
            xfs_time_uplimit = 30;
        xfs_timeout_cnt++;//串口超时计数
        if(xfs_timeout_cnt > xfs_time_uplimit)//数据接收间隔超过0.3ms
        {
			//串口接收完成或结束
            xfs_timer_flag1 = xfs_timer_flag2 = 0;
            xfs_timeout_cnt = 0;
            xfs_receive_ok = 1;
            xfs_protocol_dat_flag = 0;
        }
    }   
}




void xfs_recognize_test(void)
{  
	uint8_t result=30;
	if(Xfs_VoiceRecognize(&result, 5000) != 1)
	{
		DEBUG("recognition failed!");
		Xfs_VoiceBroadcastStart((unsigned char *)"语音识别失败", XFS_FORMAT_GBK, 1);
		while(Xfs_GetStatus() != XFS_ACK_IDEL);
		return;
	}
	
	printf("get :%s\r\n",cmd_word_list[result]);
	Xfs_VoiceBroadcastStart((unsigned char *)cmd_word_list[result], XFS_FORMAT_GBK, 1);
	while(Xfs_GetStatus() != XFS_ACK_IDEL);
	

}




