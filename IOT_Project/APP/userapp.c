#include "userapp.h"

void Key_Operation()
{
	u8 ret;
	static KBD_EVENT key;
	ret = get_kbd_fifo(&key);
	if(ret == 0)
	{
		switch(key.key_num)
		{
			case KBD_KEY0:
			case KBD_KEY1:
			case KBD_KEY2:
			case KBD_KEY3:
			case KBD_KEY4:		
			case KBD_KEY5:
			case KBD_KEY6:
			case KBD_KEY7:
			case KBD_KEY8:
			case KBD_KEY9:
			case KBD_UP:
				break;
			case KBD_DOWN:
				break;
			case KBD_LEFT:
				break;
			case KBD_RIGHT:
				break;
			case KBD_PROJ1:
				if(key.key_event == KBD_KEY_DONW)
				{
					Current_Menu = MENU1;
					printf("KBD_PROJ1 Down\r\n");
				}
				break;
			case KBD_PROJ2:
				if(key.key_event == KBD_KEY_DONW)
				{
					Current_Menu = MENU2;
					printf("KBD_PROJ2 Down\r\n");
				}
				break;
			case KBD_PROJ3:
				if(key.key_event == KBD_KEY_DONW)
				{
					Current_Menu = MENU3;
					printf("KBD_PROJ3 Down\r\n");
				}
				break;
			case KBD_PROJ4:
				if(key.key_event == KBD_KEY_DONW)
				{
					Current_Menu = MENU4;
					printf("KBD_PROJ4 Down\r\n");
				}
				break;
			case KBD_PROJ5:
				if(key.key_event == KBD_KEY_DONW)
				{
					Current_Menu = MENU5;
					printf("KBD_PROJ5 Down\r\n");
				}
				break;
			case KBD_FUNC:
				break;
			case KEY_HASH_KEY:
				break;
			case KEY_KPASTERISK:
				break;
			case KEY_BACKSPACE:
				break;
			case KEY_ENTER:
				if(key.key_event == KBD_KEY_DONW)
				{
				}
				break;
			case KEY_ESC:
				if(key.key_event==KBD_KEY_DONW)
					printf("KEY_ESC Down\r\n");
				break;
			case KEY_INVALID:
				
				break;
	
			default:break;
		}	
	}
}	
	

//显示确认码错误信息
void ShowErrMessage(u8 ensure)
{
	printf("%s\r\n",EnsureMessage(ensure));
}

//
uint8_t M3_GetNum(void)
{
	uint8_t val=0;
	while(1)
	{
		m3_key_sta = M3_key_scan(0);
		if(m3_key_sta == KEY2_PRESS)
		{
			val++;	
			printf("The val you enter is %d, Enter KEY2 to confirm the val!\r\n",val);
		}		
		else if(m3_key_sta == KEY1_PRESS)
		{
			break;
		}	
	}
	return val;
}

