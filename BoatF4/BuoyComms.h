#ifndef BUOYCOMMS_H_
#define BUOYCOMMS_H_

#include "ZetaSPI.h"



class BuoyComms: protected zetaspi //Buoy inherits public methods of zetaspi
{


    public:

    BuoyComms(SPIConfig_t Pins, DigitalOut sdn, DigitalIn gpio1, DigitalIn nirq, int type);
    void Init();
    void GetPartInfo(unsigned char* response);
    void GetCurrentState(unsigned char* response);
    void SendMessage(unsigned char* message, unsigned char msgsize);
    bool IdleRXPolling(void);
    void ReceiveAndRead(unsigned char* response, unsigned char respsize);
    Buoycmd_t Interpret(unsigned char* packet, unsigned char packetsize);
    void ReceiveAndInterpret();
    void MessageConstructor(Buoycmd_t instructons, unsigned char* packet, unsigned char packetsize);

    private:

    int DeviceType; //type variable for distinguishing between boat and buoy hardware
    



};





#endif
