#include "mbed.h"
#include "Servo.h"
#include "stdio.h"
#include <cstdio>
#include <iostream>
#include "PwmIn.h"
#include "Launcher.h"
#include "Sensors.h"

using namespace std;

Launcher Launch;
Sensors Turret1(1);
Sensors Turret2(2);

//Thread Creation and Event queue for Sensor Turrets
Thread Thread_ToF1;
EventQueue Queue_ToF1(1 * EVENTS_EVENT_SIZE);

Thread Thread_Dist1;
EventQueue Queue_Dist1(1 * EVENTS_EVENT_SIZE);

Thread Thread_Cord1;
EventQueue Queue_Cord1(1 * EVENTS_EVENT_SIZE);

Thread Thread_ToF2;
EventQueue Queue_ToF2(1 * EVENTS_EVENT_SIZE);

Thread Thread_Dist2;
EventQueue Queue_Dist2(1 * EVENTS_EVENT_SIZE);

Thread Thread_Turret1;
EventQueue Queue_Turret1(1 * EVENTS_EVENT_SIZE);

Thread Thread_Turret2;
EventQueue Queue_Turret2(1 * EVENTS_EVENT_SIZE);

//Thread Creation and Event queue for Launcher
Thread Thread_Launcher;
EventQueue Queue_Launcher(1 * EVENTS_EVENT_SIZE);

//Pin Setup Launcher Turret
PwmIn Cha7(D3);

//Setup for Sensor Turrets
Timer tick;

//Setup for Launcher Turret
float Cha7Read = 1500;
/////TEMP REPLACE WITH SESNOR VALUES/////
int xAxisControl = 0,readyToFire = 0,safteyControl = 0;
float yAxisControl = 0.5;
/////////////////////////////////////////

//Functions to Start Sensor Turrets
void Turret1Func(){
    Turret1.Turret_Function1();
}
void Turret2Func(){
    Turret2.Turret_Function2();
}
void IRSensor1(){
    Turret1.IR_Sensor1();
}
void IRSensor2(){
    Turret2.IR_Sensor2();
}
void DistAvg1(){
    Turret1.Dist_Avg1();
}
void DistAvg2(){
    Turret2.Dist_Avg2();
}

//Functions for Launcher Turret
void LauncherMain(){
    Cha7Read = Cha7.pulsewidth();
    //printf("%f\n", Cha7Read);
    if((Cha7Read>1300)&&(Cha7Read<1800)){
        Launch.stepperSControl(xAxisControl);
        Launch.servoSControl(yAxisControl);
        Launch.triggerSControl(readyToFire);
        Launch.safetySControl(safteyControl);
    }else if(Cha7Read<1300){
        Launch.stepperRCControl();
        Launch.servoRCControl();
        Launch.triggerRCControl();
        Launch.safetyRCControl();
    }else{
        //printf("Stopped\n");
        Launch.servoSControl(yAxisControl);
        Launch.triggerSControl(readyToFire);
        Launch.safetySControl(safteyControl);
    }
}

//MAIN
int main(){
    //Sesnor Code
    Turret1.Setup();
    Turret2.Setup();

    Queue_Turret1.call_every(2ms, Turret1Func);
    Thread_Turret1.start(callback(&Queue_Turret1, &EventQueue::dispatch_forever));

    Queue_Cord1.call_every(50ms, IRSensor1);
    Thread_Cord1.start(callback(&Queue_Cord1, &EventQueue::dispatch_forever));

    Queue_Turret2.call_every(2ms, Turret2Func);
    Thread_Turret2.start(callback(&Queue_Turret2, &EventQueue::dispatch_forever));

    Queue_Cord1.call_every(50ms, IRSensor2);
    Thread_Cord1.start(callback(&Queue_Cord1, &EventQueue::dispatch_forever));
    
    Queue_Dist1.call_every(10ms, DistAvg1);
    Thread_Dist1.start(callback(&Queue_Dist1, &EventQueue::dispatch_forever));

    Queue_Dist2.call_every(10ms, DistAvg2);
    Thread_Dist2.start(callback(&Queue_Dist2, &EventQueue::dispatch_forever));

    //Launcher Code
    Queue_Launcher.call_every(10ms, LauncherMain);
    Thread_Launcher.start(callback(&Queue_Launcher, &EventQueue::dispatch_forever));
}