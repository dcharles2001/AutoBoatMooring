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
        int lockBuoy = 0;
        int lost = 0;
        int lastCount = 0;
        float STEP = 0.003;
        int lockON = 0;
        int locate = 0;

        int avgDist = 0,sampleSize = 5,IRAddress = 0xB0,tolerance = 1,flashHz = 20,/*sweep = 0,*/Ix[4],Iy[4],Ix_prev[4],Iy_prev[4],flashCount[4] = {0,0,0,0},s;
        char data_buf[16];
        int* coordinates;
      
        float centreX;
        float centreY;
        float posX = 0.5; 
        float posY = 0.5;
        int lastDist = 0;
        void sweep(void);
        // Method to move the turret to a specific X-Y position
        void Track(int X, int Y);
        void ToF_Function(int BuoyID);
        int* combineNumbers1(int num1, int num2);
        int* combineNumbers2(int num1, int num2);
        int Distance();
        /*
        int dist1 = 0,dist2 = 0,avgDist1 = 0,avgDist2 = 0,lastDist1,lastDist2 = 0,lockON1 = 0,lockON2 = 0,sampleSize = 5,IRsensorAddress = 0xB0,tolerance = 5,flashHz = 20,Ix1[4],Iy1[4],Ix_prev1[4],Iy_prev1[4],flashCount1[4] = {0,0,0,0},s1,locate1 = 0,lost1 = 0,Ix2[4],Iy2[4],Ix_prev2[4],Iy_prev2[4],flashCount2[4] = {0,0,0,0},s2,locate2 = 0,lost2 = 0;
        char data_buf1[16],data_buf2[16];
        int* coordinates1;
        int* coordinates2;
        float STEP = 0.0005;
    */
    public:
        void Write_2bytes(char d1, char d2, int IRAddress);

        // Constructor
        Sensors(int turretID) : turretID(turretID){
            Write_2bytes(0x30, 0x01,  IRAddress); ThisThread::sleep_for(10ms);
            Write_2bytes(0x30, 0x08,  IRAddress); ThisThread::sleep_for(10ms);
            Write_2bytes(0x06, 0x90,  IRAddress); ThisThread::sleep_for(10ms);
            Write_2bytes(0x08, 0xC0,  IRAddress); ThisThread::sleep_for(10ms);
            Write_2bytes(0x1A, 0x40,  IRAddress); ThisThread::sleep_for(10ms);
            Write_2bytes(0x33, 0x33,  IRAddress); ThisThread::sleep_for(10ms);
            ThisThread::sleep_for(100ms);
        };

        int Target = 0;
        int Dist = 0;
        float Angle = 0;
        // The I2C object is automatically deleted when i2c is deleted
        void Setup();
        void Dist_Avg();
        void IR_Sensor();
        void Turret_Function();
    protected:
};

#endif