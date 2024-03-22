#include "timers.h"

// updaterate = clocksrc /((prescaler)(ARR+1))


void init_LPTIM(void) //initialise LPTIM
{
	/*
	//write to configuration register
	RCC->APB1ENR1 |= RCC_APB1ENR1_LPTIM1EN; //enable LPTIM clock
	
	LPTIM1->CFGR |= ( (0u << (0) ) | //CKSEL interna
										(1u << (11) ) | //PRESC prescaler /2
										(0u << (17) ) | //TRIGEN software trigger
										(0u << (22) ) //PRELOAD update immediately
	
	//CFGR 2 not available on L432 
	
	LPTIM1->ARR |= ( ); //Auto reload value
	*/
	
}

void init_TIM2(void)
{
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN; //enable timer 2 clock
	TIM2->DIER |= TIM_DIER_UIE; //timer update interrupt enabled
	
	TIM2->PSC = PSC_polling; //set prescaler
	TIM2->ARR = ARR_polling; //set ARR
	
	
	
	TIM2->CNT = 0;
	
	//NVIC_SetPriority(TIM2_IRQn, 2); //priority level 2 
	//NVIC_EnableIRQ(TIM2_IRQn); //enable timer IRQ from NVIC
	
	TIM2->CR1 |= TIM_CR1_CEN; //enable timer 
	
}
void WaiT2(unsigned int delay_us) //function for creating delay without interrupts
{
  int start = TIM2->CNT;	//read timer counter
	int current_cnt = start;	//set current counter
	while(((current_cnt - start)&(0xffff))<delay_us) //calculate correct timer value and compare with desired delay
	{
		current_cnt = TIM2->CNT; //read timer and update counter
	}
		
}


void delayms_TIM2(int cycles) 
{
	while(cycles>0)
	{
		WaiT2(1000); //1000us delay
		cycles--;
	}
	
}

void TIM2_IRQHandler(void)
{
	if(TIM2->SR & TIM_SR_UIF)
	{
		//do something 
		GPIOB->ODR |= (1u << TriggerLine); //triggerline high to signal interrupt
	}
	TIM2->SR = 0; //reset SR
}

