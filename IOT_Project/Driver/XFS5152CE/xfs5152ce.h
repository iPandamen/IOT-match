#ifndef __XFS5152CE_H
#define __XFS5152CE_H

#include "usart.h"
#include "string.h"
#include "stdlib.h"
#include "delay.h"


/* Includes ------------------------------------------------------------------*/
/* Public typedef ------------------------------------------------------------*/
/* 中文编码格式 */
typedef enum 
{
	XFS_FORMAT_GB2312 = 0x00,
	XFS_FORMAT_GBK = 0x01,
	XFS_FORMAT_BIG5 = 0x02,
	XFS_FORMAT_UNICODE = 0x03,
}xfsEnodingFormatEnum;

/* 响应状态 */
typedef enum 
{
	XFS_ACK_CMD_RIGHT = 0x41,
	XFS_ACK_CMD_ERROR = 0x45,
	XFS_ACK_INIT_SUCCESS = 0x4A,
	XFS_ACK_BUSY = 0x4E,
	XFS_ACK_IDEL = 0x4F,
}xfsAckStatusEnum;


extern const char *cmd_word_list[]; 



// xfs5152端口初始化
void Xfs5152ce_Init(void);	

// 开始语音播报
signed char Xfs_VoiceBroadcastStart(unsigned char *data, unsigned char format, int wait_finish);	

//播放缓存内容
unsigned char Xfs_PalyFromCache(unsigned char num, unsigned char format);	

//发送文本到缓存区
signed char Xfs_SendTextToCache(unsigned char *data, unsigned char cache_num);	

//语音识别
unsigned char Xfs_VoiceRecognize(unsigned char *result, signed int waittime);

//继续语音播报
unsigned char Xfs_VoiceBroadcastRestore(void);	

//暂停语音播报
unsigned char Xfs_VoiceBroadcastPause(void);	

//停止语音播报
unsigned char Xfs_VoiceBroadcastStop(void);	

//唤醒XFS5152CE
unsigned char Xfs_WakeUp(void);	

//使XFS5152CE进入睡眠状态
unsigned char Xfs_Sleep(void);	

//获得XFS5152CE的当前状态
unsigned char Xfs_GetStatus(void);	

//停止编解码
unsigned char Xfs_VoiceCODECStop(void);	

//开始语音编码
signed char Xfs_VoiceEncodeStart(unsigned char samp,unsigned char baud,unsigned char volum);

//开始语音解码，配合编码传输函数使用
signed char Xfs_VoiceDecodeStart(unsigned char samp,unsigned char baud,unsigned char volum,unsigned char *data);

//完成XFS5152CE的数据接收
void Xfs_AckReceive(unsigned char dat);	

//防止接收中断的限时函数
void Xfs_ReceiveTimeLimt(void); 


#endif



