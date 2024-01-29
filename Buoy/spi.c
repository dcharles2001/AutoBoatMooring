#include "spi.h"

void init_SPI(void)
{
	RCC->APB2ENR|=RCC_APB2ENR_SPI1EN; //enable SPI1 clock

	//configure GPIO for SPI
	//MOSI: 7
	//MISO: 6
	//SCK: 5
	//NSS: 4
	
	//clear and set alternate function mode for GPIO SPI pins
	
	SPI_PORT->MODER   &=~( ( 3u << ( SPI_MOSI * 2 ) ) |  //clear pin 7
                         ( 3u << ( SPI_MISO * 2 ) ) |  //clear pin 6
											   ( 3u << ( SPI_SCK * 2 ) ) |  //clear pin 5
											   ( 3u << ( SPI_NSS * 2 ) ) );	//clear pin 4
	
  SPI_PORT->MODER   |=(  ( 2u << ( SPI_MOSI * 2 ) ) |  //alt func mode pin 7
                         ( 2u << ( SPI_MISO * 2 ) ) |  //alt func mode pin 6
											   ( 2u << ( SPI_SCK * 2 ) ) |  //alt func mode pin 5
											   ( 1u << ( SPI_NSS * 2 ) ) );	//output mode pin 4
	
	SPI_PORT->OSPEEDR |=(  ( 3u << ( SPI_MOSI * 2 ) ) | //v high speed pin 7
												 ( 3u << ( SPI_MISO * 2 ) ) | //v high speed pin 6
												 ( 3u << ( SPI_SCK * 2 ) ) | //v high speed pin 5
												 ( 3u << ( SPI_NSS * 2 ) ) ); //v high speed pin 4
	
	//configure AFR register for SPI pins
	
	SPI_PORT->AFR[0]  &=~( ( 15u << ( SPI_MOSI * 4) ) |  //clear pin (7) function
												 ( 15u << ( SPI_MISO * 4) ) |	//clear pin (6) function
												 ( 15u << ( SPI_SCK * 4) ) ); //clear pin (5) function
												 
	
	SPI_PORT->AFR[0]  |=(  ( 5u << ( SPI_MOSI * 4 ) ) | //AF5 on pin 7
												 ( 5u << ( SPI_MISO * 4 ) ) | //AF5 on pin 6
												 ( 5u << ( SPI_SCK * 4 ) ) ); //AF5 on pin 5
	
	SPI_MODULE->CR1 |=( ( 0u << ( 0 ) ) | //CPHA 0
										 ( 0u << ( 1 ) ) | //CPOL 1
										 ( 1u << ( 2 ) ) | //Master config
										 ( 7u << ( 3 ) ) | //baud rate fpclk/256
										 ( 1u << ( 6 ) ) | //SPI peripheral enabled
										 ( 0u << ( 7 ) ) | //MSB first
										 ( 1u << ( 8 ) ) | //SSI
										 ( 1u << ( 9 ) ) | //force SSM
										 ( 0u << ( 10 ) ) | //full duplex
										 ( 0u << ( 11 ) ) ); //8 bit CRC length
										 
}

void write_SPI(uint8_t data)
{
    SPI_PORT->ODR &= ~(1u << SPI_NSS); //Bring CS low
    SPI_MODULE->DR = data; //Write to SPI data register
    while (!(SPI_MODULE->SR & (1 << 1))); //Wait until TX buffer is free
    SPI_PORT->ODR |= (1u << SPI_NSS); //Bring CS high again
} 


