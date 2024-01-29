#include "zeta.h"


void zetastartup(void)
{
	write_SPI(0x02); //send powerup command
}

unsigned char zetasendchar(unsigned char newchar)
{
	SPI_PORT->ODR &=~ (1u << SPI_NSS); //bring CS low
	SPI_MODULE->DR = 0x31; //start tx command
	
	//figure out if I need to wait for SR flag when attempting subsequent writes
	SPI_MODULE->DR = 0x66; //begin TX write
	SPI_MODULE->DR = newchar; //send new char
	
	SPI_PORT->ODR |= (1u << SPI_NSS);
	
	return newchar;
}

