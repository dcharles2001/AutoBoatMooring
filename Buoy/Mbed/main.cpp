#include "mbed.h"
#include "BuoyComms.h"

//BoatComms Boat(f429spi1, PC_7, PA_15, PB_15, BOAT);
BuoyComms Buoy(l432spi1, PB_6, PB_7, PA_2, LBUOY); //Left buoy test obj
DigitalOut GreenLED(PB_0);
DigitalOut TriggerLine(PB_1);

int main()
{
    SystemCoreClockUpdate();
    //GreenLED = 0;
    printf("Starting\n\r");
    printf("Clock: %d\n\r", SystemCoreClock);

    Buoy.Init();

    unsigned char response[RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH];
    Buoy.GetPartInfo(response);
    for(int i=0; i<8; i++)
    {
        printf("Part info: %x\n\r", response[i]); //display part info
    }
 
    unsigned char state[2];
    Buoy.GetCurrentState(state);
    for(int i=0; i<2; i++)
    {
        printf("State: %x\n\r", state[i]);
    }

    while(1)    
    {
        Buoy.ReceiveAndRead(response, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH);
        for(int i=0; i<RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH; i++)
        {
            printf("RX: %x\n\r", response[i]);
        }
        Buoycmd_t newcmd = Buoy.Interpret(response, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH);
        if(newcmd.cmd == ON)
        {
            printf("Instruction: ON\n\r");
            printf("Duration: %d seconds\n\r", newcmd.param);
        }
        

        Buoy.GetCurrentState(state);
        for(int i=0; i<2; i++)
        {
            printf("State %d: %x\n\r", i, state[i]);
        }

        ThisThread::sleep_for(2s);
    }
}

