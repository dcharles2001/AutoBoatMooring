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
    Zeta433.GetIntStatus(0x00, 0x00, 0xff);

    cmd = 0x66; //write to fifo
    unsigned char msg[33] = "Test message Test1234Test1234567";
    unsigned char testmsg[4] = "AAA";

    unsigned char cmd1 = 0x15; //fifo info
    unsigned char resp[2];
    
    unsigned char stateparam = 0x02;

    unsigned char cmd2 = 0x77; //read fifo
    unsigned char resp2[4];

    Zeta433.SendCmdArgs(0x15, 0x01, 0x01, &stateparam);

    while(1)    
    {
        //Zeta433.SendCmdGetResp(0x01, &cmd1, 0x01, &resp);
        //printf("Fifo info prior: %X\n\r", resp);

        Zeta433.SendCmdArgs(0x66, 0x01, 0x03, testmsg);
        //Zeta433.SendCmdArgs(0x66, 0x01, sizeof(msg), msg);
        
        /*
        printf("Delay\n\r");
        ThisThread::sleep_for(3s);
        */
        Zeta433.SendCmdGetResp(0x01, &cmd1, 0x02, resp);
        for(int i=0; i<2; i++)
        {
            printf("Fifo info post-fill: %X\n\r", resp[i]);
        }

        /*
        Zeta433.SendCmdGetResp(0x01, &cmd2, 0x04, resp2);
        for(int i=0; i<4; i++)
        {
            printf("Fifo read: %c\n\r", resp2[i]);
        }
        */
        Zeta433.Start_Tx(NULL); //start TX
        //Zeta433.SendCmdArgs(0x34, 0x01, 0x01, &stateparam);

        
        //printf("Sending msg\n\r");
        ThisThread::sleep_for(1s);
    }
}

