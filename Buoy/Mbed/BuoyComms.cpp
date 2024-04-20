#include "BuoyComms.h"
#include <cstdint>


BuoyComms::BuoyComms(SPIConfig_t Pins, ZetaConfig_t ZPins, int type): zetaspi(Pins, ZPins), GPIO2(ZPins.GPIO2), Preamble(ZPins.GPIO1), DeviceType(type) 
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
    constexpr unsigned char cmd = SI4455_CMD_ID_WRITE_TX_FIFO; //write tx fifo cmd id 0x66
    SendCmdArgs(cmd, 0x01, msgsize, message); //load message into zeta tx fifo
    Start_Tx(NULL); //send message
    
}

void BuoyComms::SetRx(void)
{
    Radio_StartRx();
}

void BuoyComms::ChangeState(unsigned char newstate)
{
    constexpr unsigned char cmd = SI4455_CMD_ID_CHANGE_STATE; //0x34 change state command
    SendCmdArgs(cmd, 0x01, 1, &newstate); //force state change to desired newstate
}

void BuoyComms::AttachInterruptRX(void)
{
    //Radio_StartRx();
    Preamble.rise(callback(this, &BuoyComms::SetFlag)); //Set interrupt on valid preamble
    //Preamble.enable_irq(); //enable
}

void BuoyComms::SetFlag(void)
{
    RadioEvent.set(RadioFlag);
}

void BuoyComms::WaitOnMessage(void) //this function locks its thread into a loop until the required interrupt fires, use carefully
{
    uint32_t newflag = 0; //new flag
    //increasing sleep duration reduces proportional time spent at RX power but increases potential latency
    std::chrono::milliseconds Sleepdur = 1550ms; //idle sleep duration waitcycle - RXdur
    Kernel::Clock::duration_u32 RXdur = 450ms; //active RX listening duration -  accounts for 150ms send window + 200ms listen window cycle with generous leeway
   
    unsigned char state[2];

    Preamble.enable_irq(); //reenable
    Preamble.rise(callback(this, &BuoyComms::SetFlag));

    while(newflag != RadioFlag) //loop until event occurs
    {
        ChangeState(SI4455_CMD_CHANGE_STATE_ARG_NEW_STATE_ENUM_SLEEP);
        ThisThread::sleep_for(50ms);
        GetCurrentState(state);
        for(int i=0; i<2; i++)
        {
            printf("State: %x\n\r", state[i]);
        }

        ThisThread::sleep_for(Sleepdur);

        Radio_StartRx(); //switch to RX state
        GetCurrentState(state);
        for(int i=0; i<2; i++)
        {
            printf("State: %x\n\r", state[i]);
        }
        newflag = RadioEvent.wait_all_for(RadioFlag, RXdur); //wait on interrupt event for specified time
    }

    Preamble.disable_irq(); //task done, disable IRQ
}

void BuoyComms::MessageWaitResponse(unsigned char* message)
{
    uint32_t newflag = 0; //new flag
    Kernel::Clock::duration_u32 RXdur = 200ms; //active RX listening duration - should give the recipient time to produce and send a response
    constexpr unsigned int burstcount = (WaitCycle/150) + 1; // total loop duration/approximate time to send 1 packet = required burst count + 1 to account for imprecision
   
    Preamble.enable_irq(); //reenable
    Preamble.rise(callback(this, &BuoyComms::SetFlag));

    for(int i=0; i<burstcount; i++)
    {
        SendMessage(message, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH); //send new message
        ThisThread::sleep_for(150ms); //approximate time to transmit packet at default data rate
        //by checking after every send, opportunity for reduced latency arises if recipient is already part way through its sleep cycle (which is almost certain (varying extent))
        Radio_StartRx(); //switch to RX state

        newflag = RadioEvent.wait_all_for(RadioFlag, RXdur); //wait on interrupt event for specified time
    }
       
    Preamble.disable_irq(); //task done, disable IRQ
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
    ReadRX(respsize, response); //read zeta rx fifo
    //preambleflag = false;
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





