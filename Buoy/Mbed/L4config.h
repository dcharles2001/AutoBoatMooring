#ifndef CONFIG_H
#define CONFIG_H

#include "mbed.h"
#include "RadioConfig.h"
#include "stdio.h"
#include "si4455_defs.h"

#define BOAT 0
#define LBUOY 1
#define RBUOY 2


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

const SPIConfig_t l432spi1
{
    PA_7, PA_6, PA_5, PA_3
};





#endif