//录指纹
void Add_FR(void)
{
	u8 i,ensure ,processnum=0;
	u16 ID;
	while(1)
	{
		switch (processnum)
		{
			case 0:
				i++;
				printf("请按指纹\r\n");
				ensure=AS608_GetImage();
				if(ensure==0x00) 
				{
//					BEEP=1;
					ensure=AS608_GenChar(CharBuffer1);//生成特征
//					BEEP=0;
					if(ensure==0x00)
					{
						printf("指纹正常\r\n");
						i=0;
						processnum=1;//跳到第二步						
					}
					else 
						ShowErrMessage(ensure);				
				}
				else 
					ShowErrMessage(ensure);						
				break;
			
			case 1:
				i++;
				printf("请按再按一次指纹\r\n");
				ensure=AS608_GetImage();
				if(ensure==0x00) 
				{
//					BEEP=1;
					ensure=AS608_GenChar(CharBuffer2);//生成特征
//					BEEP=0;
					if(ensure==0x00)
					{
						printf("指纹正常\r\n");
						i=0;
						processnum=2;//跳到第三步
					}
					else 
						ShowErrMessage(ensure);	
				}
				else 
					ShowErrMessage(ensure);		
				break;

			case 2:
				printf("对比两次指纹\r\n");
				ensure=AS608_Match();
				if(ensure==0x00) 
				{
					printf("对比成功,两次指纹一样\r\n");
					processnum=3;//跳到第四步
				}
				else 
				{
					printf("对比失败，请重新录入指纹\r\n");
					ShowErrMessage(ensure);
					i=0;
					processnum=0;//跳回第一步		
				}
				delay_ms(1200);
				break;

			case 3:
				printf("生成指纹模板\r\n");
				ensure=AS608_RegModel();
				if(ensure==0x00) 
				{
					printf("生成指纹模板成功\r\n");
					processnum=4;//跳到第五步
				}
				else 
				{
					processnum=0;
					ShowErrMessage(ensure);
				}
				delay_ms(1200);
				break;
				
			case 4:	
				printf("请输入储存ID,按Enter保存");
				printf("0=< ID <=299\r\n");
				do
					ID=M3_GetNum();
				while(!(ID<AS_SysPara.AS608_max));//输入ID必须小于容量的最大值
				ensure=AS608_StoreChar(CharBuffer2,ID);//储存模板
				if(ensure==0x00) 
				{						
					printf("录入指纹成功\r\n");
					AS608_ValidTempleteNum(&ValidN);//读库指纹个数
					printf("%d\r\n",AS_SysPara.AS608_max-ValidN);
					delay_ms(1500);
					return ;
				}
				else 
				{
					processnum=0;
					ShowErrMessage(ensure);
				}					
				break;				
		}
		delay_ms(400);
		if(i >= 5)//超过5次没有按手指则退出
		{
			break;	
		}				
	}
}

//刷指纹
uint8_t press_FR(void)
{
	SearchResult seach;
	u8 ensure;
	char *str;
	ensure=AS608_GetImage();
	if(ensure==0x00)//获取图像成功 
	{	
		buzzer_ctrl(ON);	//打开蜂鸣器	
		ensure=AS608_GenChar(CharBuffer1);
		if(ensure==0x00) //生成特征成功
		{		
			buzzer_ctrl(OFF);//关闭蜂鸣器	
			ensure=AS608_HighSpeedSearch(CharBuffer1,0,AS_SysPara.AS608_max,&seach);
			if(ensure==0x00)//搜索成功
			{				
				printf("刷指纹成功\r\n");						
				printf(str,"确有此人,ID:%d  匹配得分:%d\r\n",seach.pageID,seach.mathscore);
				door_open(500);
			}
			else 
			{	
				ShowErrMessage(ensure);					
				return ensure;
			}	
		}
		else
		{
			ShowErrMessage(ensure);
			return ensure;
		}
		buzzer_ctrl(OFF);//关闭蜂鸣器
		delay_ms(600);
	}
	return ensure;	
}

//删除指纹
void Del_FR(void)
{
	u8  ensure;
	u16 num;
	printf("删除指纹\r\n");
	printf("请输入指纹ID按Enter发送\r\n");
	printf("0=< ID <=299r\r\n");
	delay_ms(50);
	num = M3_GetNum();
	if(num==0xFFFF)
		goto MENU ; //返回主页面
	else if(num == 0xFF00)
		ensure=AS608_Empty();//清空指纹库
	else 
		ensure=AS608_DeletChar(num,1);//删除单个指纹
	if(ensure == 0)
	{
		printf("删除指纹成功\r\n");		
	}
  else
		ShowErrMessage(ensure);	
	delay_ms(1200);
	AS608_ValidTempleteNum(&ValidN);//读库指纹个数
	printf("ValidN:%d\r\n",AS_SysPara.AS608_max-ValidN);
MENU:	
	{
		;
	}
}

