#ifndef MAIN_H
#define MAIN_H

#include "mbed.h"

void Write_2bytes(char d1, char d2, int IRsensorAddress);
int Distance1();
int Distance2();
void Dist_Avg1();
void Dist_Avg2();
int* combineNumbers(int num1, int num2);
void IR_Sensor1();
void IR_Sensor2();
void Turret1_Function();
void Turret2_Function();

float map(float in, float inMin, float inMax, float outMin, float outMax);
void stepperRCControl();

/* //Variables for Turret
int dist1 = 0;
int avgDist1 = 0;
int lastDist1;
int dist2 = 0;
int avgDist2 = 0;
int lastDist2 = 0;
int lockON1 = 0;
int lockON2 = 0;
int sampleSize = 5;
int IRsensorAddress = 0xB0;
int tolerance = 5;
int flashHz = 20; 
int Ix1[4];
int Iy1[4];
int Ix_prev1[4];
int Iy_prev1[4];
int flashCount1[4] = {0,0,0,0};
int s1;
int locate1 = 0;
int lost1 = 0;
int sweep = 0;
int Ix2[4];
int Iy2[4];
int Ix_prev2[4];
int Iy_prev2[4];
int flashCount2[4] = {0,0,0,0};
int s2;
int locate2 = 0;
int lost2 = 0;
char data_buf1[16];
char data_buf2[16];
int* coordinates1;
int* coordinates2;
float STEP = 0.0005;
*/

#endif