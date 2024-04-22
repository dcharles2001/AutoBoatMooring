#include "Launcher.h"

PwmIn Cha1(PE_6);
PwmIn Cha2(PE_5);
PwmIn Cha5(PF_7);
//PwmIn Cha7(D3);
PwmIn Cha8(PF_8);

DigitalOut dirPin(D7);
DigitalOut stepPin(D8);

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

void Launcher::stepperRCControl(){

    Cha1Read = Cha1.pulsewidth();
    //printf("%f\n", Cha1Read);
    if (Cha1Read >1600){
        stepperx= 0;
        delayTime = map(Cha1Read, 1500, 1990, 1000, 2000);
    }else if(Cha1Read<1400){
        stepperx= 1;
        delayTime = map(Cha1Read, 1000, 1500, 1000, 2000);
    }else{
        stepperx= 3;
    }
    float delayTime1 = (3000 - delayTime)/1000;
    delayTime = delayTime/1000;
    if(stepperx< 2){
        if(stepperx== 1){
            dirPin = 1;
            stepPin = 1;
            ThisThread::sleep_for(delayTime1);
            stepPin = 0;
            ThisThread::sleep_for(delayTime1);
        }else if(stepperx== 0){
            dirPin = 0;
            stepPin = 1;
            ThisThread::sleep_for(delayTime);
            stepPin = 0;
            ThisThread::sleep_for(delayTime);
        }
    }else{
        
    }
}

void Launcher::servoRCControl(){
    Cha2Read = Cha2.pulsewidth();
    servoMove = map(Cha2Read, 1000, 1990, 0.05, 0.4);
    float difference = servoMove - Yservo;
    Yservo = Yservo + (difference/80);
    servoLocation();
    ThisThread::sleep_for(2ms);
}

float Launcher::servoLocation(){
    float location = servoMove;
    return location;
}

void Launcher::triggerRCControl(){
    Cha8Read = Cha8.pulsewidth();
    if(Cha8Read>1800){
        Trigger = 0;
    }else if(Cha8Read<1100){
        Trigger = 1;
    }
}

void Launcher::safetyRCControl(){
    Cha5Read = Cha5.pulsewidth();
    if(Cha5Read>1800){
        Saftey = 1;
    }else if(Cha5Read<1100){
        Saftey = 0;
    }
}

//Sensor Control
void Launcher::stepperSControl(int XPos){
    if (XPos == 1){
        stepperx= 1;
    }else if(XPos == 0){
        stepperx= 0;
    }else{
        stepperx= 3;
    }
    float delayTime1 = 2;
    delayTime = 2;
    if(stepperx< 2){
        if(stepperx== 1){
            dirPin = 1;
            stepPin = 1;
            ThisThread::sleep_for(delayTime1);
            stepPin = 0;
            ThisThread::sleep_for(delayTime1);
        }else if(stepperx== 0){
            dirPin = 0;
            stepPin = 1;
            ThisThread::sleep_for(delayTime);
            stepPin = 0;
            ThisThread::sleep_for(delayTime);
        }
    }else{

    }
}

void Launcher::servoSControl(float YPos){
    Yservo = YPos;
    //ThisThread::sleep_for(2ms);
}

void Launcher::triggerSControl(int fire){
    if(fire == 1){
        Trigger = 1;
    }else if(fire == 0){
        Trigger = 0;
    }
}

void Launcher::safetySControl(int safe){
    if(safe == 1){
        Saftey = 1;
    }else if(safe == 0){
        Saftey = 0;
    }
}
