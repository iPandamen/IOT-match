/**
  ******************************************************************************
  * @file    
  * @author  
  * @version 
  * @date    
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>        
#include <string.h>                     
#include <stdbool.h>
#include "stm32f10x.h"
#include "stm32f10x_spi.h" 
#include "delay.h"
#include "si4332a.h"

 
/**************SI4332AA硬件I/O口定义*****************************/
#define SI4332A_SPI_CS_PIN               GPIO_Pin_4               
#define SI4332A_SPI_CS_GPIO_PORT         GPIOA                    
#define SI4332A_SPI_CS_GPIO_CLK          RCC_APB2Periph_GPIOA
#define SI4332A_SPI                      SPI1
#define SI4332A_SPI_CLK                  RCC_APB2Periph_SPI1

#define SI4332A_SPI_SCK_PIN              GPIO_Pin_5                 
#define SI4332A_SPI_SCK_GPIO_PORT        GPIOA                   
#define SI4332A_SPI_SCK_GPIO_CLK         RCC_APB2Periph_GPIOA

#define SI4332A_SPI_MISO_PIN             GPIO_Pin_6                 
#define SI4332A_SPI_MISO_GPIO_PORT       GPIOA                       
#define SI4332A_SPI_MISO_GPIO_CLK        RCC_APB2Periph_GPIOA

#define SI4332A_SPI_MOSI_PIN             GPIO_Pin_7                 
#define SI4332A_SPI_MOSI_GPIO_PORT       GPIOA                       
#define SI4332A_SPI_MOSI_GPIO_CLK        RCC_APB2Periph_GPIOA

#define SI4332A_INT_PIN					GPIO_Pin_3
#define SI4332A_INT_GPIO_PORT			GPIOA
#define SI4332A_INT_GPIO_CLK			RCC_APB2Periph_GPIOA

#define SI4332A_SDN_GPIO_PORT			GPIOA
#define SI4332A_SDN_GPIO_PIN			GPIO_Pin_2
#define SI4332A_SDN_GPIO_CLK			RCC_APB2Periph_GPIOA

/*
**SI4332A 接口输出\输入定义
*/
#define SI4332A_CS(x)	if(x) GPIO_SetBits(SI4332A_SPI_CS_GPIO_PORT, SI4332A_SPI_CS_PIN);\
						else GPIO_ResetBits(SI4332A_SPI_CS_GPIO_PORT, SI4332A_SPI_CS_PIN)

#define SI4332A_INT_READ() GPIO_ReadInputDataBit(SI4332A_INT_GPIO_PORT, SI4332A_INT_PIN)

#define SPI_ReadWrite      SI4332A_SPI_SendByte
#define SPI_Read()         SPI_ReadWrite(0x00)
#define SPI_Write(spi_val) SPI_ReadWrite(spi_val)
#define SPI_BEGIN()
#define SPI_END()

//常数宏定义
#define SI4332A_WRITE       0x80
#define SI4332A_READ        0x00


/**
  * @brief  DeInitializes the peripherals.
  * @param  None
  * @retval None
  */
