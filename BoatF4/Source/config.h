#ifndef CONFIG_H
#define CONFIG_H

#include "mbed.h"

typedef struct  //template struct for SPI devices
{
    PinName MOSI;
    PinName MISO;
    PinName SCLK;
    PinName CS;
    PinName OE;

} SPIConfig_t ;



#endif
