#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "mbed.h"
#include "PwmIn.h"
#include "Servo.h"

class Launcher{

    public:
        void stepperRCControl();
        void servoRCControl();
        void triggerRCControl();
        void safetyRCControl();

        void stepperSControl(int XPos);
        void servoSControl(float YPos);
        void triggerSControl(int fire);
        void safetySControl(int safe);
        float servoLocation();

        int stepperx= 1, delayTime;
        float Cha1Read,Cha2Read,Cha5Read,Cha8Read = 1500;
        float servoMove;

    protected:
};

#endif