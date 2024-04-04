#include <stm32l4xx.h>
#include "usart.h"
#include "spi.h"
#include "zeta.h"

void HSIClockConfig(void);
void PLLHSIConfig(void);

int main(void)
{
	
	PLLHSIConfig(); //Apply HSI config
	//HSIClockConfig();
	SystemCoreClockUpdate(); //Update clock value
	RCC->AHB2ENR  |= (RCC_AHB2ENR_GPIOAEN); //enable GPIO A clock for USART and SPI
	RCC->AHB2ENR 	|= (RCC_AHB2ENR_GPIOBEN); //enable GPIO B clock for SDN and GPIO1
	
	//I could do everything on one APB, do later
	
	
	init_USART();

	send_array_USART("Starting\n\r");
	
	char clkstring[30];
	sprintf(clkstring, "Core clock: %d\n\r", SystemCoreClock);
	send_array_USART(clkstring);
	
	SPI_Init();
	SPI_SI4455_Init();
	
	unsigned char parambytescnt = 0x01; //1 param byte
  unsigned char cmd = 0x01; //part info command
  unsigned char respByteCount = 0x0C;
  unsigned char response[16];
    
	char respstring[40];
	
  SendCmdGetResp(parambytescnt, &cmd, respByteCount, response);
	
  for(int i=0; i<respByteCount; i++)
  {
		sprintf(respstring, "Zeta response: %x\n\r", response[i]);
		send_array_USART(respstring);
  }
	
	unsigned char readystate[2] = {0x34, 0x03};
	//SendCmds(0x02, readystate); //set ready state
	
	unsigned char fifo_clr[2] = {0x15, 0x03};
	//SendCmds(0x02, fifo_clr); //clear fifo 
	
	//GetIntStatus(0, 0, 0);
	
	
	unsigned char devstate = 0x33;
  unsigned char state[6];
	/*
  SendCmdGetResp(0x01, &devstate, 0x02, state);
	send_array_USART("State pre RX:\n\r");
	for(int i=0; i<2; i++)
	{
		sprintf(respstring, "%x\n\r", state[i]);
		send_array_USART(respstring);
	}		
	*/
	//Radio_StartRx(); //begin RX mode
	/*
  SendCmdGetResp(0x01, &devstate, 0x02, state);
	send_array_USART("State post RX:\n\r");
	for(int i=0; i<2; i++)
	{
		sprintf(respstring, "%x\n\r", state[i]);
		send_array_USART(respstring);
	}		
	*/
	cmd = 0x66; //read fifo command
	respByteCount = 0x10;
	//other values parameters will remain the same
	
	unsigned char resp[2];
	char fifostring[20];
	
	unsigned char zetaresponse[16];
	char rxstring[50];
	
	unsigned char testarray[10] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
	
	while(1)
	{
		//SendCmds(0x0A, testarray);
		//GetIntStatus(0, 0, 0);
		
		
		/*
		for(int i=0; i<1000000; i++)
		{
			__NOP();
		}
		*/
		/*
		SendCmdGetResp(0x01, &cmd, respByteCount, zetaresponse); //read 8 bytes?
		
		for(int i=0; i<respByteCount; i++)
		{
			//if(zetaresponse[i] == '\0'){break;} //break on null bit
			sprintf(rxstring, "RX: %x", zetaresponse[i]);
			send_array_USART(rxstring);
			send_array_USART("\n\r");
		}
		*/
		
		SendCmdArgs(cmd, 0x01, 0x0A, testarray); //load packet into fifo
		Start_Tx(NULL);
		
		
		SendCmdGetResp(0x01, &devstate, 0x06, state);
		
		send_array_USART("Device state:\n\r");
		for(int i=0; i<6; i++)
		{
			sprintf(respstring, "%x\n\r", state[i]);
			send_array_USART(respstring);
			
		}
		
		for(int i=0; i<1000000; i++)
		{
			__NOP();
		}
		
		
		
		
	}
	
}

void HSIClockConfig()
{
	RCC->CR |= RCC_CR_HSION; //High speed internal clock (16MHz)
	while( !(RCC->CR & RCC_CR_HSIRDY)); //wait for HSI clock ready flag
	RCC->CFGR &=~ RCC_CFGR_SW; //clear system clock switch
	RCC->CFGR |=  RCC_CFGR_SW_HSI; //set system clock switch to HSI clock
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI); //wait until system clock switch statys reads HSI
}

void PLLHSIConfig()
{
		RCC->CR |= RCC_CR_HSION; //High speed internal clock (16MHz)
		while(!(RCC->CR & RCC_CR_HSIRDY)); //wait for HSI clock ready flag
	
		RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN; //power interface clock enable
		PWR->CR1 |= PWR_CR1_VOS; 
	
	
		FLASH->ACR |= FLASH_ACR_ICEN | 
									FLASH_ACR_PRFTEN | 
									FLASH_ACR_DCEN | 
									FLASH_ACR_LATENCY_3WS;
	
	
		RCC->PLLCFGR &=~ (127u << (8)); //PLLN reset (only part of PLLCFGR not 0 by default						
	
		
		// 16/2 * 16/2 = 64 system clock 
		RCC->PLLCFGR |= (2u << 0) | //HSI source
										(1u << 4) | // PLLM primary div /2
										(16u << 8) | //multiply xn
										(1u << 16) | // PLLPEN
										(0u << 17) | // PLLP /7
										(1u << 20) | // PLLQEN
										(0u << 21) | // PLLQ /2
										(1u << 24) | // PLLREN
										(0u << 25); //PLLR /2 
										
																
	  RCC->CFGR |= (0u << 4)| //AHB div /1
								 (5u << 8)| //APB1 div /1
		             (5u << 11); //APB2 div /1
		
		
		RCC->CR |= RCC_CR_PLLON; //turn PLL on 
		while(!(RCC->CR & RCC_CR_PLLRDY)); //wait for PLL 
		
		
		RCC->CFGR &=~ RCC_CFGR_SW; //clear system clock switch 
		RCC->CFGR |= RCC_CFGR_SW_PLL; //set PLL as clock source
		
		while(!(RCC->CFGR & RCC_CFGR_SWS_PLL)); //wait for PLL switch status
	
		
	
		RCC->CR &=~ RCC_CR_MSION; //MSI off
		
	
	
}

