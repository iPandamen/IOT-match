#ifndef _hal_ldr_h_
#define _hal_ldr_h_

#include "stdint.h"
#include "sys.h"
/********************************************************
*函数名称：fp32 LDR_Lux(uint8_t ch)
*功能：根据ADC的通道号转换输出光线的流明度Lux
*输入：uint8_t ch - ADC通道号
*输出：光线的流明度，单位：Lux；
*特殊说明：这里的ADC芯片是PCF8591,
*					光敏电阻：light dependent resistor => 缩写：LDR,
*					Lux:勒克斯（照明单位）,标称光亮度的光束均匀射在
*					l m2 面积上时的照度.
**********************************************************/
float LDR_Lux(uint8_t ch);		

#endif /*_hal_ldr_h_*/
