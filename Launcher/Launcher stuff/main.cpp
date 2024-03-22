#include "mbed.h"
#include "PwmIn.h"
#include "Servo.h"

DigitalOut dirPin(D7);
DigitalOut stepPin(D8);
int x = 1;

PwmIn Cha1(D9);
PwmIn Cha2(D10);
float Cha1Read,Cha2Read;
int delayTime;

Servo servo(D6);

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
int servoMove;
void servoRCControl(){
    Cha2Read = Cha2.pulsewidth();
    //printf("%f\n", Cha2.pulsewidth());
    servoMove = map(Cha2Read, 1000, 1990, 0, 180);
    printf("%d\n", servoMove);
    wait_us(200);
}

int main()
{

    while (true) {
        
        stepperRCControl();
        servoRCControl();
    }
}