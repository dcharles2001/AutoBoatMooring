#ifndef ZETASPI_H
#define ZETASPI_H

//#include "mbed.h"
#include "SPI.h"
#include "L4config.h"


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

        SPI spidevice;
        DigitalOut CS;
        DigitalOut SDN;
        DigitalIn GPIO1;
        DigitalIn nIRQ; 

        char RF_POWER_UP[7] = {0x02, 0x01, 0x00, 0x01, 0xC9, 0xC3, 0x80}; //power up command and parameter bytes
        char RF_INT_CTL_ENABLE_1[5] = {0x11, 0x01, 0x01, 0x00, 0x00};
        char RF_FRR_CTL_A_MODE_4[8] = {0x11, 0x02, 0x04, 0x00, 0x08, 0x06, 0x04, 0x0A};
        char RF_EZCONFIG_XO_TUNE_1[5] = {0x11, 0x24, 0x01, 0x03, 0x52};
        char RF_GPIO_PIN_CFG[8] = {0x13, 0x00, 0x00, 0x01, 0x00, 0x00, 0x0B, 0x00};
};




#endif
