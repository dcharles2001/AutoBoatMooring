#ifndef CONFIG_H
#define CONFIG_H

#include "mbed.h"

typedef struct  //template struct for SPI devices
{
    PinName MOSI;
    PinName MISO;
    PinName SCLK;
    PinName CS;

} SPIConfig_t;

const SPIConfig_t f429spi1
{
    PB_5, PB_4, PB_3, PA_4
};


#endif
