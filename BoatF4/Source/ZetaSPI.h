#ifndef ZETASPI_H
#define ZETASPI_H

#include "mbed.h"
#include "SPI.h"
#include "config.h"


class zetaspi
{
    public:

        zetaspi(SPIConfig_t Pins);
        void startup(void);
        unsigned char sendchar(unsigned char newchar);
        bool sendchars(unsigned char newchar[], unsigned short count);
        unsigned char readchar(void);
        
    private:

    SPI spidevice;
    DigitalOut CS;

};




#endif
