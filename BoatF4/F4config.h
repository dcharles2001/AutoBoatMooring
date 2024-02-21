#ifndef CONFIG_H
#define CONFIG_H

#include "mbed.h"
#include "RadioConfig.h"


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

typedef struct 
{
    const unsigned char *ConfigArray;
    unsigned char ChannelNumber;
    unsigned char PacketLength;
    unsigned char StateAfterPowerUp;
    unsigned int DelayCntAfterReset;

}RadioConfig_t;

const unsigned char Radio_Configuration_Data_Array[] = RADIO_CONFIGURATION_DATA_ARRAY; //define first RADIO_CONFIGURATION_DATA member with correct array



#endif
