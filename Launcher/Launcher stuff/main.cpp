#include "mbed.h"
#include "PwmIn.h"
#include "Servo.h"

using namespace std::chrono;


DigitalOut dirPin(D7);
DigitalOut stepPin(D8);
int x = 1;

PwmIn Cha1(PE_6);
PwmIn Cha2(PE_5);
PwmIn Cha5(PF_7);
PwmIn Cha7(D3);
PwmIn Cha8(PF_8);

float servoMove;
float Cha1Read,Cha2Read,Cha5Read,Cha7Read,Cha8Read = 1500;
int delayTime;

Servo Yservo(D6);
Servo Trigger(D5);
Servo Saftey(PF_9);


float map(float in, float inMin, float inMax, float outMin, float outMax) {
  // check it's within the range
  if (inMin<inMax) { 
    if (in <= inMin) 
      return outMin;
    if (in >= inMax)
      return outMax;
  } else {  // cope with input range being backwards.
    if (in >= inMin) 
      return outMin;
    if (in <= inMax)
      return outMax;
  }
  // calculate how far into the range we are
  float scale = (in-inMin)/(inMax-inMin);
  // calculate the output.
  return outMin + scale*(outMax-outMin);
}

void stepperRCControl(){

    Cha1Read = Cha1.pulsewidth();
        //printf("%f\n", Cha1.pulsewidth());
        if (Cha1Read >1600){
            x = 1;
            delayTime = map(Cha1Read, 1500, 1990, 1000, 2000);
        }else if(Cha1Read<1400){
            x = 0;
            delayTime = map(Cha1Read, 1000, 1500, 1000, 2000);
        }else{
            x = 3;
        }
        float delayTime1 = (3000 - delayTime)/1000;
        delayTime = delayTime/1000;
        if(x < 2){
            if(x == 1){
                dirPin = 1;
                stepPin = 1;
                ThisThread::sleep_for(delayTime1);
                stepPin = 0;
                ThisThread::sleep_for(delayTime1);
            }else if(x == 0){
                dirPin = 0;
                stepPin = 1;
                ThisThread::sleep_for(delayTime);
                stepPin = 0;
                ThisThread::sleep_for(delayTime);
            }
        }else{
            stepPin = 1;
        }
}


void servoRCControl(){
    Cha2Read = Cha2.pulsewidth();
    //printf("%f\n", Cha2.pulsewidth());
    servoMove = map(Cha2Read, 1000, 1990, 0, 1);
    //printf("%f\n", servoMove);
    Yservo = servoMove;
    wait_us(200);
}

void triggerRCControl(){
    Cha8Read = Cha8.pulsewidth();
    if(Cha8Read>1800){
        Trigger = 1;
    }else if(Cha8Read<1100){
        Trigger = 0;
    }
    //printf("%f\n",Cha8Read);
}

void safetyRCControl(){
    Cha5Read = Cha5.pulsewidth();
    if(Cha5Read>1800){
        Saftey = 1;
    }else if(Cha5Read<1100){
        Saftey = 0;
    }
    //printf("%f\n",Cha8Read);
}

////////////////////////

void stepperSControl(int XPos){
        if (XPos == 1){
            x = 1;
        }else if(XPos == 0){
            x = 0;
        }else{
            x = 3;
        }
        float delayTime1 = 2;
        delayTime = 2;
        if(x < 2){
            if(x == 1){
                dirPin = 1;
                stepPin = 1;
                ThisThread::sleep_for(delayTime1);
                stepPin = 0;
                ThisThread::sleep_for(delayTime1);
            }else if(x == 0){
                dirPin = 0;
                stepPin = 1;
                ThisThread::sleep_for(delayTime);
                stepPin = 0;
                ThisThread::sleep_for(delayTime);
            }
        }else{
            stepPin = 1;
        }
}


void servoSControl(float YPos){
    Yservo = YPos;
    ThisThread::sleep_for(1);
}

void triggerSControl(int fire){
    if(fire == 1){
        Trigger = 1;
    }else if(fire == 0){
        Trigger = 0;
    }
}

void safetySControl(int safe){
    if(safe == 1){
        Saftey = 1;
    }else if(safe == 0){
        Saftey = 0;
    }
}
int xAxisControl = 0;
float yAxisControl = 0.5;
int readyToFire = 0;
int safteyControl = 0;

int main()
{

    while (true) {
        Cha7Read = Cha7.pulsewidth();
        if(Cha7Read<1300){
            stepperSControl(xAxisControl);
            servoSControl(yAxisControl);
            triggerSControl(readyToFire);
            safetySControl(safteyControl);
        }else if(Cha7Read>1800){
            stepperRCControl();
            servoRCControl();
            triggerRCControl();
            safetyRCControl();
        }else{
            printf("Stopped\n");
        }
    }
}