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
	
	Radio_StartRx(); //begin RX mode
	cmd = 0x77; //read fifo command
	respByteCount = 0x10;
	//other values parameters will remain the same
	unsigned char zetaresponse[16];
	char rxstring[50];
	while(1)
	{
		SendCmdGetResp(parambytescnt, &cmd, respByteCount, zetaresponse); //read 16 bytes?
		for(int i=0; i<respByteCount; i++)
		{
			//if(zetaresponse[i] == '\0'){break;} //break on null bit
			sprintf(rxstring, "RX: %c", zetaresponse[i]);
			send_array_USART(rxstring);
			send_array_USART("\n\r");
		}
		for(int i=0; i<1000000; i++)
		{
			__NOP();
		}
	}
	
}


