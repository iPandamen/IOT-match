#include "math.h"
#include "ldr.h"
#include "pcf8591.h"
#include "stdio.h"
#include "stdint.h"

#define V_REF			3.3     				//参考电压，单位：V
#define RES_REF  		10000 					//参考电阻，单位：Ω
#define ResolutionRatio	256						//ADC分辨率：8位分辨率，2^8=256

/********************************************************
*@brief		: 输入光敏电阻两端ADC值，输出光敏电阻值，单位：Ω；
*@param		: uint16_t ADC_Value，光敏电阻两端ADC值，
*@retval	: 光敏电阻值，单位：Ω；
*@attention	: ADC分辨率：8位分辨率，2^8=256
**********************************************************/
uint32_t LDR_CalculateResValue(uint16_t ADC_Value)
{
	float Vrt=0.0;
	uint32_t Rrt=0;
	
	Vrt=(V_REF/(ResolutionRatio-1))*ADC_Value;//光敏电阻两端电压
	Rrt=Vrt/((V_REF-Vrt)/RES_REF);//光敏电阻，阻值 
#if 0	
	printf("Vrt = %fV\r\n",Vrt);
#endif /*  */
	return (uint32_t)Rrt;
}

//Lux:勒克斯（照明单位）,标称光亮度的光束均匀射在 l m2 面积上时的照度
//光敏电阻：light dependent resistor => 缩写：LDR

/********************************************************
*@brief		: 根据ADC的通道号转换输出光线的流明度Lux
*@param		: uint8_t ch - ADC通道号
*@retval	: 光线的流明度，单位：Lux；
*@attention	: 这里的ADC芯片是PCF8591,
*					光敏电阻：light dependent resistor => 缩写：LDR,
*					Lux:勒克斯（照明单位）,标称光亮度的光束均匀射在
*					l m2 面积上时的照度.
**********************************************************/
float LDR_Lux(uint8_t ch)				 
{
	uint8_t adcVale;
	uint32_t resTemp;
	float LDR_Lux;

	PCF8591_ReadChannel(&adcVale, ch);

	resTemp=LDR_CalculateResValue(adcVale);//电阻值，单位：欧姆

	LDR_Lux = pow(10, ((log10(15) - log10(resTemp/1000.0) + 0.6) / 0.6));//这里的电阻值单位为千欧姆
#if 0	
	printf("adcVale = %d\r\n",adcVale);
	printf("Lumen = %f Lux\r\n", LDR_Lux);
	printf("LDR = %dΩ\r\n",resTemp);
#endif /*  */
	return LDR_Lux;	
}

