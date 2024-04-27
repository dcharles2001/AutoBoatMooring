#include "mbed.h"
#include "BuoyComms.h"
#include <chrono>


#define buoytype RBUOY
#define BLINKING_RATE 20ms // Default Blinking rate in milliseconds

/*
#ifdef buoytype 
#elif buoytype == LBUOY
    #define BLINKING_RATE 60ms
#else
    #define buoytype RBUOY
#endif
*/

BuoyComms Buoy(f429spi1, F4Zeta, buoytype); //boat comms object
Buoycmd_t ReceiveCMDs(unsigned char* message, bool interpret);

Timer LEDTimer;

int main()
{
    Buoy.Init();

    unsigned char response[RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH];
    unsigned char goodpacket[8] = "1111111"; //response message on successful instruction
    unsigned char badpacket[8] = "0000000"; //bad message response

    std::chrono::seconds Ontime = 30s; //30s default

    printf("Blinkrate: %llu\n\r", BLINKING_RATE);
    DigitalOut led(D3);

    while (true) 
    {
        LEDTimer.reset();

        Buoy.WaitOnMessage(); //wait on RX event
        Buoycmd_t newcmd = ReceiveCMDs(response, 1); //get packet from RX buffer and interpret
        if(Buoy.GetDeviceType() == RBUOY)
        {
            Buoy.ChangeState(1); //sleep while waiting
            printf("RBUOY\n\r");
            ThisThread::sleep_for(2s); //delay to stage responses between buoys
        }
        if((newcmd.cmd == ON) && (newcmd.param >= MINTIME)) //check if the instruction makes sense
        {
            ThisThread::sleep_for(100ms);
            for(int i=0; i<10; i++) //send response burst
            {
                Buoy.SendMessage(goodpacket, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH); //send success response message
                //ThisThread::sleep_for(150ms);
            }
            Ontime = (std::chrono::seconds)newcmd.param;
            printf("Instruction: ON\n\r");
            printf("Duration: %d seconds\n\r", newcmd.param);
        }else{

            printf("Failure response\n\r");
            for(int i=0; i<5; i++) //send response burst
            {
                Buoy.SendMessage(badpacket, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH); //send failure response message message
            }
            continue; //restart loop - wait for another instruction
        }

        LEDTimer.start();
        while((std::chrono::seconds)LEDTimer.elapsed_time().count() < Ontime)
        {
            led = !led;
            ThisThread::sleep_for(BLINKING_RATE);
        }
        LEDTimer.stop();
        LEDTimer.reset();

    }
}

Buoycmd_t ReceiveCMDs(unsigned char* message, bool interpret)
{
    if(Buoy.ReceiveAndRead(message, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH)) 
    {
        //fail, nothing in fifo
        printf("RX FIFO empty\n\r");
        return{0, 0};
    }

    for(int i=0; i<RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH; i++)
    {
        printf("RX: %x\n\r", message[i]);
    }

    if(interpret)
    {
        Buoycmd_t newcmd = Buoy.Interpret(message, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH);
        return newcmd;
    }

   return {0, 0}; //interpretation not requested, return nothing     
}