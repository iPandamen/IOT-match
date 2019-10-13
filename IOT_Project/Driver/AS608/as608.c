


#include "as608.h"

const uint8_t HeadPackage[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x00};
//应答包包头
const uint8_t HeadResponse[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x07,0x00};
//数据包包头
const uint8_t HeadData[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x02,0x00};
//结束包包头
const uint8_t HeadDataEnd[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x08,0x00};

SysPara AS_SysPara;	//指纹模块AS608参数

u16 ValidN=0; // //模块内有效指纹个数

//初始化PA6为下拉输入		    
//读摸出感应状态(触摸感应时输出高电平信号)
void AS608_StaGPIO_Init(void)
{   
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//使能GPIOA时钟
	//初始化读状态引脚GPIOA
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//输入下拉模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO	
}

////串口发送一个字节
//static void MYUSART_SendData(u8 data)
//{
//	while((USART2->SR&0X40)==0); 
//	USART2->DR = data;
//}

//发送包头
void SendHead(void)
{
	MYUSART_SendData(0xEF);
	MYUSART_SendData(0x01);
}
//发送地址
void SendAddr(void)
{
	MYUSART_SendData((u8)(AS608_DEFAULTADDR>>24));
	MYUSART_SendData((u8)(AS608_DEFAULTADDR>>16));
	MYUSART_SendData((u8)(AS608_DEFAULTADDR>>8));
	MYUSART_SendData((u8)(AS608_DEFAULTADDR));
}
//发送包标识
void SendFlag(u8 flag)
{
	MYUSART_SendData(flag);
}
//发送包长度
void SendLength(int length)
{
	MYUSART_SendData(length>>8);
	MYUSART_SendData(length);
}
//发送指令码
void Sendcmd(u8 cmd)
{
	MYUSART_SendData(cmd);
}
//发送校验和
void SendCheck(u16 check)
{
	MYUSART_SendData(check>>8);
	MYUSART_SendData(check);
}

//判断中断接收的数组有没有应答包
//waittime为等待中断接收数据的时间（单位1ms）
//返回值：数据包首地址
u8 *Recive_response(u16 waittime)
{
	char *data;
	uint16_t len;
	USART2_RX_STA=0;
	while(--waittime)
	{
		delay_ms(1);
		if(USART2_RX_STA&0X8000)//接收到一次数据
		{
			len = USART2_RX_STA & 0x7FFF;
			printf_hex(USART2_RX_BUF,len);
			data=strstr((const char*)USART2_RX_BUF,(const char*)HeadResponse);
			if(data)
				return (u8*)data;	
		}
	}
	return 0;
}

//接收数据包
void Recive_Data(void)
{
	uint8_t *temp;
	USART2_RX_STA=0;
	while(1)
	{
		delay_ms(1);
//		USART2_RX_STA=0;
		temp = (uint8_t *)strstr((const char*)USART2_RX_BUF,(const char*)HeadData);
		if(strstr((const char*)USART2_RX_BUF,(const char*)HeadDataEnd))
		{
			break;			
		}
	}
	printf("Data:\r\n");
	printf("%s\r\n",temp);
}

//录入图像 AS608_GetImage
//功能:探测手指，探测到后录入指纹图像存于ImageBuffer。 
//模块返回确认字
u8 AS608_GetImage(void)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x03);
	Sendcmd(GetImage);
	temp =  0x01+0x03+GetImage;
	SendCheck(temp);
	data=Recive_response(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}

//生成特征 AS608_GenChar
//功能:将ImageBuffer中的原始图像生成指纹特征文件存于CharBuffer1或CharBuffer2			 
//参数:BufferID --> charBuffer1:0x01	charBuffer1:0x02												
//模块返回确认字
u8 AS608_GenChar(u8 BufferID)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x04);
	Sendcmd(GenChar);
	MYUSART_SendData(BufferID);
	temp = 0x01+0x04+GenChar+BufferID;
	SendCheck(temp);
	data=Recive_response(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}

