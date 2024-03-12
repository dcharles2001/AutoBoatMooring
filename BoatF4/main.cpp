#include "mbed.h"
#include "ZetaSPI.h"

zetaspi Zeta433(f429spi1, PC_7, PA_15, PB_15);

unsigned int packetconstructor(unsigned char* preamble, unsigned char preamblesize, unsigned char* payload, unsigned char payloadsize, 
unsigned char* syncword, unsigned char syncwordsize, unsigned char* packet, unsigned char packetsize);

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

    unsigned char preamble[2] = {0xAA, 0xAA}; //2 byte preamble 10101010 101010
    unsigned char testmsg[8] = {0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81}; //8 byte test payload
    unsigned char syncword[2] = {0x2D, 0xD4}; //2 byte syncword 
   
    unsigned char testpacket[12];

    //packetconstructor(preamble, sizeof(preamble), testmsg, sizeof(testmsg), syncword, sizeof(syncword), testpacket, sizeof(testpacket));
    /*
    for(int i=0; i<12; i++) 
    {
        printf("Packet: %x\n\r", testpacket[i]);
    }
    */
    unsigned char fifo_clr[2] = {0x15, 0x03}; //fifo info
    unsigned char fifo_tx = 0x15; //tx fifo info
    unsigned char resp[2];

    unsigned char readystate[2] = {0x34, 0x03};

    

    while(1)    
    {
        Zeta433.SendCmdArgs(fifo_clr[1], 0x01, 0x01, &fifo_clr[2]); //clear fifo
        Zeta433.SendCmdArgs(readystate[1], 0x01, 0x01, &readystate[2]); //set ready state
        //printf("Fifo info prior: %X\n\r", resp);

        Zeta433.SendCmdArgs(cmd, 0x01, 0x08, testmsg); //load packet into fifo
        //Zeta433.SendCmdArgs(0x66, 0x01, sizeof(msg), msg);
        
        /*
        printf("Delay\n\r");
        ThisThread::sleep_for(3s);
        */
        /*
        Zeta433.SendCmdGetResp(0x01, &fifo_tx, 0x02, resp);
        for(int i=0; i<2; i++)
        {
            printf("Fifo info post-fill: %X\n\r", resp[i]);
        }
        */
        
        Zeta433.Start_Tx(NULL); //start TX
        //Zeta433.SendCmdArgs(0x34, 0x01, 0x01, &stateparam);

        
        //printf("Sending msg\n\r");
        ThisThread::sleep_for(1s);
    }
}

unsigned int packetconstructor(unsigned char* preamble, unsigned char preamblesize, unsigned char* payload, unsigned char payloadsize, unsigned char* syncword, unsigned char syncwordsize, unsigned char* packet, unsigned char packetsize)
{

    if(packetsize != sizeof(preamble) + sizeof(payload) + sizeof(syncword)) //mismatch, this will not work
    {
        printf("Packet component size mismatch\n\r");
        return 1; //fail
    }

    if(payloadsize != RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH) //check config packet length
    {
        printf("Packet size mismatch\n\r");
        return 1; //fail
    }
    
  
    for(int i=0; i<preamblesize; i++)
    {
        packet[i] = preamble[i]; //copy preamble into packet
    }

    for(int i=preamblesize; i<(preamblesize + payloadsize); i++)
    {
        packet[i] = payload[i-preamblesize];
    }

    for(int i=(preamblesize + payloadsize); i<(packetsize - syncwordsize); i++)
    {
        packet[i] = syncword[i-(preamblesize + payloadsize)];
    }

    return 0; //success
}
