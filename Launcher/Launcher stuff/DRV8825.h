#ifndef MBED_DRV8825_H
#define MBED_DRV8825_H
#endif

#include "mbed.h"

class DRV8825
{
public:
    DRV8825(/*PinName _en, PinName m0, PinName m1, PinName m2,*/ PinName _stepPin, PinName dir);
    void settings(float microstep, int dir, float speed);
    void enable();
    void disable();
    DigitalOut stepPin;
    DigitalOut direction;
private:
    //DigitalOut en;
    //BusOut microstepping;
    //DigitalOut stepPin;
    //DigitalOut direction;
};