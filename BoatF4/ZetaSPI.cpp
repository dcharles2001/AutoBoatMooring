#include "ZetaSPI.h"
#include <cstdio>

zetaspi::zetaspi(SPIConfig_t Pins, DigitalOut sdn, DigitalIn gpio1, DigitalIn nirq): spidevice(Pins.MOSI, Pins.MISO, Pins.SCLK), CS(Pins.CS), SDN(sdn), GPIO1(gpio1), nIRQ(nirq)
{
    spidevice.format(8, 0); //8 bits, cpol, cpha 0
    spidevice.frequency(10000);
    CS = 1;

}

void zetaspi::startup(void)
{
    SDN = 1;
    SDN = 0; //bring sdn low
    ThisThread::sleep_for(14); //must wait for POR + max SPI timeout
    CS = 0;
    spidevice.write(0x02); //send powerup command
    CS = 1;
}

void zetaspi::altstartup(void)
{
    SDN = 1;
    for(int i=0; i<550; i++) //approx 10us delay
    {
        __NOP();
    }
    SDN = 0; //bring sdn low
    while(GPIO1 == 0); //wait until GPIO1 goes high
    //printf("GPIO1 high\n\r");
    CS = 0;
    spidevice.write(0x00); // NOP
    CS = 1;

    //ThisThread::sleep_for(14ms); //must wait for POR + max SPI timeout

    //spidevice.write(RF_POWER_UP, 7, &dummyrx, 1); //send powerup command
    radioconfig(RadioConfig.ConfigArray);


    
    
    
}

void zetaspi::radioconfig(volatile const unsigned char *cmdPTR)
{
    while(*cmdPTR != 0x00) //repeat until final entry 0x00 of array is reached
    {
        unsigned int parambytescount = *cmdPTR; //number of command bytes for current command
        *cmdPTR++; //increment pointer to command byte

        if(*cmdPTR == 0x66) //TX fifo write?
        {
            /*
            unsigned char CTS = 0x00; //CTS byte
            while(CTS != 0xff) //repeat until CTS acknowledgement (requires error detection)
            {
                CS = 0;
                CTS = spidevice.write(0x44); //check CTS
                CS = 1;
                
            }
            */
            writemultiple(cmdPTR, parambytescount); //write cmd and param bytes
            cmdPTR += parambytescount; //increment pointer to next cmd byte count
            
            CS = 0;
            spidevice.write(0x00); //NOP
            CS = 1;


        }else {
            
            writemultiple(cmdPTR, parambytescount);
            cmdPTR += parambytescount; //increment pointer to next cmd byte count
            CS = 0;
            spidevice.write(0x00); //NOP
            CS = 1;
            
            /*
            unsigned char CTS = 0x00; //CTS byte
            while(CTS != 0xff) //repeat until CTS acknowledgement (requires error detection)
            {
                CS = 0;
                CTS = spidevice.write(0x44); //check CTS
             CS = 1;
            }
            */

        }

    }
}


unsigned char zetaspi::sendcharTX(unsigned char newchar)
{
    CS = 0;
    spidevice.write(0x66); //begin to write to tx buffer
    spidevice.write(newchar); //send char
    spidevice.write(0x31); //start TX command
    CS = 1;
    return newchar;
}

/*
bool zetaspi::sendcharsTX(unsigned char newchar[], unsigned short count)
{
    if(count > 8)
    {
        return 1; //failure, too many bits
    }
    CS = 0;
    spidevice.write(0x31);
    spidevice.write(0x66);
    for (int i = 0; i<count-1; i++)
    {
        spidevice.write(newchar[i]);
    }
    CS = 1;
    return 0; //success
    
    
}
*/
unsigned char zetaspi::readcharRX(void)
{
    CS = 0;
    spidevice.write(0x32); //switch to RX mode
    unsigned char receivechar = spidevice.write(0x66);
    CS = 1;
    return receivechar;

}

void zetaspi::writemultiple(volatile const unsigned char *arrayPTR, unsigned int arraySize)
{
    CS = 0;
    for(int i = 0; i<arraySize; i++)
    {
        spidevice.write(*arrayPTR);
        arrayPTR++; //increment pointer to next param byte
    }
    CS = 1;
    for(int i=0; i<10; i++) //small delay to satisfy Tsw
    {
        __NOP();
    }
}

unsigned char zetaspi::readandwrite(unsigned char newchar)
{
    CS = 0;
    unsigned char responsebyte = spidevice.write(newchar); //send byte and read response
    CS = 1;
    return responsebyte;
}

void zetaspi::readandwritemultiple(char *tx_buffer, int tx_length, char *rx_buffer, int rx_length)
{
    CS = 0;
    spidevice.write(tx_buffer, tx_length, rx_buffer, rx_length); 
    CS = 1;
}