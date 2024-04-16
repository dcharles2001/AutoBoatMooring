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

Thread Thread_Cord2;
EventQueue Queue_Cord2(1 * EVENTS_EVENT_SIZE);

Thread Thread_Turret1;
EventQueue Queue_Turret1(1 * EVENTS_EVENT_SIZE);

Thread Thread_Turret2;
EventQueue Queue_Turret2(1 * EVENTS_EVENT_SIZE);

//Thread Creation and Event queue for Launcher
Thread Thread_Launcher;
EventQueue Queue_Launcher(1 * EVENTS_EVENT_SIZE);

//Pin Setup Launcher Turret
//PwmIn Cha7(PC_8);
DigitalIn ES(PF_15);
DigitalIn Swt1(PE_12);
DigitalIn Swt2(PE_10);
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
int ESCount = 0;
int Swt1Count = 0;
int Swt2Count = 0;
int stopCount = 0;

int ESConf = 0;
int Swt1Conf = 0;
int Swt2Conf = 0;
int stopConf = 0;

int countCounter = 100;

int flip = 1;
void LauncherMain(){
    //while(true){
        //printf("Swt1 %d     :Swt2 %d    :ES %d  :Stop %d\n",Swt1Count,Swt2Count,ESCount,stopCount);
        if(ESConf == 0){
            if(Swt1Conf == 1){
               /* SUDO CODE FOR LAUNCHER X-SWEEP
                if(limitOne = 1){
                    flip = 1;
                }else if(limitTwo = 1){
                    flip = -1;
                }
            */
                xAxisControl= xAxisControl + flip;
                ThisThread::sleep_for(10ms);
                //Launch.stepperSControl(xAxisControl);
                //Launch.servoSControl(yAxisControl);
                //Launch.triggerSControl(readyToFire);
                //Launch.safetySControl(safteyControl);
            }else if(Swt2Conf == 1){
                Launch.stepperRCControl();
                Launch.servoRCControl();
                Launch.triggerRCControl();
                Launch.safetyRCControl();
            }else if(stopConf == 1){
                Launch.servoSControl(yAxisControl);
                Launch.triggerSControl(readyToFire);
                Launch.safetySControl(safteyControl);
                
            }
    /*}
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
    }*/
        }
}

//MAIN
int main(){
    //Sensor Code
    Turret1.Setup();
    Turret2.Setup();
    ThisThread::sleep_for(200ms);
    //Turret 1
    Queue_Turret1.call_every(2ms, Turret1Func);
    Thread_Turret1.start(callback(&Queue_Turret1, &EventQueue::dispatch_forever));
    //IR 1
    Queue_Cord1.call_every(15ms, IRSensor1);
    Thread_Cord1.start(callback(&Queue_Cord1, &EventQueue::dispatch_forever));
    //ToF 1
    Queue_Dist1.call_every(5ms, DistAvg1);
    Thread_Dist1.start(callback(&Queue_Dist1, &EventQueue::dispatch_forever));
    //Turret 2
    Queue_Turret2.call_every(2ms, Turret2Func);
    Thread_Turret2.start(callback(&Queue_Turret2, &EventQueue::dispatch_forever));
    //IR 2
    Queue_Cord2.call_every(5ms, IRSensor2);
    Thread_Cord2.start(callback(&Queue_Cord2, &EventQueue::dispatch_forever));
    //ToF 2
    Queue_Dist2.call_every(10ms, DistAvg2);
    Thread_Dist2.start(callback(&Queue_Dist2, &EventQueue::dispatch_forever));
    //Launcher Code
    Queue_Launcher.call_every(10ms, LauncherMain);
    Thread_Launcher.start(callback(&Queue_Launcher, &EventQueue::dispatch_forever));

    while(1){
        if(ES == 1){
            ESCount++;
            if(ESCount >= countCounter/20){
                Thread_Launcher.terminate();
                Thread_Turret1.terminate();
                Thread_Turret2.terminate();
                Thread_ToF1.terminate();
                Thread_Dist1.terminate();
                Thread_Cord1.terminate();
                Thread_ToF2.terminate();
                Thread_Dist2.terminate();
                Thread_Cord2.terminate();

                ESConf = 1;

                Swt1Conf = 0;
                Swt1Count = 0;

                Swt2Conf = 0;
                Swt2Count = 0;

                stopConf = 0;
                stopCount = 0;
                //printf("EMERGENCY STOP\n");
            }
        } else if(Swt1 == 0){
            Swt1Count++;
            if(Swt1Count >= countCounter){
                ESConf = 0;
                ESCount = 0;

                Swt1Conf = 1;

                Swt2Conf = 0;
                Swt2Count = 0;

                stopConf = 0;
                stopCount = 0;
                //printf("Swt1\n");
            }
        }else if(Swt2 == 0){
            Swt2Count++;
            if(Swt2Count >= countCounter){
                ESConf = 0;
                ESCount = 0;

                Swt1Conf = 0;
                Swt1Count = 0;

                Swt2Conf = 1;

                stopConf = 0;
                stopCount = 0;
                //printf("Swt2\n");
            }
        }else{
            stopCount++;
            if(stopCount >= countCounter){
                ESConf = 0;
                ESCount = 0;

                Swt1Conf = 0;
                Swt1Count = 0;

                Swt2Conf = 0;
                Swt2Count = 0;

                stopConf = 1;
                //printf("Stopped\n");
            }
        }
    }
}