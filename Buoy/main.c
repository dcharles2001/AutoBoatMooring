#include <stm32l4xx.h>
#include "usart.h"
#include "spi.h"
#include "zeta.h"

void altstartzeta(void);
void startzeta(void);

int main(void)
{
	SystemCoreClockUpdate(); //should be 16MHz default
	RCC->AHB2ENR  |= (RCC_AHB2ENR_GPIOAEN); //enable GPIO A clock for USART and SPI
	RCC->AHB2ENR 	|= (RCC_AHB2ENR_GPIOBEN); //enable GPIO B clock for SDN and GPIO1
	
	init_USART();

	send_array_USART("Starting\n\r");
	
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
	SendCmds(0x02, readystate); //set ready state
	
	unsigned char fifo_clr[2] = {0x15, 0x03};
	SendCmds(0x02, fifo_clr); //clear fifo 
	
	GetIntStatus(0, 0, 0);
	
	
	Radio_StartRx(); //begin RX mode
	
	
	cmd = 0x77; //read fifo command
	respByteCount = 0x08;
	//other values parameters will remain the same
	
	unsigned char resp[2];
	char fifostring[20];
	
	unsigned char zetaresponse[8];
	char rxstring[50];
	
	
	
	while(1)
	{
		GetIntStatus(0, 0, 0);
		for(int i=0; i<1000000; i++)
		{
			__NOP();
		}
		
		SendCmdGetResp(0x01, &fifo_clr[1], 0x02, resp);
    for(int i=0; i<2; i++)
    {
			sprintf(fifostring, "Fifo: %x", resp[i]); 
			send_array_USART(fifostring);
			send_array_USART("\n\r");
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


