#include <stm32l4xx.h>
#include "usart.h"
#include "spi.h"
#include "zeta.h"

void HSIClockConfig(void);

int main(void)
{
	
	HSIClockConfig();
	SystemCoreClockUpdate(); 
	RCC->AHB2ENR  |= (RCC_AHB2ENR_GPIOAEN); //enable GPIO A clock for USART and SPI
	RCC->AHB2ENR 	|= (RCC_AHB2ENR_GPIOBEN); //enable GPIO B clock for SDN and GPIO1
	
	init_USART();

	send_array_USART("Starting\n\r");
	
	unsigned int clkspeed = SystemCoreClock;
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
  unsigned char state[2];
  SendCmdGetResp(0x01, &devstate, 0x02, state);
	sprintf(respstring, "Device state pre RX:\n\r%x\n\r%x\n\r", state[0], state[1]);
	send_array_USART(respstring);
	
	Radio_StartRx(); //begin RX mode
	
  SendCmdGetResp(0x01, &devstate, 0x02, state);
	sprintf(respstring, "Device state post RX:\n\r%x\n\r%x\n\r", state[0], state[1]);
	send_array_USART(respstring);
	
	cmd = 0x77; //read fifo command
	respByteCount = 0x08;
	//other values parameters will remain the same
	
	unsigned char resp[2];
	char fifostring[20];
	
	unsigned char zetaresponse[8];
	char rxstring[50];
	
	unsigned char testarray[10] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
	
	while(1)
	{
		//SendCmds(0x0A, testarray);
		//GetIntStatus(0, 0, 0);
		
		SendCmdGetResp(0x01, &devstate, 0x02, state);
		sprintf(respstring, "Device state post RX:\n\r%x\n\r%x\n\r", state[0], state[1]);
		send_array_USART(respstring);
		for(int i=0; i<1000000; i++)
		{
			__NOP();
		}
		
		
		SendCmdGetResp(0x01, &cmd, respByteCount, zetaresponse); //read 8 bytes?
		
		
		
		for(int i=0; i<respByteCount; i++)
		{
			//if(zetaresponse[i] == '\0'){break;} //break on null bit
			sprintf(rxstring, "RX: %x", zetaresponse[i]);
			send_array_USART(rxstring);
			send_array_USART("\n\r");
		}
		
	
		
		/*
		for(int i=0; i<1000000; i++)
		{
			__NOP();
		}
		*/
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

