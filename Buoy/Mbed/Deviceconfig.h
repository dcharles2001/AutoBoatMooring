#ifndef CONFIG_H
#define CONFIG_H

#include "mbed.h"
#include "RadioConfig.h"
#include "stdio.h"
#include "si4455_defs.h"

#define BOAT 0
#define LBUOY 1
#define RBUOY 2

//cmds and params
#define OFF 0x00
#define ON 0x01

#define MINTIME 30 //minimum LED on time 


#define RadioFlag (1UL << 0)

#define WaitCycle 2000

typedef struct  //template struct for SPI devices
{
    PinName MOSI;
    PinName MISO;
    PinName SCLK;
    PinName CS;

} SPIConfig_t;

/*
const SPIConfig_t f429spi1
{
    PB_5, PB_4, PB_3, PA_4
};
*/


const SPIConfig_t l432spi1
{
    PA_7, PA_6, PA_5, PA_3
};


typedef struct
{
    PinName SDN;
    PinName GPIO2;
    PinName GPIO1;

}ZetaConfig_t;


const ZetaConfig_t L4Zeta
{
    PB_6,
    PB_7,
    PA_8,
};


/*
const ZetaConfig_t F4Zeta
{
    PC_7,
    PA_15,
    PB_15
};
*/

typedef struct
{
    char cmd;
    char param;
} Buoycmd_t;



#endif
