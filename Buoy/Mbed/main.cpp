#include "mbed.h"
#include "BuoyComms.h"

//BoatComms Boat(f429spi1, PC_7, PA_15, PB_15, BOAT);
BuoyComms Buoy(l432spi1, PB_6, PB_7, PA_2, LBUOY); //Left buoy test obj
DigitalOut GreenLED(PB_0);
DigitalOut TriggerLine(PB_1);

Buoycmd_t ReceiveCMDs(unsigned char* message);

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

    unsigned char message[8] = "CMDGOOD"; //response message

    while(1)    
    {
        //receive new cmd
        if(!Buoy.IdleRXPolling()) //don't bother trying to parse cmds until anything arrives in receive fifo 
        {
            ThisThread::sleep_for(500ms);
            printf("Receive!\n\r");
            Buoycmd_t newcmd = ReceiveCMDs(response);
            if(newcmd.cmd == ON)
            {
                printf("Instruction: ON\n\r");
                printf("Duration: %d seconds\n\r", newcmd.param);
                //send reply
                //ThisThread::sleep_for(1s);
                Buoy.SendMessage(message, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH); //send message
            }

            ThisThread::sleep_for(2s);
           
        }else 
        {
            printf("Nothing yet\n\r");
            ThisThread::sleep_for(250ms);
        }
    }
}


Buoycmd_t ReceiveCMDs(unsigned char* message)
{
    Buoy.ReceiveAndRead(message, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH);

    for(int i=0; i<RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH; i++)
    {
        printf("RX: %x\n\r", message[i]);
    }

    Buoycmd_t newcmd = Buoy.Interpret(message, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH);

   return newcmd;     
}
