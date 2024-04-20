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

DigitalIn LimitRight(PD_1);
DigitalIn LimitLeft(PG_0);

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
    Turret1.Turret_Function();
}
void Turret2Func(){
    Turret2.Turret_Function();
}
void IRSensor1(){
    Turret1.IR_Sensor();
}
void IRSensor2(){
    Turret2.IR_Sensor();
}
void DistAvg1(){
    Turret1.Dist_Avg();
}
void DistAvg2(){
    Turret2.Dist_Avg();
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
int counting = 0;
void LauncherMain(){
    //while(true){
        //printf("Swt1 %d     :Swt2 %d    :ES %d  :Stop %d\n",Swt1Count,Swt2Count,ESCount,stopCount);
        if(ESConf == 0){
                                //if(0){
            float pi = 3.14159265;
                
                    //printf("LimitRight %d\n",LR);
                    //if(Vert1 - Vert2<5){
                    //launchElevation = (Vert1 + Vert2)/2;
                    //printf("Angle %d  :   Dist %d     :Elevation %d\n",launchAngle, launchDist, launchElevation);
                    //}
                    //ThisThread::sleep_for(500ms);
                    //printf("Angle %d  :   Dist %d     :Elevation %d",launchAngle, launchDist, launchElevation);
            
            
            if(Swt1Conf == 1){
                while(flip == 1){
                    xAxisControl = 0;
                    Launch.stepperSControl(xAxisControl);
                    if(LimitRight == 0){
                        flip = 2;
                        counting = 0;
                    }
                }
                while(counting<10){
                    xAxisControl = 1;
                    Launch.stepperSControl(xAxisControl);
                    counting++;
                }
                while(Swt1Conf == 1){
                    if(Turret1.Target != 0 && Turret2.Target!= 0  && Turret1.Target != Turret2.Target){//(Turret1.Target != 0 && Turret2.Target != 0 && Turret1.Target != Turret2.Target){
                        int X1 = Turret1.Angle * 90;// + 45;    //Raw X Data 1 (0-1)
                        int X2 = Turret2.Angle * 90;// + 45;  //Raw X Data 2(0-1)
                        int Y1 = Turret1.Angle * 90;    //Raw X Data 1 (0-1)
                        int Y2 = Turret2.Angle * 90;  //Raw X Data 2(0-1)
                        int Dist1 = Turret1.Dist;  //Total Distance 1
                        int Dist2 = Turret2.Dist;  //Total Distance 2
                            //printf("X1 %d at %dcm  :   X2 %d at %dcm \n",X1,Dist1,X2,Dist2);
                        int Horz1 = Dist1*sin(X1*pi/180); // Horizontal Disance 1
                        int Vert1 = Dist1*cos(Y1*pi/180);

                        int Horz2 = Dist2*sin(X2*pi/180); // Horizontal Disance 2
                        int Vert2 = Dist2*cos(Y2*pi/180);
                        int aimAngle = (X1 + X2)/2;
                        int launchDist = (Horz1 + Horz2)/2;
                        int launchElevation = 0;
                        int LR = LimitRight;
        
                        int actualAngle = counting/3;

                        if(actualAngle<aimAngle){
                            xAxisControl = 1;
                            Launch.stepperSControl(xAxisControl);
                            counting++;
                        }else if(actualAngle>aimAngle){
                            xAxisControl = 0;
                            Launch.stepperSControl(xAxisControl);
                            counting--;
                        }
                            //printf("Aim %d :  Actual %d\n", actualAngle,aimAngle);
                    }
                }
                
            
                
                        
                
              /*  
                if(LimitLeft == 0){
                    flip = 2;
                }else if(LimitRight == 0){
                    flip = 0;
                }
                
                if(flip == 1){
                    xAxisControl = 1;
                }else if(flip == 2){
                    xAxisControl = 0;
                    counting++;
                    printf("count = %d",counting);
                
                }
                //if(0){
                    float pi = 3.14159265;

                    int X1 = Turret1.posX * 90 + 45;    //Raw X Data 1 (0-1)
                    float Y1 = Turret1.posY;      //Raw Y Data 1 (0-1)

                    int X2 = Turret2.posX * 90 + 45;  //Raw X Data 2(0-1)
                    float Y2 = Turret2.posY;  //Raw X Data 2 (0-1)
                    
                   // X1 = pi*X1/180; //Converts 0-1 range to 0-pi rads (0-90 deg)
                    Y1 = 90*pi*Y1/180;
                   // X2 = 90*pi*X2/180;
                    Y2 = 90*pi*Y2/180;

                    int Dist1 = Turret1.lastDist1;  //Total Distance 1
                    int Dist2 = Turret2.lastDist2;  //Total Distance 2
                    //printf("X1 %d at %dcm  :   X2 %d at %dcm \n",X1,Dist1,X2,Dist2);

                    int Horz1 = Dist1*sin(Y1); // Horizontal Disance 1
                    int Horz2 = Dist2*sin(Y2); // Horizontal Disance 2

                    int Xc = 28;        //Distance between centre and a turret
                    int Yc = (Xc + (Horz1*(cos((pi-X1)))) + (Horz2*(cos(pi-X2))))/2; //Middle of rope
                    int Z = Xc - Yc;        //X distance between centre of launcher and centre of rope

                    float avgHeight = ((Horz1*(sin(pi-X1))) + (Horz2*(sin(pi-X2))))/2; //Average height value for trapezium
                    float launchAngle = atan(avgHeight / Z) * (180/pi);        //Angle between two centres, converted to Degrees
                    //printf("Buoys are %d cm apart\n", Yc*2);
                    //printf("Launcher %f",launchAngle);

                ThisThread::sleep_for(100ms);*/
                //Launch.stepperSControl(xAxisControl);
                //Launch.servoSControl(yAxisControl);
                //Launch.triggerSControl(readyToFire);
                //Launch.safetySControl(safteyControl);
                //}
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
    Queue_Turret1.call_every(10ms, Turret1Func);
    Thread_Turret1.start(callback(&Queue_Turret1, &EventQueue::dispatch_forever));
    //IR 1
    Queue_Cord1.call_every(80ms, IRSensor1);
    Thread_Cord1.start(callback(&Queue_Cord1, &EventQueue::dispatch_forever));
    //ToF 1
    Queue_Dist1.call_every(5ms, DistAvg1);
    Thread_Dist1.start(callback(&Queue_Dist1, &EventQueue::dispatch_forever));
    //Turret 2

    Queue_Turret2.call_every(10ms, Turret2Func);
    Thread_Turret2.start(callback(&Queue_Turret2, &EventQueue::dispatch_forever));
    //IR 2
    Queue_Cord2.call_every(80ms, IRSensor2);
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