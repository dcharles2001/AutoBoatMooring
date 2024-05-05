#include "BuoyComms.h"
#include <cstdint>


BuoyComms::BuoyComms(SPIConfig_t Pins, ZetaConfig_t ZPins, int type): zetaspi(Pins, ZPins), GPIO2(ZPins.GPIO2), Preamble(ZPins.GPIO1), DeviceType(type) 
{
    
}

void BuoyComms::Init() //initialise
{
    SPI_Init(); //init SPI peripheral
    SPI_SI4455_Init(); //init si4455 configuration

}

void BuoyComms::GetPartInfo(unsigned char* response) //provides basic Si4455 info
{
    //response must point to an array of at least 8 bytes to store the full response
    unsigned char cmd = SI4455_CMD_ID_PART_INFO; //part info cmd id 0x01
    SendCmdGetResp(1, &cmd, 8, response);
}

void BuoyComms::GetCurrentState(unsigned char* response) //provides current Si4455 device state 
{
    //response must point to an array of at least 2 bytes to store the full response
    unsigned char cmd = SI4455_CMD_ID_REQUEST_DEVICE_STATE; //device state cmd id 0x33
    SendCmdGetResp(1, &cmd, 2, response);
}

void BuoyComms::SendMessage(unsigned char* message, unsigned char msgsize) //load message into Si4455 TX buffer
{
    //msgsize must be set correctly to represent the number of elements in array that message points to
    constexpr unsigned char cmd = SI4455_CMD_ID_WRITE_TX_FIFO; //write tx fifo cmd id 0x66
    SendCmdArgs(cmd, 0x01, msgsize, message); //load message into zeta tx fifo
    Start_Tx(NULL); //send message
    
}

void BuoyComms::SetRx(void) //provides access to protected ZETA method
{
    Radio_StartRx(); //switch to RX state
}

int BuoyComms::ChangeState(unsigned char newstate) //change Si4455 device state
{
    if(newstate > 8) //check if the requested state change is invalid
    {
        return 1;
    }
    constexpr unsigned char cmd = SI4455_CMD_ID_CHANGE_STATE; //0x34 change state command
    SendCmdArgs(cmd, 0x01, 1, &newstate); //force state change to desired newstate
    return 0;
}

void BuoyComms::ActiveInterruptRX(void) //switch to RX state and enable interrupt on valid preamble
{
    Radio_StartRx();
    Preamble.enable_irq();
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
    std::chrono::milliseconds Sleepdur = 1500ms; //idle sleep duration waitcycle - RXdur
    Kernel::Clock::duration_u32 RXdur = 500ms; //active RX listening duration -  accounts for 150ms send window + 200ms listen window cycle with generous leeway
   
    unsigned char state[2];

    Preamble.enable_irq(); //reenable
    Preamble.rise(callback(this, &BuoyComms::SetFlag));

    while(newflag != RadioFlag) //loop until event occurs
    {
        ChangeState(SI4455_CMD_CHANGE_STATE_ARG_NEW_STATE_ENUM_SLEEP);
        //ThisThread::sleep_for(50ms);

        ThisThread::sleep_for(Sleepdur);

        Radio_StartRx(); //switch to RX state
        
        newflag = RadioEvent.wait_all_for(RadioFlag, RXdur); //wait on interrupt event for specified time
    }

    ThisThread::sleep_for(50ms);

    Preamble.disable_irq(); //task done, disable IRQ
}

void BuoyComms::MessageWaitResponse(unsigned char* message)
{
    uint32_t newflag = 0; //new flag
    Kernel::Clock::duration_u32 RXdur = 350ms; //active RX listening duration - should give the recipient time to produce and send a response
    constexpr unsigned int burstcount = (WaitCycle/150) + 1; // total loop duration/approximate time to send 1 packet = required burst count + 1 to account for imprecision
   
    Preamble.enable_irq(); //reenable
    Preamble.rise(callback(this, &BuoyComms::SetFlag));

    unsigned char state[2];

    for(int i=0; i<burstcount; i++)
    {
        SendMessage(message, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH); //send new message
        ThisThread::sleep_for(150ms); //approximate time to transmit packet at default data rate
        //by checking after every send, opportunity for reduced latency arises if recipient is already part way through its sleep cycle (which is almost certain (varying extent))
        Radio_StartRx(); //switch to RX state
        
        newflag = RadioEvent.wait_all_for(RadioFlag, RXdur); //wait on interrupt event for specified time
        if(newflag == RadioFlag)
        {
            printf("Flag set!\n\r");
            break;
        }
    }
    
    //Radio_StartRx();
    Preamble.disable_irq(); //task done, disable IRQ
}

bool BuoyComms::IdleRXPolling(void) //basic polling method for receiving packets
{
    Radio_StartRx(); //enter RX mode

    //poll fifo for content
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

bool BuoyComms::ReceiveAndRead(unsigned char* response, unsigned char respsize)
{
    //respsize must be set correctly to represent the number of elements in array that response points to
    unsigned char checkfifo = SI4455_CMD_ID_FIFO_INFO;
    unsigned char fifoinfo[2];
    SendCmdGetResp(1, &checkfifo, 2, fifoinfo);
    if(fifoinfo[0] > 0) //content present in RX fifo?
    {
        ReadRX(respsize, response); //read zeta rx fifo
        return 0; //success
    }else {
        return 1; //fifo empty
    }
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

Buoycmd_t BuoyComms:: InterpretBoyerMoore(unsigned char* packet, unsigned char packetsize)
{
    Buoycmd_t buoyinstruct;
    char c = 0; //Boyer-Moore counter, initial value must be 0
    unsigned char m; //current Boyer-Moore element
    //Boyer-Moore x will be represented by the current element of packet
    //Using this algorithm, the majority element can be found
    //If the majority element is 1, the packet has passed the confidence test 

    for(int i=0; i<(packetsize-1); i++)
    {
        if(c == 0)
        {
            m = packet[i];
        }else if(m == packet[i])
        {
            c++; //increment counter
        }else {
            c--; //decrement counter
        }
    }

    if(m == 1)
    {
        buoyinstruct.cmd = ON;
        buoyinstruct.param = packet[packetsize-1]; //on duration
    }else 
    {    
        buoyinstruct.cmd = OFF;
        buoyinstruct.param = 0; //turning off, on duration irrelevant
    }

    return buoyinstruct;
}

bool BuoyComms::InterpretResponse(unsigned char* packet)
{
    Buoycmd_t buoyinstruct;
    unsigned char onscore = 0;
    unsigned char offscore = 0;

    //confidence testing 
    for(int i=0; i<(RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH); i++) 
    {
        if(packet[i] == 1)
        {
            onscore++;
        }else if(packet[i] == 0)
        {
            offscore++;
        }else {
            //content corrupted
        }
    }

    if(onscore > offscore) //must be confident to activate, less confidence required for duration
    {
        return false; //success, good packet
    }else {
        return true; //failure, bad packet
    }

}

int BuoyComms::InstructionConfigurator(Buoycmd_t instructions, unsigned char* packet, unsigned char packetsize)
{
    if(sizeof(packet)/(sizeof(packet[0])) != packetsize) //check if packetsize aligns with size of packet array
    {
        return 1;
    }

    for(int i = 0; i<(packetsize-1); i++)
    {
        packet[i] = instructions.cmd;
    }
    packet[packetsize-1] = instructions.param;

    return 0;
}

int BuoyComms::GetDeviceType(void)
{
    return DeviceType;
}





