#include <stm32l4xx.h>
#include "usart.h"

int main(void)
{
	SystemCoreClockUpdate(); //should be 16MHz default
	init_USART();
	
	while(1)
	{
		send_USART('a');
	}
	
}
