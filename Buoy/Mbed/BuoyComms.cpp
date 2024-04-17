#include "BuoyComms.h"


BuoyComms::BuoyComms(SPIConfig_t Pins, DigitalOut sdn, DigitalIn gpio2, DigitalIn nirq, int type): zetaspi(Pins, sdn, gpio2, nirq), DeviceType(type) 
{
    
}

void BuoyComms::Init()
{
    SPI_Init(); //init SPI peripheral
    SPI_SI4455_Init(); //init si4455 configuration

}

void BuoyComms::GetPartInfo(unsigned char* response)
{
    //response must point to an array of at least 8 bytes to store the full response
    unsigned char cmd = SI4455_CMD_ID_PART_INFO; //part info cmd id 0x01
    SendCmdGetResp(1, &cmd, 8, response);
}

void BuoyComms::GetCurrentState(unsigned char* response)
{
    //response must point to an array of at least 2 bytes to store the full response
    unsigned char cmd = SI4455_CMD_ID_REQUEST_DEVICE_STATE; //device state cmd id 0x33
    SendCmdGetResp(1, &cmd, 2, response);
}

void BuoyComms::SendMessage(unsigned char* message, unsigned char msgsize)
{
    //msgsize must be set correctly to represent the number of elements in array that message points to
    unsigned char cmd = SI4455_CMD_ID_WRITE_TX_FIFO; //write tx fifo cmd id 0x66
    SendCmdArgs(cmd, 0x01, msgsize, message); //load message into zeta tx fifo
    Start_Tx(NULL); //send message
    
}

bool BuoyComms::IdleRXPolling(void)
{
    Radio_StartRx(); //enter RX mode

    //poll fifo 
    unsigned char cmd = SI4455_CMD_ID_FIFO_INFO; //read fifo info command 0x15
    unsigned char resp[2];
    SendCmdGetResp(0x01, &cmd, 2, resp); //send command and read response
    if(resp[0] > 0) //fifo has content
    {
        return 0; //nothing
    }else {
        return 1; //content received
    }
      
}

void BuoyComms::ReceiveAndRead(unsigned char* response, unsigned char respsize)
{
    //respsize must be set correctly to represent the number of elements in array that response points to
    //Radio_StartRx(); //enter RX mode 
    unsigned char cmd = SI4455_CMD_ID_READ_RX_FIFO; //read rx fifo cmd id 0x77
    ReadRX(respsize, response); //read rx fifo
}

Buoycmd_t BuoyComms::Interpret(unsigned char* packet, unsigned char packetsize)
{
    Buoycmd_t buoyinstruct;
    unsigned char onscore = 0;
    unsigned char offscore = 0;

    //confidence testing 
    for(int i=0; i<(packetsize-1); i++) 
    {
        if(packet[i] == ON)
        {
            onscore++;
        }else if(packet[i] == OFF)
        {
            offscore++;
        }else {
            //content corrupted
        }
    }

    if(onscore > offscore) //must be confident to activate, less confidence required for duration
    {
        buoyinstruct.cmd = ON;
        buoyinstruct.param = packet[packetsize-1]; //on duration
    }else {
        buoyinstruct.cmd = OFF;
        buoyinstruct.param = 0; //turning off, on duration irrelevant
    }
    
    return buoyinstruct; //return interpreted instructions

}

void BuoyComms::MessageConstructor(Buoycmd_t instructions, unsigned char* packet, unsigned char packetsize)
{
    for(int i = 0; i<(packetsize-1); i++)
    {
        packet[i] = instructions.cmd;
    }
    packet[packetsize-1] = instructions.param;
}





