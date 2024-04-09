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

void BuoyComms::ReceiveAndRead(unsigned char* response, unsigned char respsize)
{
    //respsize must be set correctly to represent the number of elements in array that response points to
    Radio_StartRx(); //enter RX mode 
    ThisThread::sleep_for(2s);
    ReadRX(respsize, response);
}

