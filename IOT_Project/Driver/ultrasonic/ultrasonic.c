/**
  ******************************************************************************
  * @file    TIM/InputCapture/main.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <stdio.h>
#include "ultrasonic.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup TIM_Input_Capture
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_ICInitTypeDef  TIM_ICInitStructure;

__IO uint16_t IC4ReadValue1 = 0, IC4ReadValue2 = 0;
__IO uint16_t CaptureNumber = 0;
__IO uint32_t Capture = 0;
__IO uint32_t TIM4_ultrasonic_dist = 0;

ULTRASONIC_CALLBACK_FUNC ult_cb = NULL;


#define ULTRASONIC_DEBOUNCE_MM		2  //2mm for debounce
#define ABS(_x, _y) (((_x) > (_y)) ? ((_x) -(_y)) : ((_y) -(_x)))

/* Private function prototypes -----------------------------------------------*/
void TIM4_ch3_RCC_Configuration(void);
void TIM4_ch3_GPIO_Configuration(void);
void TIM4_ch3_NVIC_Configuration(void);
void TIM4_ch3_start_capture_Rising(void);
void TIM4_ch3_start_capture_Falling(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
void TIM4_ch3_input_capture_ultrasonic_init(ULTRASONIC_CALLBACK_FUNC ult_update_notify_cb)
{
#if 0

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	/* Enable the TIM3 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	/* Time base configuration */
	//TIM_TimeBaseStructure.TIM_Period = (SystemCoreClock) / (7200*2) -1;
	//TIM_TimeBaseStructure.TIM_Prescaler = 0;
	
	TIM_TimeBaseStructure.TIM_Period = (1200) / (100*2) -1;
	TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 1200) - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	
	TIM_Cmd(TIM4, ENABLE);

#endif
	

  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */     
       
  /* System Clocks Configuration */
  TIM4_ch3_RCC_Configuration();

  /* NVIC configuration */
  TIM4_ch3_NVIC_Configuration();

  /* Configure the GPIO ports */
  TIM4_ch3_GPIO_Configuration();

  /* TIM3 configuration: Input Capture mode ---------------------
     The external signal is connected to TIM3 CH2 pin (PA.07)  
     The Rising edge is used as active edge,
     The TIM3 CCR2 is used to compute the frequency value 
  ------------------------------------------------------------ */
  
  TIM_PrescalerConfig(TIM4, (SystemCoreClock / TIM4_SRC - 1), TIM_PSCReloadMode_Immediate);

  TIM4_ch3_start_capture_Rising();

  if(ult_update_notify_cb)
  	ult_cb = ult_update_notify_cb;

#if 0
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_BothEdge;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x0;

  TIM_ICInit(TIM3, &TIM_ICInitStructure);
  
  /* TIM enable counter */
  TIM_Cmd(TIM3, ENABLE);

  /* Enable the CC2 Interrupt Request */
  TIM_ITConfig(TIM3, TIM_IT_CC2, ENABLE);
#endif
}

void TIM4_ch3_start_capture_Rising(void)
{
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	
	TIM_ICInit(TIM4, &TIM_ICInitStructure);
	
	/* TIM enable counter */
	TIM_Cmd(TIM4, ENABLE);
	
	/* Enable the CC2 Interrupt Request */
	TIM_ITConfig(TIM4, TIM_IT_CC3, ENABLE);

}

void TIM4_ch3_start_capture_Falling(void)
{
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	
	TIM_ICInit(TIM4, &TIM_ICInitStructure);
	
	/* TIM enable counter */
	TIM_Cmd(TIM4, ENABLE);
	
	/* Enable the CC2 Interrupt Request */
	TIM_ITConfig(TIM4, TIM_IT_CC3, ENABLE);

}


/**
  * @brief  Configures the different system clocks.
  * @param  None
  * @retval None
  */
void TIM4_ch3_RCC_Configuration(void)
{
  /* TIM4 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

  /* GPIOB clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
}

/**
  * @brief  Configure the GPIOD Pins.
  * @param  None
  * @retval None
  */
void TIM4_ch3_GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* TIM4 channel 3 pin (PB.08) configuration: ultrasonic input capture */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*(PB.09) configuration: ultrasonic enable */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIOB, &GPIO_InitStructure);

  //ultrasonic enable
  GPIO_ResetBits(GPIOB, GPIO_Pin_9);
}

/**
  * @brief  Configure the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void TIM4_ch3_NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the TIM4 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

//get distance which tested by ualtrasonic 
uint16_t read_ultrasonic_dist(void)
{
	return TIM4_ultrasonic_dist;
}

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/
/**
  * @brief  This function handles TIM4 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM4_IRQHandler(void)
{ 
  uint32_t TIM4_ultrasonic_dist_tmp = 0;
  
  if(TIM_GetITStatus(TIM4, TIM_IT_CC3) == SET) 
  {
    /* Clear TIM4 Capture compare interrupt pending bit */
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC3);
    if(CaptureNumber == 0)
    {
      /* Get the Input Capture value */
      IC4ReadValue1 = TIM_GetCapture3(TIM4);
	  //printf("v1:%d\r\n", IC4ReadValue1);
      CaptureNumber = 1;

	  TIM4_ch3_start_capture_Falling();
    }
    else if(CaptureNumber == 1)
    {
      /* Get the Input Capture value */
      IC4ReadValue2 = TIM_GetCapture3(TIM4); 
	  //printf("v2:%d\r\n", IC4ReadValue2);
      
      /* Capture computation */
      if (IC4ReadValue2 > IC4ReadValue1)
      {
        Capture = (IC4ReadValue2 - IC4ReadValue1); 
      }
      else
      {
        Capture = ((0xFFFF - IC4ReadValue1) + IC4ReadValue2); 
      }

		/* V = 331.5 + 0.607T
		   D = V*T/2
		   (T = 25, V = 346.675m/s = 0.346675mm/us) */

	  TIM4_ultrasonic_dist_tmp = Capture * 346675 / TIM4_SRC / 2;
	  if(ABS(TIM4_ultrasonic_dist_tmp, TIM4_ultrasonic_dist) > ULTRASONIC_DEBOUNCE_MM){
	  	TIM4_ultrasonic_dist = TIM4_ultrasonic_dist_tmp;
	  	 if(ult_cb)
		 	(*ult_cb)(TIM4_ultrasonic_dist);
	  }
      CaptureNumber = 0;

	  //start input capture
	  TIM4_ch3_start_capture_Rising();
    }
  }
	  
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  while (1)
  {}
}

#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
