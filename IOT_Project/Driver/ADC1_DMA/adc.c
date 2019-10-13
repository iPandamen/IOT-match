/**********************************************************
*	@file 		adc.c
*	@author		Panda
*	@version	V0.0.0
*	@date		2019-09
*	@brief		
***********************************************************/

#include "adc.h"

u16 ADCRawData[ADC_CHANNEL_NUMS];


/*******************************************************************************
 * @Function		: ADC_Calibrate
 * @Description		: ADC内部校准
 * @Arguments   	: ADCx：ADC通道
 * @Returns     	: 无
 * @Other       	: 重新对ADC内部校准
 ******************************************************************************/
static void ADC_Calibrate(ADC_TypeDef *ADCx) 
{
	// Enable ADC reset calibration register
	ADC_ResetCalibration(ADCx);
	// Check the end of ADC reset calibration register
	while(ADC_GetResetCalibrationStatus(ADCx)){}
	// Start ADC calibration
	ADC_StartCalibration(ADCx);
	// Check the end of ADC calibration
	while(ADC_GetCalibrationStatus(ADCx)){}
}

/*******************************************************************************
 * @Function		: ADC_DMA_Init
 * @Description		: ADC转换DMA传输初始化函数
 * @Arguments   	: 无
 * @Returns     	: 无
 * @Other       	: 采用DMA传输
 ******************************************************************************/
void ADC_DMA_Init(void) 
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	DMA_InitTypeDef 	DMA_InitStructure;
	ADC_InitTypeDef 	ADC_InitStructure;
//	NVIC_InitTypeDef 	NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);								//DMA1时钟初始化
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);		
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* DMA channel1 configuration */
	DMA_DeInit(DMA1_Channel1);																				//DMA1的通道1
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);	//ADC1_DR_Address;	 //ADC地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADCRawData[0];	//内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					//外设为数据源
	DMA_InitStructure.DMA_BufferSize =ADC_CHANNEL_NUMS;					//保存了DMA要传输的数据总大小，
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//外设地址固定
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  			//内存地址自增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;		//外设数据单位，半字16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;		//存储器数据单位，半字16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;					//循环模式，传输完成后自动装载DMA要传输的数据总大小，
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;			//优先级高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;					//禁止内存到内存的传输
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);											


	/* DMA channel1 interrupt */
//	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;				//中断通道
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;			//抢占优先级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//次优先级
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;							//中断使能
//	NVIC_Init(&NVIC_InitStructure);   
//	
//	DMA_ClearITPendingBit(DMA1_IT_TC1);								//清除中断标志位
//	DMA_ITConfig(DMA1_Channel1,DMA_IT_TC,ENABLE);  					//配置DMA发送完成后产生中断

	DMA_Cmd(DMA1_Channel1, ENABLE);																		/* Enable DMA channel1 */
	
	/* ADC1 configuration */
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;						//独立ADC模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE ; 	 						//扫描模式用于多通道采集
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;						//开启连续转换模式，即不停地进行ADC转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;		//不使用外部触发转换
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 					//采集数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = ADC_CHANNEL_NUMS;					//要转换的通道数
	ADC_Init(ADC1, &ADC_InitStructure);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); 																/*配置ADC时钟，为PCLK2的6分频，即12MHz*/
	
	/*配置ADC1的通道1-5为239.5个采样周期，序列为1 */
	ADC_TempSensorVrefintCmd(ENABLE);																	//开启温度传感器及 Vref 通道
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, ADC1_CH10_RANK1, ADC_SampleTime_239Cycles5 );	//ADC-CH1 SENSE_A
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, ADC1_CH11_RANK2, ADC_SampleTime_239Cycles5 );	//ADC-CH1 SENSE_A
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, ADC1_CH12_RANK3, ADC_SampleTime_239Cycles5 );	//ADC-CH2 SENSE_B
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, ADC1_CH13_RANK4, ADC_SampleTime_239Cycles5 );	//ADC-CH3 SENSE_C

	ADC_DMACmd(ADC1, ENABLE);		// Enable ADC1 DMA
	ADC_Cmd(ADC1, ENABLE);			// Enable ADC1
	
	ADC_Calibrate(ADC1);			//复位校准寄存器  
	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);				// 由于没有采用外部触发，所以使用软件触发ADC转换
}




/*******************************************************************************
 * @Function		: DMA1_Channel1_IRQHandler
 * @Description		: DMA1 通道1 ADC转换中断函数
 * @Arguments   	: 无
 * @Returns     	: 无
 * @Other       	: 
 ******************************************************************************/
void DMA1_Channel1_IRQHandler(void) 
{
	uint8_t i=0;
	if(DMA_GetITStatus(DMA1_IT_TC1) != RESET)
	{
		printf("%f V\r\n",ADCRawData[i] * 3.3/4096);
		delay_ms(50);
		DMA_ClearITPendingBit(DMA1_IT_TC1);
	}
}





/***********************************END OF FILE*********************************/
