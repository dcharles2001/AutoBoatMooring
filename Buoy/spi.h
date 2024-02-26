#ifndef SPI_H
#define SPI_H

#include <stm32l4xx.h>

#include "Config.h"
#include "usart.h"

void init_SPI(void);
void write_SPI(uint8_t newchar);
void write_SPI_noCS(uint8_t newchar);
void write_SPIBytes_noCS(uint8_t *newchars, unsigned int bytes);
uint8_t read_SPI_noCS(void);
uint8_t readandwrite_SPI(uint8_t newchar);
uint8_t readandwrite_SPI_noCS(uint8_t newchar);


#endif

