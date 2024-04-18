#ifndef SENSORS_H
#define SENSORS_H

#include "mbed.h"
#include "stdio.h"
#include <cstdio>
#include <iostream>
#include "Servo.h"
using namespace std;

class Sensors{
   private:
        int turretID;
        int flip = 1;
        int fail = 0;
        //float posX = 0.5; 
        //float posY = 0.5;
        int dist = 0;
        int lockBuoy = 0;
 /*
        int dist1 = 0,dist2 = 0,avgDist1 = 0,avgDist2 = 0,lastDist1,lastDist2 = 0,lockON1 = 0,lockON2 = 0,sampleSize = 5,IRsensorAddress = 0xB0,tolerance = 5,flashHz = 20,Ix1[4],Iy1[4],Ix_prev1[4],Iy_prev1[4],flashCount1[4] = {0,0,0,0},s1,locate1 = 0,lost1 = 0,Ix2[4],Iy2[4],Ix_prev2[4],Iy_prev2[4],flashCount2[4] = {0,0,0,0},s2,locate2 = 0,lost2 = 0;
        char data_buf1[16],data_buf2[16];
        int* coordinates1;
        int* coordinates2;
        float STEP = 0.0005;
    */
    public:
        void Write_2bytes(char d1, char d2, int IRsensorAddress);

        // Constructor
        Sensors(int turretID) : turretID(turretID){
            Write_2bytes(0x30, 0x01,  IRsensorAddress); ThisThread::sleep_for(10ms);
            Write_2bytes(0x30, 0x08,  IRsensorAddress); ThisThread::sleep_for(10ms);
            Write_2bytes(0x06, 0x90,  IRsensorAddress); ThisThread::sleep_for(10ms);
            Write_2bytes(0x08, 0xC0,  IRsensorAddress); ThisThread::sleep_for(10ms);
            Write_2bytes(0x1A, 0x40,  IRsensorAddress); ThisThread::sleep_for(10ms);
            Write_2bytes(0x33, 0x33,  IRsensorAddress); ThisThread::sleep_for(10ms);
            ThisThread::sleep_for(100ms);
        };

        int locatedBuoy = 0;
        float posX = 0.5; 
        float posY = 0.5;
        int lastDist1 = 0,lastDist2 = 0,lockON1 = 0,lockON2 = 0,Target = 0;
        // The I2C object is automatically deleted when i2c is deleted
        void Setup();
        void sweep(void);
        // Method to move the turret to a specific X-Y position
        void Track(int X, int Y, int tolerance);
        void ToF_Function(int BuoyID);
        void Dist_Avg1();
        void Dist_Avg2();
        void IR_Sensor1();
        void IR_Sensor2();
        void Turret_Function1();
        void Turret_Function2();
        int Distance1();
        int Distance2();
        int* combineNumbers(int num1, int num2);
    protected:
        int dist1 = 0,dist2 = 0,avgDist1 = 0,avgDist2 = 0,sampleSize = 5,IRsensorAddress = 0xB0,tolerance = 5,flashHz = 20,/*sweep = 0,*/Ix1[4],Iy1[4],Ix_prev1[4],Iy_prev1[4],flashCount1[4] = {0,0,0,0},s1,locate1 = 0,lost1 = 0,Ix2[4],Iy2[4],Ix_prev2[4],Iy_prev2[4],flashCount2[4] = {0,0,0,0},s2,locate2 = 0,lost2 = 0;
        char data_buf1[16],data_buf2[16];
        int* coordinates1;
        int* coordinates2;
        float STEP = 0.00075;
};

#endif