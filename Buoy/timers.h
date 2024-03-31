#ifndef TIMERS_H
#define TIMERS_H

#include "Config.h"

#define PSC_polling 320 //psc for 1us
#define ARR_polling 0xffff //max for 16 bits

#define PSC_1ms 15 //1MHz scale
#define ARR_1ms 999 //1000 counts for 1ms


void init_LPTIM(void);
void init_TIM2(void);
void WaiT2(unsigned int delay_us);
void delayms_TIM2(int cycles);
void TIM2_IRQHandler(void);

void init_TIM15(void);



#endif

