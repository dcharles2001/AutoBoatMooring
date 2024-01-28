#ifndef SPI_H
#define SPI_H

#include <stm32l4xx.h>

#define SPI_MODULE SPI1
#define SPI_PORT GPIOA
#define SPI_MOSI 7
#define SPI_MISO 6
#define SPI_SCK 5
#define SPI_NSS 4

void init_SPI(void);
void write_SPI(uint8_t data);

#endif

