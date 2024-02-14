#include "mbed.h"
#include "ZetaSPI.h"

zetaspi Zeta433(f429spi1, PC_7, PA_15);

int main()
{
    printf("Starting\n\r");
    Zeta433.altstartup();
    unsigned char response = Zeta433.readandwrite(0x44); //check CTS byte
    printf("Zeta response: %x\n\r", response);
    
    while(1)
    {
        printf("Sending test char X\n\r");
        Zeta433.sendcharTX('X'); //send test char
        unsigned char response = Zeta433.readandwrite(0x44); //check CTS byte
        printf("Zeta response: %x\n\r", response);
        ThisThread::sleep_for(1000);
    }
}

