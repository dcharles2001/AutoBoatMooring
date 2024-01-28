#include "usart.h"

void init_USART(void)
{
	RCC->APB1ENR1 |= (RCC_APB1ENR1_USART2EN); //enable USART 2 clock
  //RCC->AHB2ENR  |= (RCC_AHB2ENR_GPIOAEN); //enable GPIO A clock
  // Configure pins A2, A15 for USART2
  USART_PORT->MODER    &=~( ( 3u << ( 2 * 2 ) ) |  //clear pin 2
                       ( 3u << ( 15 * 2 ) ) ); //clear pin 15
	
  USART_PORT->MODER    |= ( ( 2u << ( 2 * 2 ) ) | //2u (bin10 Alt func) pin 2
                       ( 2u << ( 15 * 2 ) ) ); //2u (bin10 Alt func) pin 15
	
  USART_PORT->OTYPER   &=~( ( 1u << 2 ) | //p-p pin 2
                       ( 1u << 15 ) ); //p-p pin 15 
	
  USART_PORT->OSPEEDR  &=~( ( 3u << ( 2 * 2 ) ) | //clear pin 2 
                       ( 3u << ( 15 * 2 ) ) ); //clear pin 15
	
  USART_PORT->OSPEEDR  |= ( ( 2u << ( 2 * 2 ) ) | //2u (bin10 high speed) pin 2
                       ( 2u << ( 15 * 2 ) ) ); //2u (bin10 high speed) pin 15
	
  USART_PORT->AFR[ 0 ] &=~( ( 15u << ( 2 * 4 ) ) ); //clear pin function
  USART_PORT->AFR[ 0 ] |= ( ( 7u << ( 2 * 4 ) ) );  //set USART as alt function for pin 2
  USART_PORT->AFR[ 1 ] &=~( ( 15u << ( ( 15 - 8 ) * 4 ) ) ); //clear pin function
  USART_PORT->AFR[ 1 ] |= ( ( 3u << ( ( 15 - 8 ) * 4 ) ) );	//set USART as alt function for pin 15
	
	USART_MODULE->CR1|=(										//USART CONFIG
			USART_CR1_TE												//transmit enable
			|USART_CR1_RE												//receive enable
			|USART_CR1_UE												//usart main enable bit
				);
	uint16_t div = SystemCoreClock/9600; //calculate value for baud rate register (BRR)
	
	USART_MODULE->BRR = div; //set baud rate
	//set USART control register
	USART_MODULE->CR1|=(										//USART CONFIG
			USART_CR1_TE												//transmit enable
			|USART_CR1_RE												//receive enable
			|USART_CR1_UE												//usart main enable bit
			|USART_CR1_RXNEIE										//RXNE interrupt enable
				);
	
}

char send_USART(unsigned char newchar)
{
	while(!(USART_MODULE->ISR & USART_ISR_TC)); //wait until TX buffer is empty
	USART_MODULE->TDR = newchar;
	return USART_MODULE->TDR;
}



