/**********************************************************
*	@file 		fan.c
*	@author		Panda
*	@version	V0.0.0
*	@date		2019-09
*	@brief
***********************************************************/
#include "fan.h"

/*******************************************
*	@brief	fan	GPIO引脚初始化
*	@param	None
*	@retval	None
********************************************/

void fan_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_FAN_PORT,ENABLE);

	GPIO_InitStructure.GPIO_Pin = FAN_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(FAN_PORT, &GPIO_InitStructure);
	
	GPIO_ResetBits(FAN_PORT,FAN_PIN);
}

/*******************************************
*	@brief		fan control 
*	@param		sta: fan 状态
*		@arg	1: fan on
*		@arg 	0: fon off
*	@retval		None
********************************************/
void fan_ctrl(bool sta)
{
	if(sta == ON)
	{
		if(device_sta.sta_fan == OFF)
		{
			GPIO_SetBits(FAN_PORT,FAN_PIN);
			device_sta.sta_fan = ON;
		}
		else
		{	
		}
	}
	else if(sta == OFF)
	{	
		if(device_sta.sta_fan == ON)
		{
			GPIO_ResetBits(FAN_PORT,FAN_PIN);
			device_sta.sta_fan = OFF;
		}
		else
		{
			
		}
	}
}

/****************************************************
*@brief		获取风扇的状态
*@param		None
*@retval	返回风扇的状态
*	@val	ON:
*	@val 	OFF:
*****************************************************/
bool fan_getsta(void)
{
	device_sta.sta_fan = FAN_STA;
	return device_sta.sta_fan;
}
 
/***************************************END OF FILE**********************************************/
