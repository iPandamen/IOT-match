#ifndef __AS608_H
#define __AS608_H

/* include--------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"
#include "delay.h" 	
#include "usart.h"

/* define--------------------------------------------------------*/
#define AS608_Sta   PAin(4)	//读指纹模块状态引脚
#define CharBuffer1 0x01
#define CharBuffer2 0x02
#define AS608_DEFAULTADDR   0XFFFFFFFF //芯片默认地址

#define MYUSART_SendData USART2_SendData	// 使用的串口发送函数

/* typedef--------------------------------------------------------*/
typedef enum
{
	CMD = 0x01,
	DATA_ = 0x02,
	DATA_END = 0x08,
	RESPONSE = 0x07
}PackageTag;

typedef enum 
{
	GetImage = 0x01, 	//从传感器上读入图像存于图像缓冲区
	GenChar,		 	//根据原始图像生成指纹特征存于 CharBuffer1 或 CharBuffer2
	Match,			 	//精确比对 CharBuffer1 与 CharBuffer2 中的特征文件
	Search,			 	//以 CharBuffer1 或 CharBuffer2 中的特征文件搜索整个或部分指纹库
	RegModel,        	//将 CharBuffer1 与 CharBuffer2 中的特征文件合并生成模板存于CharBuffer2
	StoreChar,       	//将特征缓冲区中的文件储存到 flash 指纹库中
	LoadChar,		 	//从 flash 指纹库中读取一个模板到特征缓冲区
	UpChar,          	//将特征缓冲区中的文件上传给上位机
	DownChar,        	//从上位机下载一个特征文件到特征缓冲区
	UpImage,         	//上传原始图像
	DownImage,       	//下载原始图像
	DeletChar,       	//删除flash指纹库中的一个特征文件
	Empty,			 	//清空flash指纹库
	WriteReg,        	//写SOC系统寄存器
	ReadSysPara,     	//读系统基本参数
	Enroll,          	//注册模板
	Identify,        	//验证指纹
	SetPwd,          	//设置设备握手口令
	VfyPwd,			 	//验证握手口令	
	GetRandomCode,   	//采样随机数
	SetChipAddr,     	//设置芯片地址
	ReadFlashInfoPage,	//读取FLASH Information Page内容
	Port_Control,    	//通讯端口(UART/USB)开关控制
	WriteNotepad,    	//写记事本
	ReadNotepad,	 	//读记事本
	BurnCode,		 	//烧写片内FLASH（PS1802 SOC 该指令为烧写片外FALSH代码）
	HighSpeedSearch, 	//高速搜索FLASH
	GenBinImage,	 	//生成二值化指纹图像	 
	ValidTempleteNum,	//读有效模板个数
	UserGPIOConmmand,	//用户GPIO控制命令
	ReadIndexTable,	 	//读索引表            
}AS608_CmdCode;      
                     
typedef struct       
{                    
	u16 pageID;//指纹ID
	u16 mathscore;//匹配得分
}SearchResult;       
                     
typedef struct       
{                    
	u16 AS608_max;//指纹最大容量
	u8  AS608_level;//安全等级
	u32 AS608_addr;
	u8  AS608_size;//通讯数据包大小 0:32bytes 1:64bytes 2:128bytes 3:256bytes
	u8  AS608_N;//波特率基数N
}SysPara;

/* variable--------------------------------------------------------*/
extern SysPara AS_SysPara;	// 指纹模块AS608参数
extern u16 ValidN;	//模块内有效指纹个数

/* function--------------------------------------------------------*/
void AS608_StaGPIO_Init(void);//初始化PA6读状态引脚

void Recive_Data(void); // ??????????

u8 AS608_GetImage(void); //录入图像 
 
u8 AS608_GenChar(u8 BufferID);//生成特征 

u8 AS608_Match(void);//精确比对两枚指纹特征 

u8 AS608_Search(u8 BufferID,u16 StartPage,u16 PageNum,SearchResult *p);//搜索指纹 
 
u8 AS608_RegModel(void);//合并特征（生成模板） 
 
u8 AS608_StoreChar(u8 BufferID,u16 PageID);//储存模板 

uint8_t AS608_LoadChar(uint8_t BufferID,uint16_t PageID);//读出模板

uint8_t AS608_UpChar(uint8_t BufferID);//上传特征或模板

uint8_t AS608_DownChar(uint8_t BufferID);//下载特征或模板

u8 AS608_DeletChar(u16 PageID,u16 N);//删除模板 

u8 AS608_Empty(void);//清空指纹库 

u8 AS608_WriteReg(u8 RegNum,u8 DATA);//写系统寄存器 
 
u8 AS608_ReadSysPara(SysPara *p); //读系统基本参数 

u8 AS608_VfyPwd(void);//验证设备握手口令
	
u8 AS608_SetChipAddr(u32 addr);  //设置模块地址 

u8 AS608_WriteNotepad(u8 NotePageNum,u8 *content);//写记事本 

u8 AS608_ReadNotepad(u8 NotePageNum,u8 *note);//读记事 

u8 AS608_HighSpeedSearch(u8 BufferID,u16 StartPage,u16 PageNum,SearchResult *p);//高速搜索 
  
u8 AS608_ValidTempleteNum(u16 *ValidN);//读有效模板个数 

u8 AS608_HandShake(u32 *AS608_Addr); //与AS608模块握手

const char *EnsureMessage(u8 ensure);//确认码错误信息解析

#endif

