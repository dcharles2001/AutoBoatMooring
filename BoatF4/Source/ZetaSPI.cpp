#include "ZetaSPI.h"

zetaspi::zetaspi(SPIConfig_t Pins): spidevice(Pins.MOSI, Pins.MISO, Pins.SCLK), CS(Pins.CS)
{
    spidevice.format(8, 0); //8 bits, cpol, cpha 0
    spidevice.frequency(1000000);

}

void zetaspi::startup(void)
{
    CS = 0;
    spidevice.write(0x02); //send powerup command
    CS = 1;
}

unsigned char zetaspi::sendchar(unsigned char newchar)
{
    CS = 0;
    spidevice.write(0x31); //start TX command
    spidevice.write(0x66); //begin to write to tx buffer
    spidevice.write(newchar); //send char
    CS = 1;
    return newchar;
}

bool zetaspi::sendchars(unsigned char newchar[], unsigned short count)
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

unsigned char zetaspi::readchar(void)
{
    CS = 0;
    spidevice.write(0x32); //switch to RX mode
    unsigned char receivechar = spidevice.write(0x66);
    CS = 1;
    return receivechar;

}