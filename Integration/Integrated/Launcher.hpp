#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "mbed.h"
#include "PwmIn.h"
#include "Servo.h"
#include "BuoyComms.h"

extern BuoyComms Boat;
extern EventQueue PrintQueue;

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

        //---comms integration stuff
        void commsCheck(Buoycmd_t newcmd);
        bool checkbuoysTime(void);
        std::chrono::seconds getbuoysTime(void);
        bool getTimerStatus(void);
        //---

        int stepperx= 1, delayTime;
        float Cha1Read,Cha2Read,Cha5Read,Cha8Read = 1500;
        float servoMove;

    protected:

		//---comms integration stuff
        bool TimerActive = 0; //indicates timer status
        Timer BuoysTimer; //timer for tracking Buoys on time
        std::chrono::seconds newtime; //time we have told the buoys to turn on for
		//---
};

#endif