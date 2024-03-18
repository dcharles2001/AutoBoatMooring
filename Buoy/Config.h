#ifndef CONFIG_H
#define CONFIG_H

#include "stdio.h"

#include "Radio_Config.h"
#include "si4455_defs.h"


//STM32L432KC SPI details
#define USART_MODULE USART2
#define USART_PORT GPIOA
#define BAUDRATE 9600

//STM32L432KC SPI details
#define SPI_MODULE SPI1
#define SPI_PORT GPIOA
//GPIOA
#define SPI_MOSI 7
#define SPI_MISO 6
#define SPI_SCK 5
#define SPI_NSS 3


//GPIOB
#define ZetaGPIO1 7
#define SDN 6 
#define TriggerLine 1 //for triggering a scope at a given point



#endif
