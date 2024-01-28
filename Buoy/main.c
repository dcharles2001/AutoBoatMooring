#include <stm32l4xx.h>
#include "usart.h"
#include "spi.h"

int main(void)
{
	SystemCoreClockUpdate(); //should be 16MHz default
	RCC->AHB2ENR  |= (RCC_AHB2ENR_GPIOAEN); //enable GPIO A clock for USART and SPI
	
	init_USART();
	
	while(1)
	{
		send_USART('a');
	}
	
}