//精确比对两枚指纹特征 AS608_Match
//功能:精确比对CharBuffer1 与CharBuffer2 中的特征文件 
//模块返回确认字
u8 AS608_Match(void)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x03);
	Sendcmd(Match);
	temp = 0x01+0x03+Match;
	SendCheck(temp);
	data=Recive_response(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//搜索指纹 AS608_Search
//功能:以CharBuffer1或CharBuffer2中的特征文件搜索整个或部分指纹库.若搜索到，则返回页码。			
//参数:  BufferID @ref CharBuffer1	CharBuffer2
//说明:  模块返回确认字，页码（相配指纹模板）
u8 AS608_Search(u8 BufferID,u16 StartPage,u16 PageNum,SearchResult *p)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x08);
	Sendcmd(Search);	//指令码
	MYUSART_SendData(BufferID);
	MYUSART_SendData(StartPage>>8);
	MYUSART_SendData(StartPage);
	MYUSART_SendData(PageNum>>8);
	MYUSART_SendData(PageNum);
	temp = 0x01+0x08+Search+BufferID
	+(StartPage>>8)+(u8)StartPage
	+(PageNum>>8)+(u8)PageNum;
	SendCheck(temp);
	data=Recive_response(2000);
	if(data)
	{
		ensure = data[9];
		p->pageID   =(data[10]<<8)+data[11];
		p->mathscore=(data[12]<<8)+data[13];	
	}
	else
		ensure = 0xff;
	return ensure;	
}
//合并特征（生成模板）AS608_RegModel
//功能:将CharBuffer1与CharBuffer2中的特征文件合并生成 模板,结果存于CharBuffer1与CharBuffer2	
//说明:  模块返回确认字
u8 AS608_RegModel(void)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x03);
	Sendcmd(RegModel);
	temp = 0x01+0x03+RegModel;
	SendCheck(temp);
	data=Recive_response(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;		
}
//储存模板 AS608_StoreChar
//功能:将 CharBuffer1 或 CharBuffer2 中的模板文件存到 PageID 号flash数据库位置。			
//参数:  BufferID @ref charBuffer1:0x01	charBuffer1:0x02
//       PageID（指纹库位置号）
//说明:  模块返回确认字
u8 AS608_StoreChar(u8 BufferID,u16 PageID)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x06);
	Sendcmd(StoreChar);
	MYUSART_SendData(BufferID);
	MYUSART_SendData(PageID>>8);
	MYUSART_SendData(PageID);
	temp = 0x01+0x06+StoreChar+BufferID
	+(PageID>>8)+(u8)PageID;
	SendCheck(temp);
	data=Recive_response(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;	
}
//功能说明： 将 flash 数据库中指定 ID 号的指纹模板读入到模板缓冲区 CharBuffer1 或 CharBuffer2
//输入参数： BufferID(缓冲区号)，PageID(指纹库模板号)
// 返回参数： 确认字
uint8_t AS608_LoadChar(uint8_t BufferID,uint16_t PageID)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x06);
	Sendcmd(LoadChar);
	MYUSART_SendData(BufferID);
	MYUSART_SendData(PageID>>8);
	MYUSART_SendData(PageID);
	temp = 0x01+0x06+LoadChar+BufferID
	+(PageID>>8)+(u8)PageID;
	SendCheck(temp);
	data=Recive_response(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;	
}

//上传特征或模板 PS_UpChar
//功能说明： 将特征缓冲区中的特征文件上传给上位机
//输入参数： BufferID(缓冲区号)
//返回参数： 确认字
//应答之后发送后续数据包
uint8_t AS608_UpChar(uint8_t BufferID)
{
	u16 temp;
	u8 ensure;
	u8 *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x04);
	Sendcmd(UpChar);
	MYUSART_SendData(BufferID);
	temp = 0x01+0x04+UpChar+BufferID;
	SendCheck(temp);
	data = Recive_response(2000);
	if(data)
		ensure = data[9];
	else
		ensure=0xff;
	
	if(ensure==0)
	{
		
	}
	return ensure;	
}


