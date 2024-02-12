#include "mbed.h"
#include "ZetaSPI.h"

zetaspi Zeta433(f429spi1, PC_7, PA_15);

int main()
{
    Zeta433.altstartup();
    
    
    while(1)
    {
        unsigned char response = Zeta433.readandwrite(0x44); //check CTS byte
        printf("Zeta response: %x\n\r", response);
        ThisThread::sleep_for(1000);
    }
}

