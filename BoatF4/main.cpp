#include "mbed.h"
#include "BuoyComms.h"

BuoyComms Boat(f429spi1, PC_7, PA_15, PB_15, BOAT);
//BuoyComms Buoy(l432spi1, PB_6, PB_7, PA_2, LBUOY); //Left buoy test obj
DigitalOut GreenLED(PB_0);
DigitalOut TriggerLine(PB_1);

int main()
{
    SystemCoreClockUpdate();
    //GreenLED = 0;
    printf("Starting\n\r");
    printf("Clock: %d\n\r", SystemCoreClock);

    Boat.Init();

    unsigned char response[8];
    Boat.GetPartInfo(response);
    for(int i=0; i<8; i++)
    {
        printf("Part info: %x\n\r", response[i]); //display part info
    }
 
    unsigned char state[2];
    Boat.GetCurrentState(state);
    for(int i=0; i<2; i++)
    {
        printf("State: %x\n\r", state[i]);
    }
    Buoycmd_t newcmd = {ON, 30}; //turn on for 30 seconds
    unsigned char TestMessage[RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH]; //new message
    Boat.MessageConstructor(newcmd, TestMessage, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH);
    

    while(1)    
    {
        Boat.SendMessage(TestMessage, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH);
        Boat.GetCurrentState(state);
        for(int i=0; i<2; i++)
        {
            printf("State: %x\n\r", state[i]);
        }
        

        ThisThread::sleep_for(1s);
    }
}