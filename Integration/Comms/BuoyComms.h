#ifndef BUOYCOMMS_H_
#define BUOYCOMMS_H_

#include "ZetaSPI.h"

//extern EventFlags RadioEvent;

class BuoyComms: protected zetaspi //Buoy inherits public methods of zetaspi
{


    public:

    BuoyComms(SPIConfig_t Pins, ZetaConfig_t ZPins, int type);
    void Init();
    void GetPartInfo(unsigned char* response);
    void GetCurrentState(unsigned char* response);
    void SendMessage(unsigned char* message, unsigned char msgsize);
    void SetRx(void);
    int ChangeState(unsigned char newstate);
    void ActiveInterruptRX(void);
    void SetFlag(void);
    void WaitOnMessage(void);
    void MessageWaitResponse(unsigned char* message);
    bool IdleRXPolling(void);
    bool ReceiveAndRead(unsigned char* response, unsigned char respsize);
    Buoycmd_t Interpret(unsigned char* packet, unsigned char packetsize);
    bool InterpretResponse(unsigned char* packet);
    void ReceiveAndInterpret();
    int InstructionConfigurator(Buoycmd_t instructons, unsigned char* packet, unsigned char packetsize);
    int GetDeviceType(void);

    private:

    int DeviceType; //type variable for distinguishing between boat and buoy hardware
    DigitalIn GPIO2;
    InterruptIn Preamble; //valid preamble interrupt
    EventFlags RadioEvent;


};





#endif
