#include "mbed.h"
#include "BuoyComms.h"

//BuoyComms Buoy(f429spi1, F4Zeta, RBUOY);
BuoyComms Buoy(l432spi1, L4Zeta, LBUOY); //Left buoy test obj
DigitalOut GreenLED(PB_0);
DigitalOut TriggerLine(PB_1);

//InterruptIn TestIn(PA_8);

//EventFlags RadioEvent;
//osThreadId_t ThreadID0;

Buoycmd_t ReceiveCMDs(unsigned char* message, bool interpret);
void TestCallback(void);

int main()
{
    SystemCoreClockUpdate();
    //GreenLED = 0;
    printf("Starting\n\r");
    printf("Clock: %d\n\r", SystemCoreClock);

    //ThreadID0 = ThisThread::get_id();

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

    unsigned char goodpacket[8] = "1111111"; //response message on successful instruction
    unsigned char badpacket[8] = "0000000"; //bad message response
    
    while(1)
    {
        Buoy.WaitOnMessage(); //wait on RX event
        Buoycmd_t newcmd = ReceiveCMDs(response, 1); //get packet from RX buffer and interpret
        if(Buoy.GetDeviceType() == RBUOY)
        {
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
            printf("Instruction: ON\n\r");
            printf("Duration: %d seconds\n\r", newcmd.param);
        }else{

            for(int i=0; i<5; i++) //send response burst
            {
                Buoy.SendMessage(badpacket, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH); //send failure response message message
            }
            continue; //restart loop - wait for another instruction
        }

        // now go on to do LED control

        for(int i=0; i<newcmd.param; i++)
        {
            //LED stuff
            ThisThread::sleep_for(2s);

        }

        

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

/*
void TestCallback(void)
{
    RadioEvent.set(RadioFlag); //set flag for radio RX event
    //Buoy.SetFlag();
}
*/