void Upload_Char(u8 *data)
{
	u8 ensure;
	printf("读图像中... ...\r\n");
	ensure = AS608_GetImage();
	ShowErrMessage(ensure);
	if(ensure == 0)
	{
		printf("读图像成功!\r\n");
		printf("生成特征码... ...\r\n");
		ensure = AS608_GenChar(CharBuffer1);
		ShowErrMessage(ensure);
		if(ensure == 0)
		{
			printf("生成特征码成功\r\n");
			printf("上传特征码中... ...\r\n");
			ensure = AS608_UpChar(CharBuffer1);
			ShowErrMessage(ensure);
			if(ensure == 0)
			{
				printf("等待数据包接收完毕!\r\n");
				Recive_Data();
			}
		}
	}
}

void device_sta_output(void)
{
	printf("\r\n");
	printf("sta_lock: %s\r\n", lock_status[device_sta.sta_lock]);
	printf("sta_lamp: %s\r\n", lock_status[device_sta.sta_lamp]);
	printf("sta_buzzer: %s\r\n", lock_status[device_sta.sta_buzzer]);
	printf("\r\n");
}
	
void userApp(void)
{
	uint16_t i=0;
	uint8_t buf[100]={0};
	while(1)
	{
		Key_Operation();
		switch(Current_Menu)
		{
			case MENU1:
				LCD_PRINT(0,0,lcd_show_buf,"MENU1");
				ESP8266_GetIpData(AT_RX_BUF,(char*)data_buf); 	// 从 ESP8266 获取数据
				ClrAtRxBuf();
				NLECloud_DataAnalysis_TCP((char*)data_buf);	// 分析	server 发送的数据
				break;
			case MENU2:
				LCD_PRINT(0,0,lcd_show_buf,"MENU2");
				//串口命令控制
				USART1_DataAnalysis();
				break;
			case MENU3:
				LCD_PRINT(0,0,lcd_show_buf,"MENU3_LF");
				if(m3_key_sta == KEY1_PRESS)
				{
					
				}
				break;
			case MENU4:
				LCD_PRINT(0,0,lcd_show_buf,"MENU4_FR");
				if(m3_key_sta == KEY1_PRESS)
				{
					printf("Add FR!\r  \n");
//					Add_FR();
//					ensure=AS608_LoadChar(CharBuffer1,0);
//					ShowErrMessage(ensure);
//					ensure=AS608_UpChar(CharBuffer1);
//					ShowErrMessage(ensure);
//					Recive_Data();
//					Upload_Char(data_char);
				}
				break;
			case MENU5:
				LCD_PRINT(0,0,lcd_show_buf,"MENU5");
				if(m3_key_sta == KEY1_PRESS)
				{
	
				}
				break;		
		}
		
		i++;
		delay_ms(10);
		if(i%300 == 0)
			ESP8266_IpSend((char *)PING_REQ, strlen((const char *)PING_REQ));
		if(i%100 == 0)
		{	
			printf("\r\n\r\nCurrent Menu:%3d\r\n",Current_Menu);
			
			printf("send sta_lock:%3d\r\n",device_sta.sta_lock);
			sprintf((char*)buf,"2019-09-15 %d-%d-%d",calendar.hour,calendar.min,calendar.sec);
			printf("date:%s\r\n",buf);
			Data_Report_TCP_Number(2,"sta_lock",(char*)buf,device_sta.sta_lock,001);
			
			printf("send sta_ligth:%3d\r\n",device_sta.sta_lamp);
			sprintf((char*)buf,"2019-09-15 %d-%d-%d",calendar.hour,calendar.min,calendar.sec);
			Data_Report_TCP_Number(2,"sta_lamp",(char*)buf,device_sta.sta_lamp,002);
		}
		if(i%500 == 0)
			device_sta_output();
		
		LCD_PRINT(1,0,lcd_show_buf,"Lock:%s",lock_status[device_sta.sta_lock]);
		LCD_PRINT(2,0,lcd_show_buf,"Light:%s",light_status[device_sta.sta_lamp]);
	}
}

