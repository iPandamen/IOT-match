
#ifndef ADC_H
#define ADC_H
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "stm32f10x.h"

#include "timer.h"


#define ADC_CHANNEL_NUMS 4

//MEMS RANK1~RANK3
#define ADC1_CH10_RANK1		1
#define ADC1_CH11_RANK2		2
#define ADC1_CH12_RANK3		3
#define ADC1_CH13_RANK4		4
#define ADC1_CH14_RANK5		5


extern u16 ADCRawData[ADC_CHANNEL_NUMS];



void ADC_DMA_Init(void) ;



#endif