//下载特征或模板 PS_DownChar
//功能说明： 上位机下载特征文件到模块的一个特征缓冲区
//输入参数： BufferID(缓冲区号)
//返回参数： 确认字
uint8_t AS608_DownChar(uint8_t BufferID)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x04);
	Sendcmd(DownChar);
	MYUSART_SendData(BufferID);
	temp = 0x01+0x04+DownChar+BufferID;
	SendCheck(temp);
	data=Recive_response(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;	
}

//上传图像 PS_UpImage
//功能说明： 将图像缓冲区中的数据上传给上位机
//输入参数： none
//返回参数： 确认字
uint8_t AS608_UpImage(void)
{
	u8  ensure;
	
	return ensure;	
}

//下载图像 PS_DownImage
// 功能说明： 上位机下载图像数据给模块
// 输入参数： none
// 返回参数： 确认字
uint8_t AS608_DownImage(void)
{
	return 0;
}


//删除模板 AS608_DeletChar
//功能:  删除flash数据库中指定ID号开始的N个指纹模板
//参数:  PageID(指纹库模板号)，N删除的模板个数。
//说明:  模块返回确认字
u8 AS608_DeletChar(u16 PageID,u16 N)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x07);
	Sendcmd(DeletChar);
	MYUSART_SendData(PageID>>8);
	MYUSART_SendData(PageID);
	MYUSART_SendData(N>>8);
	MYUSART_SendData(N);
	temp = 0x01+0x07+DeletChar
	+(PageID>>8)+(u8)PageID
	+(N>>8)+(u8)N;
	SendCheck(temp);
	data=Recive_response(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//清空指纹库 AS608_Empty
//功能:  删除flash数据库中所有指纹模板
//参数:  无
//说明:  模块返回确认字
u8 AS608_Empty(void)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x03);
	Sendcmd(Empty);
	temp = 0x01+0x03+Empty;
	SendCheck(temp);
	data=Recive_response(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
/*********************************************************
*@brief		写系统寄存器 AS608_WriteReg
*	   		写模块寄存器
*@param		RegNum:寄存器序号4\5\6
*	@arg	4: 波特率控制寄存器
*	@arg	5: 比对阈值寄存器
*	@arg	6: 包大小寄存器
*@param		DATA: 写入的数据
*	@arg	RegNum == 4，9600的倍数
*	@arg	RegNum == 5, 1:level1,2:level2,**,5:level5
*	@arg	RegNum == 6, 0:32bytes,1:64bytes,2:128bytes,3:256bytes
*@return	模块返回确认字
**********************************************************/
u8 AS608_WriteReg(u8 RegNum,u8 DATA)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x05);
	Sendcmd(WriteReg);
	MYUSART_SendData(RegNum);
	MYUSART_SendData(DATA);
	temp = RegNum+DATA+0x01+0x05+WriteReg;
	SendCheck(temp);
	data=Recive_response(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	if(ensure==0)
		printf("\r\n设置参数成功！");
	else
		printf("\r\n%s",EnsureMessage(ensure));
	return ensure;
}

