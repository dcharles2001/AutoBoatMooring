#include <stm32l4xx.h>
#include "usart.h"
#include "spi.h"


void altstartzeta(void);
void startzeta(void);


int main(void)
{
	SystemCoreClockUpdate(); //should be 16MHz default
	RCC->AHB2ENR  |= (RCC_AHB2ENR_GPIOAEN); //enable GPIO A clock for USART and SPI
	
	init_USART();
	init_SPI();
	
	send_USART('G');
	
	altstartzeta();
	
}

void startzeta(void)
{
	GPIOA->ODR |= (1u << SDN); //bring SDN high briefly
	for(int i = 0; i<250; i++)
	{
			__NOP();
	}
	GPIOA->ODR &=~ (1u << SDN); //bring SDN low again
	
	for(int i = 0; i<50000; i++) //wait a bunch
	{
			__NOP();
	}
	
	write_SPI(0x02);
	uint8_t responsebyte = readandwrite_SPI(0x44); //CTS check
	if(responsebyte == 0xff)
	{
		send_USART('Y');
	}else
	{
		send_USART('N');
	}	
}

void altstartzeta(void)
{
	GPIOA->ODR |= (1u << SDN); //bring SDN high briefly
	for(int i = 0; i<2; i++)
	{
			__NOP();
	}
	
	GPIOA->ODR &=~ (1u << SDN); //bring SDN low again
	uint32_t IDR_read = GPIOA->IDR; //read input data register
	while(!(IDR_read & (1u << 1))) //wait for GPIO1 to go high
	{
		IDR_read = GPIOA->IDR; //read IDR again
	}
	write_SPI(0x02); //write startup command
	
	/*
	while(GPIOA->IDR & (1u << 1)) //wait for GPIO1 to go low 
	{
		IDR_read = GPIOA->IDR; //read IDR again
	}
	*/
	for(int i = 0; i<3000; i++)
	{
			__NOP();
	}
	
	unsigned char responsebyte = readandwrite_SPI(0x44); //CTS check
	if(responsebyte == 0xff)
	{
		send_USART('Y');
	}else
	{
		send_USART('N');
	}	
}
