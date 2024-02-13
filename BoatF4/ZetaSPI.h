#ifndef ZETASPI_H
#define ZETASPI_H

//#include "mbed.h"
#include "SPI.h"
#include "L4config.h"


class zetaspi
{
    public:

        zetaspi(SPIConfig_t Pins, DigitalOut sdn, DigitalIn gpio1);
        void startup(void);
        void altstartup(void);
        unsigned char sendcharTX(unsigned char newchar);
        //bool sendcharsTX(unsigned char newchar[], unsigned short count);
        unsigned char readcharRX(void);
        unsigned char readandwrite(unsigned char newchar);
        
    private:

        SPI spidevice;
        DigitalOut CS;
        DigitalOut SDN;
        DigitalIn GPIO1;


};




#endif