/*****************************************************
*@brief		读系统基本参数 AS608_ReadSysPara
*			读取模块的基本参数（波特率，包大小等)
*@parm  	SysPara *p:读出的系统基本参数
*@return	模块返回确认字 + 基本参数（16bytes）
******************************************************/
u8 AS608_ReadSysPara(SysPara *param)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x03);
	Sendcmd(ReadSysPara);
	temp = 0x01+0x03+ReadSysPara;
	SendCheck(temp);
	data=Recive_response(1000);
	if(data)
	{
		ensure = data[9];
		param->AS608_max = (data[14]<<8)+data[15];
		param->AS608_level = data[17];
		param->AS608_addr=(data[18]<<24)+(data[19]<<16)+(data[20]<<8)+data[21];
		param->AS608_size = data[23];
		param->AS608_N = data[25];
	}		
	else
		ensure=0xff;
	if(ensure==0x00)
	{
		printf("模块最大指纹容量=%d\r\n",param->AS608_max);
		printf("对比等级=%d\r\n",param->AS608_level);
		printf("地址=%#x\r\n",param->AS608_addr);
		printf("波特率=%d\r\n",param->AS608_N*9600);
	}
	else 
			printf("\r\n%s",EnsureMessage(ensure));
	return ensure;
}

//PS_VfyPwd
//指令代码：13H
//功能：验证设备握手口令
u8 AS608_VfyPwd(void)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x07);
	Sendcmd(VfyPwd);
	MYUSART_SendData(0x00);
	MYUSART_SendData(0x00);
	MYUSART_SendData(0x00);
	MYUSART_SendData(0x00);
	temp = 0x01+0x07+VfyPwd;
	SendCheck(temp);
	data=Recive_response(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;	
	if(ensure==0x00)
		printf("\r\n握手口令验证成功！");
	else
		printf("\r\n%s",EnsureMessage(ensure));
	return ensure;
}

//设置模块地址 AS608_SetAddr
//功能:  设置模块地址
//参数:  AS608_addr
//说明:  模块返回确认字
u8 AS608_SetChipAddr(u32 AS608_addr)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x07);
	Sendcmd(SetChipAddr);
	MYUSART_SendData(AS608_addr>>24);
	MYUSART_SendData(AS608_addr>>16);
	MYUSART_SendData(AS608_addr>>8);
	MYUSART_SendData(AS608_addr);
	temp = 0x01+0x07+SetChipAddr
	+(u8)(AS608_addr>>24)+(u8)(AS608_addr>>16)
	+(u8)(AS608_addr>>8) +(u8)AS608_addr;				
	SendCheck(temp);
	AS_SysPara.AS608_addr =AS608_addr;//发送完指令，更换地址
	data=Recive_response(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;	
		AS_SysPara.AS608_addr = AS608_addr;
	if(ensure==0x00)
		printf("\r\n设置地址成功！");
	else
		printf("\r\n%s",EnsureMessage(ensure));
	return ensure;
}


