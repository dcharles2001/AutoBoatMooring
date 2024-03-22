#include "mbed.h"
#include "PwmIn.h"
#include "Servo.h"

DigitalOut dirPin(D7);
DigitalOut stepPin(D8);
int x = 1;

PwmIn Cha1(D9);
PwmIn Cha2(D10);
PwmIn Cha8(D3);
float Cha1Read,Cha2Read,Cha8Read;
int delayTime;

Servo Yservo(D6);
Servo Trigger(D5);

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
            delayTime = map(Cha1Read, 1500, 1990, 500, 2000);
        }else if(Cha1Read<1400){
            x = 0;
            delayTime = map(Cha1Read, 1000, 1500, 500, 2000);
        }else{
            x = 3;
        }
        if(x < 2){
            if(x == 1){
                dirPin = 1;
                stepPin = 1;
                wait_us(2500-delayTime);
                stepPin = 0;
                wait_us(2500-delayTime);
            }else if(x == 0){
                dirPin = 0;
                stepPin = 1;
                wait_us(delayTime);
                stepPin = 0;
                wait_us(delayTime);
            }
        }else{
            stepPin = 1;
        }
}

float servoMove;
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

int main()
{

    while (true) {
        
        stepperRCControl();
        servoRCControl();
        triggerRCControl();
    }
}