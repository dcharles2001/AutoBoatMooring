#include "spi.h"

void init_SPI(void)
{
	RCC->APB2ENR  |= (RCC_APB2ENR_SPI1EN); //enable SPI1 clock 
	//RCC->APB1ENR1|=RCC_APB1ENR1_SPI3EN; //enable SPI3 clock

	
	//clear and set alternate function mode for GPIO SPI pins
	
	SPI_PORT->MODER   &=~( ( 3u << ( SPI_MOSI * 2 ) ) |  //clear pin 7
                         ( 3u << ( SPI_MISO * 2 ) ) |  //clear pin 6
											   ( 3u << ( SPI_SCK * 2 ) ) );  //clear pin 5
											   
	
  SPI_PORT->MODER   |=(  ( 2u << ( SPI_MOSI * 2 ) ) |  //alt func mode pin 7
                         ( 2u << ( SPI_MISO * 2 ) ) |  //alt func mode pin 6
											   ( 2u << ( SPI_SCK * 2 ) ) );  //alt func mode pin 5
											   
								 
	//SPI_PORT->OTYPER  |=(  ( 1u << ( SPI_MISO ) ) );  //open drain pin 7
                         
	
	//SPI_PORT->PUPDR   &=~ ( 3u << ( SPI_MISO * 2 ) );  //clear pin 6
	//SPI_PORT->PUPDR   |= ( (3u << ( SPI_MISO * 2 ) ) ); //Pull-down on pin 6
	
	/*
	SPI_PORT->OSPEEDR |=(  ( 2u << ( SPI_MOSI * 2 ) ) | //high speed pin 7
												 ( 2u << ( SPI_MISO * 2 ) ) | //high speed pin 6
												 ( 2u << ( SPI_SCK * 2 ) ) | //high speed pin 5
												 ( 2u << ( SPI_NSS * 2 ) ) ); //high speed pin 4
	*/
	//SPI_PORT->ODR |= (1u << SPI_NSS); //CS sits high initially
	/*
	GPIOB->MODER &=~ ( ( 3u << ( SDN * 2 ) ) | //clear pin 3
										 ( 3u << ( ZetaGPIO1 * 2 ) ) ); //clear pin 1
	*/						 
	//GPIOA->PUPDR &=~ ( 3u << (ZetaGPIO1 * 2 ) ); //clear pin 1 pupdr
	//GPIOA->PUPDR |=  ( 2u << (ZetaGPIO1 * 2 ) ); //pull down on pin 1
	/*									 
	GPIOB->MODER |= ( ( 1u << ( SDN * 2 ) ) | //output mode pin 3
										( 0u << ( ZetaGPIO1 * 2 ) ) ); //input mode pin 1
		*/								 
	//GPIOB->ODR &=~ (1u << SDN); //default low
	
	//configure AFR register for SPI pins
	
	SPI_PORT->AFR[0]  &=~( ( 15u << ( SPI_MOSI * 4) ) |  //clear pin (7) function
												 ( 15u << ( SPI_MISO * 4) ) |	//clear pin (6) function
												 ( 15u << ( SPI_SCK * 4) ) ); //clear pin (5) function
												 
												 
	
	SPI_PORT->AFR[0]  |=(  ( 5u << ( SPI_MOSI * 4 ) ) | //AF5 on pin 7
												 ( 5u << ( SPI_MISO * 4 ) ) | //AF5 on pin 6
												 ( 5u << ( SPI_SCK * 4 ) ) ); //AF5 on pin 5
												
	
	/*
	SPI_PORT->AFR[0]  &=~( ( 15u << ( SPI_MOSI * 4) ) |  //clear pin function
												 ( 15u << ( SPI_MISO * 4) ) |	//clear pin function
												 ( 15u << ( SPI_SCK * 4) )  ); //clear pin function
												
												 
	
	SPI_PORT->AFR[0]  |=(  ( 6u << ( SPI_MOSI * 4 ) ) | //AF6 on pin 
												 ( 6u << ( SPI_MISO * 4 ) ) | //AF6 on pin 
												 ( 6u << ( SPI_SCK * 4 ) ) ); //AF6 on pin 
	*/									 
												 
	SPI_MODULE->CR1 |=( ( 0u << ( 0 ) ) | //CPHA 0
										  ( 0u << ( 1 ) ) | //CPOL 0
										  ( 1u << ( 2 ) ) | //Master config
										  ( 0u << ( 3 ) ) | //baud rate fpclk/n
										  //( 1u << ( 6 ) ) | //SPI peripheral enabled
										  ( 0u << ( 7 ) ) | //MSB first
										  ( 1u << ( 8 ) ) | //SSI
										  ( 1u << ( 9 ) ) | //force SSM
										  ( 0u << ( 10 ) ) | //full duplex
										  ( 0u << ( 11 ) )  | //8 bit CRC length
											( 0u << ( 15 ) ) ); //Bidirectional mode									
										 
	SPI_MODULE->CR2 |=( ( 0u << ( 0 ) ) | //RX buffer DMA 
											( 0u << ( 1 ) ) | //TX buffer DMA 
											( 0u << ( 2 ) ) | //SS output disable
											( 0u << ( 3 ) ) | //NSS pulse disable
											( 0u << ( 4 ) ) | //Frame format motorola mode
											( 0u << ( 5 ) ) | //Error interrupt 
											( 0u << ( 6 ) ) | //RX buffer not empty interrupt 
											( 0u << ( 7 ) ) | //TX buffer empty interrupt 
											( 1u << ( 8 ) ) | //8 bit data size
											( 1u << ( 12 ) ) ); //FRXTH 8 bit FIFO threshold
											
	SPI_MODULE->CR1 |=( 1u << (6) ); //SPI peripheral enable

}

