#include "timers.h"

// updaterate = clocksrc /((prescaler)(ARR+1))


void init_LPTIM(void) //initialise LPTIM
{
	
	//write to configuration register
	RCC->APB1ENR1 |= RCC_APB1ENR1_LPTIM1EN; //enable LPTIM clock
	
	LPTIM1->CFGR |= ( (0u << (0) ) | //CKSEL interna
										(1u << (11) ) | //PRESC prescaler /2
										(0u << (17) ) | //TRIGEN software trigger
										(1u << (22) ) //PRELOAD update immediately
	
	LPTIM1->ARR |= ( ); //Auto reload value
										
}