void SI4332A_LowLevel_DeInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /*!< Disable the SI4332A_SPI  */
  SPI_Cmd(SI4332A_SPI, DISABLE);
  
  /*!< DeInitializes the SI4332A_SPI */
  SPI_I2S_DeInit(SI4332A_SPI);
  
  /*!< SI4332A_SPI Periph clock disable */
  RCC_APB2PeriphClockCmd(SI4332A_SPI_CLK, DISABLE);
  
  /*!< Configure SI4332A_SPI pins: SCK */
  GPIO_InitStructure.GPIO_Pin = SI4332A_SPI_SCK_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(SI4332A_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure SI4332A_SPI pins: MISO */
  GPIO_InitStructure.GPIO_Pin = SI4332A_SPI_MISO_PIN;
  GPIO_Init(SI4332A_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure SI4332A_SPI pins: MOSI */
  GPIO_InitStructure.GPIO_Pin = SI4332A_SPI_MOSI_PIN;
  GPIO_Init(SI4332A_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure SI4332A_SPI_CS_PIN pin: SI4332A Card CS pin */
  GPIO_InitStructure.GPIO_Pin = SI4332A_SPI_CS_PIN;
  GPIO_Init(SI4332A_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
}

/**
  * @brief  Initializes the peripherals.
  * @param  None
  * @retval None
  */
void SI4332A_LowLevel_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /*!< SI4332A_SPI_CS_GPIO, SI4332A_SPI_MOSI_GPIO, SI4332A_SPI_MISO_GPIO 
       and SI4332A_SPI_SCK_GPIO Periph clock enable */
  RCC_APB2PeriphClockCmd(SI4332A_SPI_CS_GPIO_CLK | SI4332A_SPI_MOSI_GPIO_CLK | SI4332A_SPI_MISO_GPIO_CLK | SI4332A_SPI_SCK_GPIO_CLK, ENABLE);

  /*!< SI4332A_SPI Periph clock enable */
  RCC_APB2PeriphClockCmd(SI4332A_SPI_CLK, ENABLE);
  
  /*!< Configure SI4332A_SPI pins: SCK */
  GPIO_InitStructure.GPIO_Pin = SI4332A_SPI_SCK_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(SI4332A_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure SI4332A_SPI pins: MOSI */
  GPIO_InitStructure.GPIO_Pin = SI4332A_SPI_MOSI_PIN;
  GPIO_Init(SI4332A_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure SI4332A_SPI pins: MISO */
  GPIO_InitStructure.GPIO_Pin = SI4332A_SPI_MISO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
  GPIO_Init(SI4332A_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);
  
  /*!< Configure SI4332A_SPI_CS_PIN pin: SI4332A Card CS pin */
  GPIO_InitStructure.GPIO_Pin = SI4332A_SPI_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(SI4332A_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
}

/**
  * @brief  DeInitializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void SI4332A_DeInit(void)
{
  SI4332A_LowLevel_DeInit();
}

/**
  * @brief  Initializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void SI4332A_SPI_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;

  SI4332A_LowLevel_Init();
    
  /*!< Deselect the FLASH: Chip Select high */
  SI4332A_CS(1);

  /*!< SPI configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = /*SPI_CPOL_Low*/SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
#if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL)
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
#else
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
#endif

  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SI4332A_SPI, &SPI_InitStructure);

  /*!< Enable the SI4332A_SPI  */
  SPI_Cmd(SI4332A_SPI, ENABLE);
}

/**
  * @brief  
  * @param  None
  * @retval None
  */
static void spi_Init(void)
{
	SI4332A_SPI_Init();
}	 

/**
  * @brief  Sends a byte through the SPI interface and return the byte received
  *         from the SPI bus.
  * @param  byte: byte to send.
  * @retval The value of the received byte.
  */
uint8_t SI4332A_SPI_SendByte(uint8_t byte)
{
  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SI4332A_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(SI4332A_SPI, byte);

  /*!< Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(SI4332A_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /*!< Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SI4332A_SPI);
}

/**
  * @brief  
  * @param  None
  * @retval None
  */
/*********************************************************************************************************
** Function name:           SI4332A_read_register
** Descriptions:            read register
*********************************************************************************************************/
static uint8_t SI4332A_read_register(const uint8_t address)
{
    uint8_t ret;
    SI4332A_CS(0);
    SPI_ReadWrite(address | SI4332A_READ);
    ret = SPI_Read();
    SI4332A_CS(1);
    return ret;
}

/**
  * @brief  
  * @param  None
  * @retval None
  */
/*********************************************************************************************************
** Function name:           SI4332A_read_register_s
** Descriptions:            read registerS
*********************************************************************************************************/
static void SI4332A_read_register_s(const uint8_t address, uint8_t values[], const uint8_t n)
{
    uint8_t i;
    SI4332A_CS(0);
    SPI_ReadWrite(address | SI4332A_READ);
    // mcp2515 has auto-increment of address-pointer
    for (i=0; i<n ; i++)
    {
      values[i] = SPI_Read();
    }
    SI4332A_CS(1);
}

/**
  * @brief  
  * @param  None
  * @retval None
  */
/*********************************************************************************************************
** Function name:           SI4332A_set_register
** Descriptions:            set register
*********************************************************************************************************/
static void SI4332A_set_register(const uint8_t address, const uint8_t value)
{
    SI4332A_CS(0);
    SPI_ReadWrite(address | SI4332A_WRITE);
    SPI_ReadWrite(value);
    SI4332A_CS(1);
}

/**
  * @brief  
  * @param  None
  * @retval None
  */
/*********************************************************************************************************
** Function name:           SI4332A_set_register_s
** Descriptions:            set registerS
*********************************************************************************************************/
static void SI4332A_set_register_s(const uint8_t address, const uint8_t values[], const uint8_t n)
{
    uint8_t i;
    SI4332A_CS(0);
    SPI_ReadWrite(address | SI4332A_WRITE);
    for (i=0; i<n; i++)
    {
      SPI_ReadWrite(values[i]);
    }
    SI4332A_CS(1);
}

/**
  * @brief  
  * @param  None
  * @retval None
  */
static void SI4332A_IRQ_Init(void)
{
 	GPIO_InitTypeDef  GPIO_InitStructure;
 	
 	RCC_APB2PeriphClockCmd(SI4332A_INT_GPIO_CLK, ENABLE);	 //PC5
	
 	GPIO_InitStructure.GPIO_Pin = SI4332A_INT_PIN;	
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //上拉
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(SI4332A_INT_GPIO_PORT, &GPIO_InitStructure);
}

/**
  * @brief  
  * @param  None
  * @retval None
  */
static void SI4332A_SDN_Init(void)
{
 	GPIO_InitTypeDef  GPIO_InitStructure;
 	
 	RCC_APB2PeriphClockCmd(SI4332A_SDN_GPIO_CLK, ENABLE);	 //PA2
	
 	GPIO_InitStructure.GPIO_Pin = SI4332A_SDN_GPIO_PIN;	
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(SI4332A_SDN_GPIO_PORT, &GPIO_InitStructure);

	//Shut down
	GPIO_WriteBit(SI4332A_SDN_GPIO_PORT, SI4332A_SDN_GPIO_PIN, Bit_SET);
	delay_ms(20);
	//NOT shut down
	GPIO_WriteBit(SI4332A_SDN_GPIO_PORT, SI4332A_SDN_GPIO_PIN, Bit_RESET);
	delay_ms(20);
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
 int SI4332A_Init(void)
{
	int ret=0;
	int i;

	SI4332A_SDN_Init();
	
	spi_Init();//SPI_BEGIN();
	SI4332A_IRQ_Init();

	//printf("Initialize the SI4432A ISM chip\r\n");
	
							/* ======================================================== *
							 *				Initialize the SI4432A ISM chip				* 
							 * ======================================================== */

	//read interrupt status registers to clear the interrupt flags and release SI4332A_INT_READ() pin
	SI4332A_read_register(0x03);					//read the Interrupt Status1 register
	SI4332A_read_register(0x04);					//read the Interrupt Status2 register

	//SW reset   
  	SI4332A_set_register(0x07, 0x80);				//write 0x80 to the Operating & Function Control1 register 

	//wait for POR interrupt from the radio (while the nIRQ pin is high)
	i=0;
	while ( SI4332A_INT_READ() == 1)
	{
		delay_us(10);
		if(i++ > 10000)
		{
			//SW reset	 
			SI4332A_set_register(0x07, 0x80);	//write 0x80 to the Operating & Function Control1 register 
			delay_ms(100);
			break;
		}
	}		
	//read interrupt status registers to clear the interrupt flags and release SI4332A_INT_READ() pin
	SI4332A_read_register(0x03);					//read the Interrupt Status1 register
	SI4332A_read_register(0x04);					//read the Interrupt Status2 register
	
	//wait for chip ready interrupt from the radio (while the nIRQ pin is high)
	i=0;
	while ( SI4332A_INT_READ() == 1)
	{
		delay_us(10);
		if(i++ > 10000){break;}
	}		
	//read interrupt status registers to clear the interrupt flags and release SI4332A_INT_READ() pin
	SI4332A_read_register(0x03);					//read the Interrupt Status1 register
	SI4332A_read_register(0x04);					//read the Interrupt Status2 register
					
	/*set tx power,default 0x18
	txpow[2:0] SI4432A Output Power
	000 +1 dBm
	001 +2 dBm
	010 +5 dBm
	011 +8 dBm
	100 +11 dBm
	101 +14 dBm
	110 +17 dBm
	111 +20 dBm
	*/
	SI4332A_set_register(0x6D, 0x18 | 0x05/*0x07*/);
	
	/*set the physical parameters*/
	//set the center frequency to 915 MHz
//	SI4332A_set_register(0x75, 0x75);														//write 0x75 to the Frequency Band Select register             
//	SI4332A_set_register(0x76, 0xBB);														//write 0xBB to the Nominal Carrier Frequency1 register
//	SI4332A_set_register(0x77, 0x80);  													//write 0x80 to the Nominal Carrier Frequency0 register


	//set the center frequency to 434 MHz
	SI4332A_set_register(0x75, 0x53);														//write 0x75 to the Frequency Band Select register             
	SI4332A_set_register(0x76, 0x64);														//write 0xBB to the Nominal Carrier Frequency1 register
	SI4332A_set_register(0x77, 0x00);  													//write 0x80 to the Nominal Carrier Frequency0 register

	SI4332A_set_register(0x79, FHSA);			
	SI4332A_set_register(0x7A, FHCHA);		


	
	//set the desired TX data rate (9.6kbps)
	SI4332A_set_register(0x6E, 0x4E);														//write 0x4E to the TXDataRate 1 register
	SI4332A_set_register(0x6F, 0xA5);														//write 0xA5 to the TXDataRate 0 register
	SI4332A_set_register(0x70, 0x2C);														//write 0x2C to the Modulation Mode Control 1 register


							/*set the modem parameters according to the exel calculator(parameters: 9.6 kbps, deviation: 45 kHz, channel filter BW: 112.1 kHz*/
	SI4332A_set_register(0x1C, 0x05);														//write 0x05 to the IF Filter Bandwidth register		
	SI4332A_set_register(0x20, 0xA1);														//write 0xA1 to the Clock Recovery Oversampling Ratio register		
	SI4332A_set_register(0x21, 0x20);														//write 0x20 to the Clock Recovery Offset 2 register		
	SI4332A_set_register(0x22, 0x4E);														//write 0x4E to the Clock Recovery Offset 1 register		
	SI4332A_set_register(0x23, 0xA5);														//write 0xA5 to the Clock Recovery Offset 0 register		
	SI4332A_set_register(0x24, 0x00);														//write 0x00 to the Clock Recovery Timing Loop Gain 1 register		
	SI4332A_set_register(0x25, 0x13);														//write 0x13 to the Clock Recovery Timing Loop Gain 0 register		
	SI4332A_set_register(0x1D, 0x40);														//write 0x40 to the AFC Loop Gearshift Override register		
	SI4332A_set_register(0x72, 0x1F);														//write 0x1F to the Frequency Deviation register		
										

							/*set the packet structure and the modulation type*/
	//set the preamble length to 10bytes if the antenna diversity is used and set to 5bytes if not 
#ifdef ANTENNA_DIVERSITY
	SI4332A_set_register(0x34, 0x14); 													//write 0x14 to the Preamble Length register 	
#else
	SI4332A_set_register(0x34, 0x0A);														//write 0x0A to the Preamble Length register
#endif
	//set preamble detection threshold to 20bits
	SI4332A_set_register(0x35, 0x28); 													//write 0x28 to the Preamble Detection Control  register

	//Disable header bytes; set variable packet length (the length of the payload is defined by the
	//received packet length field of the packet); set the synch word to two bytes long
	SI4332A_set_register(0x33, 0x02);														//write 0x02 to the Header Control2 register    
	
	//Set the sync word pattern to 0x2DD4
	SI4332A_set_register(0x36, 0x2D);														//write 0x2D to the Sync Word 3 register
	SI4332A_set_register(0x37, 0xD4);														//write 0xD4 to the Sync Word 2 register

	//enable the TX & RX packet handler and CRC-16 (IBM) check
	SI4332A_set_register(0x30, 0x8D);														//write 0x8D to the Data Access Control register
	//Disable the receive header filters
   SI4332A_set_register(0x32, 0x00);														//write 0x00 to the Header Control1 register            
	//enable FIFO mode and GFSK modulation
	SI4332A_set_register(0x71, 0x63);														//write 0x63 to the Modulation Mode Control 2 register

#ifdef ANTENNA_DIVERSITY
	//enable the antenna diversity mode
	SI4332A_set_register(0x08, 0x80);														//write 0x80 to the Operating Function Control 2 register
#endif								 					

											/*set the GPIO's according the testcard type*/
#ifdef ANTENNA_DIVERSITY
  	SI4332A_set_register(0x0C, 0x17);														//write 0x17 to the GPIO1 Configuration(set the Antenna 1 Switch used for antenna diversity )
	SI4332A_set_register(0x0D, 0x18);														//write 0x18 to the GPIO2 Configuration(set the Antenna 2 Switch used for antenna diversity ) 
#endif
#ifdef ONE_SMA_WITH_RF_SWITCH
  	SI4332A_set_register(0x0C, 0x12);														//write 0x12 to the GPIO1 Configuration(set the TX state)
	SI4332A_set_register(0x0D, 0x15);														//write 0x15 to the GPIO2 Configuration(set the RX state) 
#endif
    
											/*set the non-default SI4432A registers*/
	//set the VCO and PLL
	SI4332A_set_register(0x5A, 0x7F);														//write 0x7F to the VCO Current Trimming register 
	SI4332A_set_register(0x58, 0x80);														//write 0x80 to the ChargepumpCurrentTrimmingOverride register
	SI4332A_set_register(0x59, 0x40);														//write 0x40 to the Divider Current Trimming register 	
	//set the AGC
	SI4332A_set_register(0x6A, 0x0B);														//write 0x0B to the AGC Override 2 register
	//set ADC reference voltage to 0.9V
	SI4332A_set_register(0x68, 0x04);														//write 0x04 to the Deltasigma ADC Tuning 2 register
  	SI4332A_set_register(0x1F, 0x03);														//write 0x03 to the Clock Recovery Gearshift Override register
	//set  Crystal Oscillator Load Capacitance register
	SI4332A_set_register(0x09, 0xD7);														//write 0xD7 to the Crystal Oscillator Load Capacitance register

											/*enable receiver chain*/
	SI4332A_set_register(0x07, 0x05);														//write 0x05 to the Operating Function Control 1 register
	//Enable two interrupts: 
	// a) one which shows that a valid packet received: 'ipkval'
	// b) second shows if the packet received with incorrect CRC: 'icrcerror' 
	SI4332A_set_register(0x05, 0x03);														//write 0x03 to the Interrupt Enable 1 register
	SI4332A_set_register(0x06, 0x00);														//write 0x00 to the Interrupt Enable 2 register
	//read interrupt status registers to release all pending interrupts
	SI4332A_read_register(0x03);												//read the Interrupt Status1 register
	SI4332A_read_register(0x04);												//read the Interrupt Status2 register}
	
	return ret;
}

/**********************************************************************************
*	 @brief  send message.
*	 @param  msg: message cache
*	 @param  len: send number of byte
*	 @param  timeout:  
*	 @retval actual send number of byte.
***********************************************************************************/
int SI4332A_Send_Msg(const uint8_t *msg, uint8_t len, int timeout)
{
	int ret=0;
	int i;
	
	//disable the receiver chain (but keep the XTAL running to have shorter TX on time!)
	SI4332A_set_register(0x07, 0x01);			//write 0x01 to the Operating Function Control 1 register			
																	
	//The Tx deviation register has to set according to the deviation before every transmission (+-45kHz)
	SI4332A_set_register(0x72, 0x48);			//write 0x48 to the Frequency Deviation register 
	/*SET THE CONTENT OF THE PACKET*/
	//set the length of the payload to 8bytes	
	SI4332A_set_register(0x3E, len);			//write 8 to the Transmit Packet Length register		
	//fill the payload into the transmit FIFO
	len = len < SI4332A_MAX_SIZE_FIFO ? len : SI4332A_MAX_SIZE_FIFO;
	SI4332A_set_register_s(0x7F, msg, len);		//write to the FIFO Access register	

	//Disable all other interrupts and enable the packet sent interrupt only.
	//This will be used for indicating the successfull packet transmission for the MCU
	SI4332A_set_register(0x05, 0x04);				//write 0x04 to the Interrupt Enable 1 register	
	SI4332A_set_register(0x06, 0x00);				//write 0x00 to the Interrupt Enable 2 register	
	//Read interrupt status regsiters. It clear all pending interrupts and the nIRQ pin goes back to high.
	SI4332A_read_register(0x03);					//read the Interrupt Status1 register
	SI4332A_read_register(0x04);					//read the Interrupt Status2 register

	/*enable transmitter*/
	//The radio forms the packet and send it automatically.
	SI4332A_set_register(0x07, 0x09);												//write 0x09 to the Operating Function Control 1 register

	/*wait for the packet sent interrupt*/
	//The MCU just needs to wait for the 'ipksent' interrupt.
	i=0;
	while(SI4332A_INT_READ() == 1)
	{
		delay_us(10);
		if(i++ > 10000){break;}
	}
	//read interrupt status registers to release the interrupt flags
	SI4332A_read_register(0x03);										//read the Interrupt Status1 register
	SI4332A_read_register(0x04);										//read the Interrupt Status2 register
	
	//after packet transmission set the interrupt enable bits according receiving mode
	//Enable two interrupts: 
	// a) one which shows that a valid packet received: 'ipkval'
	// b) second shows if the packet received with incorrect CRC: 'icrcerror' 
	SI4332A_set_register(0x05, 0x03); 											//write 0x03 to the Interrupt Enable 1 register
	SI4332A_set_register(0x06, 0x00); 											//write 0x00 to the Interrupt Enable 2 register
	//read interrupt status registers to release all pending interrupts
	SI4332A_read_register(0x03);										//read the Interrupt Status1 register
	SI4332A_read_register(0x04);										//read the Interrupt Status2 register
	//set the Frequency Deviation register according to the AFC limiter												
	SI4332A_set_register(0x72, 0x1F);												//write 0x1F to the Frequency Deviation register		

	/*enable receiver chain again*/
	SI4332A_set_register(0x07, 0x05);												//write 0x05 to the Operating Function Control 1 register
	
	ret = len;
	return ret;
}

/*
 *
 * @brief  read message from fifo.
 * @param  msg: message cache
 * @param  len: read number of byte
 * @param  timeout:  
 * @retval >0: actual read number of byte.
 * @retval =0: no data recived.
 * @retval -1: CRC error.
 */
int SI4332A_Read_Msg(uint8_t *msg, uint8_t len, int timeout)
{
	uint8_t ItStatus1;	
	uint8_t length;
	int ret=0;
	
	//wait for the interrupt event
	//If it occurs, then it means a packet received or CRC error happened
	if( SI4332A_INT_READ() == 0 )
	{
		//disable the receiver chain 
		SI4332A_set_register(0x07, 0x01);												//write 0x01 to the Operating Function Control 1 register
		//read interrupt status registers 
		ItStatus1 = SI4332A_read_register(0x03);										//read the Interrupt Status1 register
		SI4332A_read_register(0x04);										//read the Interrupt Status2 register

		/*CRC Error interrupt occured*/
		if( (ItStatus1 & 0x01) == 0x01 )
		{
			//reset the RX FIFO
				SI4332A_set_register(0x08, 0x02);											//write 0x02 to the Operating Function Control 2 register
				SI4332A_set_register(0x08, 0x00);											//write 0x00 to the Operating Function Control 2 register
				ret = -1;
		}

		/*packet received interrupt occured*/
		if( (ItStatus1 & 0x02) == 0x02 )
		{
			//Read the length of the received payload
			length = SI4332A_read_register(0x4B);										//read the Received Packet Length register
			ret = length;
			
			//Get the reeived payload from the RX FIFO
			SI4332A_read_register_s(0x7F, msg, length < len ? length : len);							//read the FIFO Access register
		}
		//reset the RX FIFO
		SI4332A_set_register(0x08, 0x02);												//write 0x02 to the Operating Function Control 2 register
		SI4332A_set_register(0x08, 0x00);												//write 0x00 to the Operating Function Control 2 register
		//enable the receiver chain again
		SI4332A_set_register(0x07, 0x05);												//write 0x05 to the Operating Function Control 1 register
	} 
	return ret;
}
/********************************* END OF FILE *********************************/

