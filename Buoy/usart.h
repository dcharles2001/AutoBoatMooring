#ifndef USART_H
#define USART_H

#include <stm32l4xx.h>

#define USART_MODULE USART2
#define USART_PORT GPIOA
#define BAUDRATE 9600



void init_USART(void);
char send_USART(unsigned char newchar);



#endif

