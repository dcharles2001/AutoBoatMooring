#include "DRV8825.h"
#include "mbed.h"

DigitalOut myled(LED1);
   
DRV8825::DRV8825(/*PinName _en, PinName m0, PinName m1, PinName m2,*/ PinName _stepPin, PinName dir)://en(_en),
    //microstepping(m0, m1, m2),
    stepPin(_stepPin),
    direction(dir)
{
}

void DRV8825::settings(float microstep, int dir, float speed)
{
    /*//Microsteppiing settings
    if (microstep == 1) microstepping = 0;
    else if (microstep == 1/2) microstepping = 1;
    else if (microstep == 1/4) microstepping = 2;
    else if (microstep == 1/8) microstepping = 3;
    else if (microstep == 1/16) microstepping = 4;
    else if (microstep == 1/32) microstepping = 5;
    */
    if (dir == 1) {
        direction = 0;
    } else if (dir == 0) {
        direction = 1;
    }
    
    //  Speeed or times per second
    if(stepPin == 1){
        stepPin = 0;
        wait_us(10000/speed);
    }
    else{
        stepPin = 1;
        wait_us(10000/speed);
    }
    //myled = stepPin;
    
    
}
/*
void DRV8825::enable()
{
    en = 0;
}

void DRV8825::disable()
{
    en = 1;
}*/