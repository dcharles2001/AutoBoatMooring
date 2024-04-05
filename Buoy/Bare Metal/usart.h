#ifndef USART_H
#define USART_H

#include <stm32l4xx.h>
#include "Config.h"

void init_USART(void);
char send_USART(char newchar);
void send_array_USART(char *newarray);


#endif

