#include "mbed.h"
#include "ZetaSPI.h"

zetaspi Zeta433(f429spi1, PC_7, PA_15, PB_15);

int main()
{
    printf("Starting\n\r");

    Zeta433.SPI_Init();
    Zeta433.SPI_SI4455_Init();

    unsigned char parambytescnt = 0x01; //1 param byte
    unsigned char cmd = 0x01; //part info command
    unsigned char respByteCount = 0x0C;
    unsigned char response[16];
    
    
    Zeta433.SendCmdGetResp(parambytescnt, &cmd, respByteCount, response);

    for(int i=0; i<respByteCount; i++)
    {
        printf("Zeta response: %X\n\r", response[i]);
    }

    //Zeta433.Start_Tx(NULL); //start TX

    cmd = 0x66; //write to fifo
    unsigned char msg[13] = "Test message";

    while(1)    
    {
        Zeta433.SendCmdArgs(cmd, 0x01, sizeof(msg), msg);
        Zeta433.Start_Tx(NULL); //start TX
        printf("Sending msg\n\r");
        ThisThread::sleep_for(100ms);
    }
}