//功能： 模块内部为用户开辟了256bytes的FLASH空间用于存用户记事本,
//	该记事本逻辑上被分成 16 个页。
//参数:  NotePageNum(0~15),Byte32(要写入内容，32个字节)
//说明:  模块返回确认字
u8 AS608_WriteNotepad(u8 NotePageNum,u8 *Byte32)
{
	u16 temp;
	u8  ensure,i;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(36);
	Sendcmd(WriteNotepad);
	MYUSART_SendData(NotePageNum);
	for(i=0;i<32;i++)
	 {
		 MYUSART_SendData(Byte32[i]);
		 temp += Byte32[i];
	 }
	temp =0x01+36+WriteNotepad+NotePageNum+temp;
	SendCheck(temp);
	data=Recive_response(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//读记事AS608_ReadNotepad
//功能：  读取FLASH用户区的128bytes数据
//参数:  NotePageNum(0~15)
//说明:  模块返回确认字+用户信息
u8 AS608_ReadNotepad(u8 NotePageNum,u8 *Byte32)
{
	u16 temp;
	u8  ensure,i;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x04);
	Sendcmd(ReadNotepad);
	MYUSART_SendData(NotePageNum);
	temp = 0x01+0x04+ReadNotepad+NotePageNum;
	SendCheck(temp);
	data=Recive_response(2000);
	if(data)
	{
		ensure=data[9];
		for(i=0;i<32;i++)
		{
			Byte32[i]=data[10+i];
		}
	}
	else
		ensure=0xff;
	return ensure;
}
//高速搜索AS608_HighSpeedSearch
//功能：以 CharBuffer1或CharBuffer2中的特征文件高速搜索整个或部分指纹库。
//		  若搜索到，则返回页码,该指令对于的确存在于指纹库中 ，且登录时质量
//		  很好的指纹，会很快给出搜索结果。
//参数:  BufferID， StartPage(起始页)，PageNum（页数）
//说明:  模块返回确认字+页码（相配指纹模板）
u8 AS608_HighSpeedSearch(u8 BufferID,u16 StartPage,u16 PageNum,SearchResult *p)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x08);
	Sendcmd(HighSpeedSearch);
	MYUSART_SendData(BufferID);
	MYUSART_SendData(StartPage>>8);
	MYUSART_SendData(StartPage);
	MYUSART_SendData(PageNum>>8);
	MYUSART_SendData(PageNum);
	temp = 0x01+0x08+HighSpeedSearch+BufferID
	+(StartPage>>8)+(u8)StartPage
	+(PageNum>>8)+(u8)PageNum;
	SendCheck(temp);
	data=Recive_response(2000);
 	if(data)
	{
		ensure=data[9];
		p->pageID 	=(data[10]<<8) +data[11];
		p->mathscore=(data[12]<<8) +data[13];
	}
	else
		ensure=0xff;
	return ensure;
}
//读有效模板个数 AS608_ValidTempleteNum
//功能：读有效模板个数
//参数: 无
//说明: 模块返回确认字+有效模板个数ValidN
u8 AS608_ValidTempleteNum(u16 *ValidN)
{
	u16 temp;
	u8  ensure;
	u8  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x03);
	Sendcmd(ValidTempleteNum);
	temp = 0x01+0x03+ValidTempleteNum;
	SendCheck(temp);
	data=Recive_response(2000);
	if(data)
	{
		ensure=data[9];
		*ValidN = (data[10]<<8) +data[11];
	}		
	else
		ensure=0xff;
	
	if(ensure==0x00)
	{
		printf("\r\n有效指纹个数=%d\r\n",(data[10]<<8)+data[11]);
	}
	else
		printf("%s\r\n",EnsureMessage(ensure));
	return ensure;
}

//模块应答包确认码信息解析
//功能：解析确认码错误信息返回信息
//参数: ensure
const char *EnsureMessage(u8 ensure) 
{
	const char *p;
	switch(ensure)
	{
		case  0x00:
			p="OK";break;		
		case  0x01:
			p="数据包接收错误";break;
		case  0x02:
			p="传感器上没有手指";break;
		case  0x03:
			p="录入指纹图像失败";break;
		case  0x04:
			p="指纹图像太干、太淡而生不成特征";break;
		case  0x05:
			p="指纹图像太湿、太糊而生不成特征";break;
		case  0x06:
			p="指纹图像太乱而生不成特征";break;
		case  0x07:
			p="指纹图像正常，但特征点太少（或面积太小）而生不成特征";break;
		case  0x08:
			p="指纹不匹配";break;
		case  0x09:
			p="没搜索到指纹";break;
		case  0x0a:
			p="特征合并失败";break;
		case  0x0b:
			p="访问指纹库时地址序号超出指纹库范围";
		case  0x10:
			p="删除模板失败";break;
		case  0x11:
			p="清空指纹库失败";break;	
		case  0x15:
			p="缓冲区内没有有效原始图而生不成图像";break;
		case  0x18:
			p="读写 FLASH 出错";break;
		case  0x19:
			p="未定义错误";break;
		case  0x1a:
			p="无效寄存器号";break;
		case  0x1b:
			p="寄存器设定内容错误";break;
		case  0x1c:
			p="记事本页码指定错误";break;
		case  0x1f:
			p="指纹库满";break;
		case  0x20:
			p="地址错误";break;
		default :
			p="模块返回确认码有误";break;
	}
 return p;	
}