void write_SPI(uint8_t newchar) //write a single byte over SPI and manage CS with software
{
    SPI_PORT->ODR &= ~(1u << SPI_NSS); //Bring CS low
    *(__IO uint8_t*)(&SPI_MODULE->DR) = newchar; //Write to SPI data register (only 8 bits)
    while (!(SPI_MODULE->SR & (1u << 7))); //Wait on busy flag to indicate finish of transmission
    SPI_PORT->ODR |= (1u << SPI_NSS); //Bring CS high again
} 

void write_SPI_noCS(uint8_t newchar) //write a single byte over SPI but don't manipulate CS
{		
	
	  *(__IO uint8_t*)(&SPI_MODULE->DR) = newchar; //Write to SPI data register (only 8 bits)
		while (!(SPI_MODULE->SR & (SPI_SR_TXE))); //Wait on TXE
		while((SPI_MODULE->SR & (1u << 7))); //wait for transmission to end
}

void write_SPIBytes_noCS(uint8_t *newchars, unsigned int bytes)
{
	for(unsigned int i=0; i<bytes; i++)
	{
		*(__IO uint8_t*)(&SPI_MODULE->DR) = *newchars; //Write to SPI data register (only 8 bits)
    while (!(SPI_MODULE->SR & (1u << 1))); //Wait on TXE 
	}
	newchars++;
}

uint8_t read_SPI_noCS(void) //write dummy bytes to keep clock on for SDO data, no CS control
{
	//while (!(SPI_MODULE->SR & (1u << 1))); //Wait
	uint8_t temp = *(__IO uint8_t*)(&SPI_MODULE->DR); //clear buffer
	*(__IO uint8_t*)(&SPI_MODULE->DR) = 0xff; //Dummy bits to keep clock on
	//uint8_t temp = *(__IO uint8_t*)(&SPI_MODULE->DR); //initiate read / clear buffer
	
	//now read result
	while (!(SPI_MODULE->SR & (SPI_SR_RXNE))); //Wait until RXNE is set
	uint8_t response = *(__IO uint8_t*)(&SPI_MODULE->DR); //read new data
	
	//while (!(SPI_MODULE->SR & (1 << 1)));
	
	while((SPI_MODULE->SR & (1u << 7))); //wait for transmission to end
	/*
	for(int i=0; i<1; i++)
	{
		__NOP();
	}
	*/
	return response; 
}


/*
uint8_t readandwrite_SPI(uint8_t newchar)
{
	SPI_PORT->ODR &=~ (1u << SPI_NSS); //Bring CS low
	*(__IO uint8_t*)(&SPI_MODULE->DR) = newchar; //Write to SPI data register
	while (!(SPI_MODULE->SR & (1u << 1))); //Wait until TX buffer is free
	
	//now read RX
	unsigned char temp = SPI_MODULE->DR; //initiate read action
	*(__IO uint8_t*)(&SPI_MODULE->DR) = 0xff; //Dummy bits to keep clock on
	while (!(SPI_MODULE->SR & ( 1u << 0))); //Wait until RX buffer is filled
	SPI_PORT->ODR |= (1u << SPI_NSS); //bring CS high again 
	uint8_t response = *(__IO uint8_t*)(&SPI_MODULE->DR); //read new data
	return response;
}

uint8_t readandwrite_SPI_noCS(uint8_t newchar)
{
	*(__IO uint8_t*)(&SPI_MODULE->DR) = newchar; //Write to SPI data register
	while (!(SPI_MODULE->SR & (1u << 1))); //Wait until TX buffer is free
	
	//now read RX
	unsigned char temp = SPI_MODULE->DR; //initiate read action
	*(__IO uint8_t*)(&SPI_MODULE->DR) = 0xff; //Dummy bits to keep clock on
	while (!(SPI_MODULE->SR & ( 1u << 0))); //Wait until RX buffer is filled
	uint8_t response = *(__IO uint8_t*)(&SPI_MODULE->DR); //read new data
	return response;
}
*/



