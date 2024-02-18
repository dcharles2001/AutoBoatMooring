#include "ZetaSPI.h"
#include <cstdio>

zetaspi::zetaspi(SPIConfig_t Pins, DigitalOut sdn, DigitalIn gpio1, DigitalIn nirq): spidevice(Pins.MOSI, Pins.MISO, Pins.SCLK), CS(Pins.CS), SDN(sdn), GPIO1(gpio1), nIRQ(nirq)
{
    spidevice.format(8, 0); //8 bits, cpol, cpha 0
    spidevice.frequency(100000);

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
    for(int i=0; i<900; i++)
    {
        __NOP();
    }
    SDN = 0; //bring sdn low
    while(GPIO1 == 0); //wait until GPIO1 goes high
    printf("GPIO1 high\n\r");
    CS = 0;
    spidevice.write(0x02); //send powerup command
    CS = 1;
    while(GPIO1 == 1); //wait until GPIO1 goes low
    printf("GPIO1 low\n\r");
    
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