#include "mbed.h"
#include "Servo.h"
#include <cstdio>
I2C i2c(PB_9, PB_8);  // Set I2C pins


BufferedSerial pc(USBTX, USBRX); // Define serial object with USBTX and USBRX
BufferedSerial lidarSerial(D1, D0); // Define serial object with the appropriate pins

const int IRsensorAddress = 0xB0; //>> 1;
const int ledPin = D13;
bool ledState = false;
char data_buf[16];
int Ix[4];
int Iy[4];
int s;
int direct = 1;
int X = 1;
int sampleSize = 100;
int readingsReq = 10;


float step = 0.01;
float trackSpeedX = 0.0025;
float trackSpeedY = 0.0025;

float posX = 0.5;
float posY = 0.5;

Servo servoX(D9);
Servo servoY(D6);

void sweep();

void Write_2bytes(char d1, char d2)
{
    char data[2] = {d1, d2};
    i2c.write(IRsensorAddress, data, 2);
}

void Track(int X,int Y,int tolerance){                                          //Ix[i],Iy[i], how close can the coordinate be to central
    if (X != 1023 && Y != 1023){                                                //Determines if IR source Located
        if((X<((1024/2) - (2*tolerance)) || (X>((1024/2) + 2*tolerance)))){     //If far away, move faster
            trackSpeedX = 0.004;
        }else{
            trackSpeedX = 0.002;
        }

        if((Y<((1024/2) - (2*tolerance)) || (Y>((1024/2) + 2*tolerance)))){     //If far away, move faster
            trackSpeedY = 0.004;
        }else{
            trackSpeedY = 0.002;
        }

        servoX = posX;           
        servoY = posY;

        if(X<((1024/2) - tolerance)){                                           //Adjust servo position
            posX+=trackSpeedX;
        }else if(X>((1024/2) + tolerance)){
            posX-=trackSpeedX;
        }

        if(Y<((1024/2) - tolerance)){                                           //Adjust servo position
            posY+=trackSpeedY;
        }else if(Y>((1024/2) + tolerance)){
            posY-=trackSpeedY;
        }

   }else{
       sweep();                                                                 //If nothing found, sweep
   }
}

void sweep(void){

    if (X == 1){
        posY = 0.66;
        posX = posX + step;
    } else{
        posX = posX - step;
        posY = 0.33;
    }

    if (posX<=0.0){
        X=1;
    }else if(posX>=1){
        X=-1;
    }

    servoX = posX;
    servoY = posY;
}

void IR(){
    uint8_t buf[9] = {0}; // An array that holds data
        if (lidarSerial.readable()) {
            lidarSerial.read(buf, 9); // Read 9 bytes of data
            if (buf[0] == 0x59 && buf[1] == 0x59) {
                uint16_t distance = buf[2] + buf[3] * 256;
                if(distance != 0){
                    printf("Distance: %d cm\n", distance);
                }
                
            }
        }
        memset(buf,0,sizeof(buf));
        wait_us(1000);
}

int main()
{
    pc.set_baud(115200); // Set baud rate for PC serial communication
    lidarSerial.set_baud(115200); // Set baud rate for Lidar serial communication
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
    /*
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
    */
       // for (int i = 0; i < 4; i++) { 
           for (int i = 0; i <1; i++){
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

            if (i < 0)
                printf(",");
        }
        printf("\n");
        //ThisThread::sleep_for(30ms);
        Track(Ix[0],Iy[0],60);
        IR();
        ThisThread::sleep_for(5ms);
    }
}



/*

#include "mbed.h"

BufferedSerial pc(USBTX, USBRX); // Define serial object with USBTX and USBRX
BufferedSerial lidarSerial(D1, D0); // Define serial object with the appropriate pins
int main(){
    
    pc.set_baud(115200); // Set baud rate for PC serial communication
    lidarSerial.set_baud(115200); // Set baud rate for Lidar serial communication
    
    while(true){
        uint8_t buf[9] = {0}; // An array that holds data
        if (lidarSerial.readable()) {
            lidarSerial.read(buf, 9); // Read 9 bytes of data
            if (buf[0] == 0x59 && buf[1] == 0x59) {
                uint16_t distance = buf[2] + buf[3] * 256;
                if(distance != 0){
                    printf("Distance: %d cm\n", distance);
                }
                
            }
        }
        memset(buf,0,sizeof(buf));
        wait_us(1000); 
    }
}*/