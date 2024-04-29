#include "mbed.h"
#include "Servo.h"
#include "stdio.h"
#include <cstdio>
#include <iostream>
#include "PwmIn.h"
#include "Launcher.h"
#include "Sensors.hpp"
#include "BuoyComms.h"
using namespace std;
Launcher Launch;
BuoyComms Boat(f429spi1, F4Zeta, BOAT); //boat comms object

Sensors Turret1(1);
Sensors Turret2(2);

DigitalIn LimitRight(PD_1);
DigitalIn LimitLeft(PG_0);

//Thread Creation and Event queue for Sensor Turrets

Thread PrintThread;
EventQueue PrintQueue;

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
DigitalIn GrnBtn(PE_13);
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
int GrnBtnTog = 0;
int GrnBtnCount = 0;
int countCounter = 150;
int GrnBtn_Prev = 0;
int callibrate = 0;
int counting = 0;
int limit = 0;
bool GrnBtnPress = 0;
float Yangle;
float desiredYangle;


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
                
                float location = Launch.servoLocation();
                //printf("IM IN 1\n");
                //printf("location %d\n",location);
                if(callibrate == 0){
                    xAxisControl = 0;
                    Launch.stepperSControl(xAxisControl);
                    if(LimitRight == 0){
                        callibrate = 1;
                        limit = 0;
                    }
                }

                if(callibrate == 1){
                    xAxisControl = 1;
                    limit++;
                    Launch.stepperSControl(xAxisControl);
                    if(LimitLeft == 0){
                        counting = limit;
                        callibrate = 2;
                    }
                }

                if(callibrate == 2 ){
                    xAxisControl = 0;
                    Launch.stepperSControl(xAxisControl);
                    counting--;
                    if(counting == limit/2){
                        callibrate = 3;
                    }
                }   

                if(callibrate == 3){
                    Yangle = Launch.servoLocation() * 90;
                    if(Yangle == 0){
                        Yangle = 44;
                    }
                    desiredYangle = 4;
                    callibrate = 4;
                }
                if(callibrate == 4){
                    float min = 0.1;
                    //printf("%f  :   %f\n",Yangle,desiredYangle);
                    if(Yangle>desiredYangle + 0.2){
                        Yangle -= min;
                        Launch.servoSControl(Yangle/44 * 0.495);
                        //ThisThread::sleep_for(1ms);
                    }else if(Yangle<desiredYangle - 0.2){
                        Yangle += min;
                        Launch.servoSControl(Yangle);
                        //ThisThread::sleep_for(1ms);
                    }else{
                        callibrate = 5;
                        PrintQueue.call(printf,"DONE CALIBRATING");
                    }
                    Launch.servoSControl(Yangle/44*0.495);
                }
                PrintQueue.call(printf,"Servo Angle: %d\n", Yangle);
                if(callibrate == 4){
                    float min = 0.0001;
                    if(Yangle>desiredYangle){
                        //Yangle -= min;
                        //Launch.servoSControl(Yangle);
                        //ThisThread::sleep_for(100ms);
                    }else if(Yangle<desiredYangle){
                        //Yangle += min;
                        //Launch.servoSControl(Yangle);
                        //ThisThread::sleep_for(100ms);
                    }else{
                        callibrate = 5;
                    }
                }
                if(callibrate == 5){
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
        
                        int actualAngle = counting/(limit/88); //step range divided by degree range

                        if(actualAngle<aimAngle){
                            xAxisControl = 0;
                            Launch.stepperSControl(xAxisControl);
                            counting++;
                        }else if(actualAngle>aimAngle){
                            xAxisControl = 1;
                            Launch.stepperSControl(xAxisControl);
                            counting--;
                        }else{
                            desiredYangle = 30;
                            float min = 0.1;
                            if(Yangle>desiredYangle){
                                Yangle -= min;
                                Launch.servoSControl(Yangle/44 * 0.495);
                                //ThisThread::sleep_for(1ms);
                            }else if(Yangle<desiredYangle){
                                Yangle += min;
                                Launch.servoSControl(Yangle/44*0.495);
                                //ThisThread::sleep_for(1ms);
                            }else{
                                if(Launch.Cha1Read <= 1100 && GrnBtnPress == 1){
                                    //printf("AUTO FIRE PEW PEW\n");
                                }
                            }
                        }
                            //printf("Aim %d :  Actual %d\n", actualAngle,aimAngle);
                    }
                }
                
                
            
                
                        
                
                
               
                //ThisThread::sleep_for(100ms);
                //Launch.stepperSControl(xAxisControl);
                //Launch.servoSControl(yAxisControl);
                //Launch.triggerSControl(readyToFire);
                //Launch.safetySControl(safteyControl);
                //}
                
                
            }else if(Swt2Conf == 1){
                callibrate = 0;
                Launch.stepperRCControl();
                Launch.servoRCControl();
                Launch.triggerRCControl();
                Launch.safetyRCControl();
            }else if(stopConf == 1){
                callibrate = 0;
                //Launch.servoSControl(yAxisControl);
                Launch.triggerSControl(readyToFire);
                Launch.safetySControl(safteyControl);
                Launch.stepperSControl(3);
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

void Printer(void);

//MAIN
int main(){
    
    //Sensor Code
    
    //------COMMENT BELOW OUT FOR MANUAL USE
    PrintThread.start(Printer);

/*
    Boat.Init();
    bool packetresp = 1;

    unsigned char partinfo[8];
    Boat.GetPartInfo(partinfo);
    for(int i=0; i<8; i++)
    {
        PrintQueue.call(printf,"Part: %x\n\r", partinfo[i]);
    }

    Buoycmd_t newcmd = {ON, 255}; //turn on for 50 seconds
    unsigned char TestMessage[RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH]; //new message
    Boat.InstructionConfigurator(newcmd, TestMessage, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH);
    
    
    while(packetresp)
    {
        
        unsigned char buoyresponse[RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH];
        
        Boat.MessageWaitResponse(TestMessage);
        //still in receive mode
        ThisThread::sleep_for(100ms); //essential delay
        for(int i=0; i<2; i++)
        {
            if(Boat.ReceiveAndRead(buoyresponse, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH))
            {
                PrintQueue.call(printf,"No response\n\r");
                packetresp = 1;
                break;
                
            }else
            {
                for(int j=0; j<RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH; j++)
                {
                    PrintQueue.call(printf,"RX: %c\n\r", buoyresponse[j]);
                }
                if(!Boat.InterpretResponse(buoyresponse)) //good packet check
                {
                    PrintQueue.call(printf,"Resp success\n\r");
                    packetresp = 0;
                }
            }
            ThisThread::sleep_for(2s); //wait for second buoy delay
        }
    }
    */
    //----- COMMENT ABOVE OUT FOR MANUAL USE
    
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

        int swt1 = Swt1;
        int swt2 = Swt2;
        int grnbtn = GrnBtn;
        //printf("swt1conf %d : swt2conf %d   :   stop %d\n",Swt1Conf,Swt2Conf,stopConf);
        //printf("swt1 %d : swt2 %d\n",swt1,swt2);
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
                Swt1Count = 0;

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
                Swt2Count = 0;

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
                stopCount = 0;
                //printf("Stopped\n");
            }
        }
        
        if(GrnBtn == 0 && GrnBtnTog == 0){
            if(GrnBtn_Prev != GrnBtn){
                GrnBtnCount = 0;
            }
            GrnBtn_Prev = GrnBtn;
            GrnBtnCount ++;
            if(GrnBtnCount >= countCounter/10){
                GrnBtnPress = !GrnBtnPress;
                GrnBtnTog = 1;  
                GrnBtnCount = 0;
                //printf("Green Button %d\n",GrnBtnPress);
            }
        }else if (GrnBtn == 1){
                GrnBtnTog = 0;
                GrnBtnCount = 0;
            }
        }
        
    }

void Printer(void)
{
    PrintQueue.dispatch_forever(); //dispatches print calls
}