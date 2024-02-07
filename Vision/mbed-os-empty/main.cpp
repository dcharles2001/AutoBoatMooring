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
int s;
int direct = 1;
float Sweep = 0.5;
float pos = 0.5;

Servo myservo(D9);
void sweep();

void Write_2bytes(char d1, char d2)
{
    char data[2] = {d1, d2};
    i2c.write(IRsensorAddress, data, 2);
}

void Track(int X,int Y,int tolerance){                     //Ix[i],Iy[i], how close can the coordinate be to central
    if (X != 1023 && Y != 1023){
        if(X<((1024/2) - tolerance)){
            myservo = pos;
            pos+=0.002;
        }else if(X>((1024/2) + tolerance)){
            myservo = pos;
            pos-=0.002;
        }
   }else{
       sweep();
   }
}

void sweep(void){
    if (direct == 1){
        Sweep -= 0.01;        //ThisThread::sleep_for(200ms);
        if(Sweep == 0){
            direct = 0;
        }
    }else{
        if(Sweep<1){
            Sweep +=0.01;
        }else{
            direct = 1;
        }
    }
    myservo = Sweep;
}


int main()
{
    while(1){
        sweep();
        printf("%f",Sweep);
        ThisThread::sleep_for(100ms);
    }
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

        for (int i = 0; i < 4; i++) {
            if (Ix[i] < 1000)
                printf("");
            if (Ix[i] < 100)
                printf("");
            if (Ix[i] < 10)
                printf("");
            printf("%d,", int(Ix[i]));

            if (Iy[i] < 1000)
                printf("");
            if (Iy[i] < 100)
                printf("");
            if (Iy[i] < 10)
                printf("");
            printf("%d", int(Iy[i]));

            if (i < 3)
                printf(",");
        }
        printf("\n");
        ThisThread::sleep_for(30ms);
        Track(Ix[0],Iy[0],20);
        ThisThread::sleep_for(10ms);
    }
}