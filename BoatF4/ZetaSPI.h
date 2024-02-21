#ifndef ZETASPI_H
#define ZETASPI_H

//#include "mbed.h"
#include "SPI.h"
#include "F4config.h"
#include "RadioConfig.h"


class zetaspi
{
    public:

        zetaspi(SPIConfig_t Pins, DigitalOut sdn, DigitalIn gpio1, DigitalIn nirq);
        void startup(void);
        void altstartup(void);
        unsigned char sendcharTX(unsigned char newchar);
        //bool sendcharsTX(unsigned char newchar[], unsigned short count);
        unsigned char readcharRX(void);
        unsigned char readandwrite(unsigned char newchar);
        void getpartinfo(unsigned char responsebuff[]);
        void readandwritemultiple(char *tx_buffer, int tx_length, char *rx_buffer, int rx_length);
        
    private:

        void radioconfig(const unsigned char *cmdPTR);

        const RadioConfig_t RadioConfig = RADIO_CONFIGURATION_DATA; //get radio config data from header

        SPI spidevice;
        DigitalOut CS;
        DigitalOut SDN;
        DigitalIn GPIO1;
        DigitalIn nIRQ; 

        
};




#endif
