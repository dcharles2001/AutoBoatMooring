#include "mbed.h"
#include "Servo.h"
#include <cstdio>
I2C i2c(PB_9, PB_8);  // Set I2C pins

const int IRsensorAddress = 0xB0; //>> 1;
const int ledPin = D13;
bool ledState = false;
char data_buf[16];
int Ix[4];
int Iy[4];
int Ix_prev[4];
int Iy_prev[4];

int s;
int direct = 1;
float Sweep = 0.5;
float pos = 0.5;

int lockON1 = 0;
int flashHz = 20;
int tolerance = 50;

int flashCount[4] = {0,0,0,0};

Servo myservo(D9);
void sweep();

void Write_2bytes(char d1, char d2)
{
    char data[2] = {d1, d2};
    i2c.write(IRsensorAddress, data, 2);
}


int main()
{
    // IR sensor initialize
    Write_2bytes(0x30, 0x01); ThisThread::sleep_for(10ms);
    Write_2bytes(0x30, 0x08); ThisThread::sleep_for(10ms);
    Write_2bytes(0x06, 0x90); ThisThread::sleep_for(10ms);
    Write_2bytes(0x08, 0xC0); ThisThread::sleep_for(10ms);
    Write_2bytes(0x1A, 0x40); ThisThread::sleep_for(10ms);
    Write_2bytes(0x33, 0x33); ThisThread::sleep_for(10ms);
    ThisThread::sleep_for(100ms);

    while (1) {
        // IR sensor read
    static int fail = 0;
    static int reading = 0;

        i2c.write(IRsensorAddress, "\x36", 1);  // Send the register address to read
        i2c.read(IRsensorAddress, data_buf, 16);  // Read 16 bytes

        Ix[0] = data_buf[1];
        Iy[0] = data_buf[2];
        s   = data_buf[3];
        Ix[0] += (s & 0x30) <<4;
        Iy[0] += (s & 0xC0) <<2;

        Ix[1] = data_buf[4];
        Iy[1] = data_buf[5];
        s   = data_buf[6];
        Ix[1] += (s & 0x30) <<4;
        Iy[1] += (s & 0xC0) <<2;

        Ix[2] = data_buf[7];
        Iy[2] = data_buf[8];
        s   = data_buf[9];
        Ix[2] += (s & 0x30) <<4;
        Iy[2] += (s & 0xC0) <<2;

        Ix[3] = data_buf[10];
        Iy[3] = data_buf[11];
        s   = data_buf[12];
        Ix[3] += (s & 0x30) <<4;
        Iy[3] += (s & 0xC0) <<2;
        for(int measurements = 0; measurements<12; measurements++){
            for(int i = 0; i < 3; i++){
                if((Ix[i] == 1023 && Ix_prev[i] != 1023) || ((Ix[i] != 1023 && Ix_prev[i] == 1023))){
                flashCount[i] = flashCount[i] + 1;
            }
            Ix_prev[i] = Ix[i];
            Iy_prev[i] = Iy[i];

            i2c.write(IRsensorAddress, "\x36", 1);  // Send the register address to read
            i2c.read(IRsensorAddress, data_buf, 16);  // Read 16 bytes

            Ix[0] = data_buf[1];
            Iy[0] = data_buf[2];
            s   = data_buf[3];
            Ix[0] += (s & 0x30) <<4;
            Iy[0] += (s & 0xC0) <<2;

            Ix[1] = data_buf[4];
            Iy[1] = data_buf[5];
            s   = data_buf[6];
            Ix[1] += (s & 0x30) <<4;
            Iy[1] += (s & 0xC0) <<2;

            Ix[2] = data_buf[7];
            Iy[2] = data_buf[8];
            s   = data_buf[9];
            Ix[2] += (s & 0x30) <<4;
            Iy[2] += (s & 0xC0) <<2;

            Ix[3] = data_buf[10];
            Iy[3] = data_buf[11];
            s   = data_buf[12];
            Ix[3] += (s & 0x30) <<4;
            Iy[3] += (s & 0xC0) <<2;
            ThisThread::sleep_for(30ms);
            }
        }
        if(flashCount[0]>3 && flashCount[0] < 8){
                printf("Slow Buoy Found 1st     ");
            }else if(flashCount[0] > 6 && flashCount[0] < 12){
                printf("Fast Buoy Found 1st     ");
            }else if (flashCount[0] < 4){
                printf("Reflection Found 1st        ");
            }
        if(flashCount[1]>3 && flashCount[1] < 8){
                printf("Slow Buoy Found 2nd\n");
            }else if(flashCount[1] > 6 && flashCount[1] < 12){
                printf("Fast Buoy Found 2nd\n");
            }else if (flashCount[1] < 4){
                printf("Reflection Found 2nd\n");
            }
        //printf("%d : %d\n", Ix[0], flashCount);
        for(int i = 0; i < 3; i++){
            flashCount[i] = 0;
        }
        //ThisThread::sleep_for(1ms);
    }
}

