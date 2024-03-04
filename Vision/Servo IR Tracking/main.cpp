#include "mbed.h"
#include "Servo.h"
#include "stdio.h"
#include <cstdio>
//#include <string>
//#include <cmath>
//#include <memory>
#include <iostream>

float STEP = 0.005;
using namespace std;

//int Cord1X1,Cord1X2,Cord1X3
int sampleSize = 5;

I2C i2c1(PB_9, PB_8);
BufferedSerial pc(USBTX, USBRX);
BufferedSerial lidarSerial(D1, D0);


Servo servoX1(D9);
Servo servoY1(D6);

Timer tick;
Thread Thread_Turret1;
EventQueue Queue_Turret1(1 * EVENTS_EVENT_SIZE);


Thread Thread_ToF;
EventQueue Queue_ToF(1 * EVENTS_EVENT_SIZE);


void Write_2bytes(char d1, char d2, int IRsensorAddress) {
    char data[2] = {d1, d2};
    if (IRsensorAddress == 0xB0) {
        i2c1.write(IRsensorAddress, data, 2);
    }
}

class Turret {
private:
    int turretID;
    int flip = 1;
    int ignoreReads = 0;
    int fail = 0;
    int averageDistance = 0;
    int lastDistance = 0;
    float posX = 0.5; 
    float posY = 0.5;
    char data_buf[16];
    int Ix[4];
    int Iy[4];
    int s;
public:
    // Constructor
    Turret(int turretID) : turretID(turretID){
        if(turretID == 1){
            Write_2bytes(0x30, 0x01, 0xB0); ThisThread::sleep_for(10ms);
            Write_2bytes(0x30, 0x08, 0xB0); ThisThread::sleep_for(10ms);
            Write_2bytes(0x06, 0x90, 0xB0); ThisThread::sleep_for(10ms);
            Write_2bytes(0x08, 0xC0, 0xB0); ThisThread::sleep_for(10ms);
            Write_2bytes(0x1A, 0x40, 0xB0); ThisThread::sleep_for(10ms);
            Write_2bytes(0x33, 0x33, 0xB0); ThisThread::sleep_for(10ms);
        ThisThread::sleep_for(100ms);
        }
    }
    // The I2C object is automatically deleted when i2c is deleted
    ~Turret(){
    }  

    // Copy constructor (deleted)
    //Turret(const Turret&) = delete;

    // Copy assignment operator (deleted)
    //Turret& operator=(const Turret&) = delete;

    void sweep(void) {
        if (flip == 1) {
            posY = 0.6;
            posX = posX + STEP;
        } else {
            posX = posX - STEP;
            posY = 0.4;
        }

        if (posX <= 0.0) {
            flip = 1;
            cout<<"No Buoy Found"<<endl;
        } else if (posX >= 1) {
            flip = -1;
        }

        if (turretID == 1) {
            servoX1 = posX;
            servoY1 = posY;
        }
    }

    // Method to move the turret to a specific X-Y position
    void Track(int X, int Y, int tolerance) {
        float trackSpeedX = (((512.000000 - X)) / 150000);
        float trackSpeedY = (((512.000000 - Y)) / 150000);

        if (trackSpeedX > 0.003) {
            trackSpeedX = 0.005;
        } else if (trackSpeedX < -0.003) {
            trackSpeedX = -0.005;
        }
        if (trackSpeedY > 0.003) {
            trackSpeedY = 0.005;
        } else if (trackSpeedY < -0.003) {
            trackSpeedY = -0.005;
        }

        posX -= trackSpeedX; 
        posY += trackSpeedY;

        if (abs(512 - X) > tolerance) {
            if (turretID == 1) {
                servoX1 = posX;
            }
        }
        if (abs(512 - Y) > tolerance) {          
            if (turretID == 1) {
                servoY1 = posY;
            }     
        }
    }

    uint16_t Distance() {
        uint8_t buf[9] = {0}; // An array that holds data
        int lastdistance;
        if (turretID == 1) {
            if (lidarSerial.readable()) {
                for (int j = 0; j < sampleSize; j++) {
                    lidarSerial.read(buf, 9); // Read 9 bytes of data
                    
                    //check = buf[0] + buf[1] + buf[2] + buf[3] + buf[4] + buf[5] + buf[6] + buf[7];
                
                    if (buf[0] == 0x59 && buf[1] == 0x59) {
                        uint16_t distance = buf[2] + buf[3] * 256;
                        cout << "Reading TF-Luna" << endl;
                        lastdistance = distance;
                        if (distance != 0) {
                            averageDistance += distance;
                        } else {
                            return 0;
                        }
                    
                        if (j == sampleSize) {
                            if (ignoreReads < 1) {
                                ignoreReads++;
                            } else {
                                averageDistance /= sampleSize;
                                lastdistance = averageDistance;
                            }
                            averageDistance = 0;
                        }
                        
                    }
                }
            } else {
                return -1;
            }
        }
        memset(buf, 0, sizeof(buf));
        ThisThread::sleep_for(1ms);
        return lastdistance;
    }


    int* combineNumbers(int num1, int num2) {
        static int Temp[2];
        Temp[1] = num1;
        Temp[0] = num2;

        return Temp;
    }

    int* IR_Sensor(int IRAddress) {
        //if (turretID == 1) {
            i2c1.write(IRAddress, "\x36", 1);  // Send the register address to read
            i2c1.read(IRAddress, data_buf, 16);  // Read 16 bytes
        //}
        Ix[0] = data_buf[1];
        Iy[0] = data_buf[2];
        s     = data_buf[3];
        Ix[0] += (s & 0x30) << 4;
        Iy[0] += (s & 0xC0) << 2;
    
        Ix[1] = data_buf[4];
        Iy[1] = data_buf[5];
        s     = data_buf[6];
        Ix[1] += (s & 0x30) << 4;
        Iy[1] += (s & 0xC0) << 2;

        Ix[2] = data_buf[7];
        Iy[2] = data_buf[8];
        s     = data_buf[9];
        Ix[2] += (s & 0x30) << 4;
        Iy[2] += (s & 0xC0) << 2;

        Ix[3] = data_buf[10];
        Iy[3] = data_buf[11];
        s     = data_buf[12];
        Ix[3] += (s & 0x30) << 4;
        Iy[3] += (s & 0xC0) << 2;

        int* coordinates = combineNumbers(Ix[0], Iy[0]);
        return coordinates;
    }
};

extern Turret Turret1;
class Turret;
Turret Turret1(1);

void Turret1_Function() {

    int* Coordinates = Turret1.IR_Sensor(0xB0);

    int X = Coordinates[0];
    int Y = Coordinates[1];
    static int fail;

    if (X == 0 && Y == 0) {
        cout<<"Error: No IR Sensor Detected"<<endl;
    } else if (X == 1023 && Y == 1023) {
        if (fail <= 199) {
            fail++;
        } else if (fail == 200) {
            cout<<"No Buoy Found"<<endl;
            fail++;
        } else {
            Turret1.sweep();
        }
    }else{
            fail = 0;
            Turret1.Track(X, Y, 5);
            uint16_t distance = Turret1.Distance();
            if (distance == 10000) {
                cout<<"TOF Unreadable"<<endl;
            }else if (distance == 0){
                cout<<"ERROR: TOF reading 0"<<endl;
            }else{
            cout<<"Distance is " << distance << " cm"<<endl;
            }
        }
        ThisThread::sleep_for(1ms);
}


int lastDistance = 0;
int averageDistance = 0;
int ignoreReads = 0;
int j = 0;

void Dist_Function(int turretID){
    uint8_t buf[9] = {0}; // An array that holds data
        if (lidarSerial.readable()) {
            lidarSerial.read(buf, 9); // Read 9 bytes of data
            if (buf[0] == 0x59 && buf[1] == 0x59) {
                uint16_t distance = buf[2] + buf[3] * 256;
                if(distance != 0){
                    averageDistance += distance;
	                j++;
                }
            if(j == sampleSize){
		        averageDistance /= sampleSize;
                if(ignoreReads<1){
                    ignoreReads++;
                }else{
		            cout<<averageDistance<<endl;
                }
                j = 0;
		        averageDistance = 0;
	        }
        }
    }
    memset(buf,0,sizeof(buf));
    ThisThread::sleep_for(1ms);
}

void ToF_Function(){
    Dist_Function(1);
    //ThisThread::sleep_for(5ms);
}

int main() {
    pc.set_baud(115200);
    //i2c1.frequency(100000);
    //
    lidarSerial.set_baud(115200); // Set baud rate for Lidar serial communication
    i2c1.frequency(115200);

    Write_2bytes(0x30, 0x01, 0xB0); ThisThread::sleep_for(10ms);
    Write_2bytes(0x30, 0x08, 0xB0); ThisThread::sleep_for(10ms);
    Write_2bytes(0x06, 0x90, 0xB0); ThisThread::sleep_for(10ms);
    Write_2bytes(0x08, 0xC0, 0xB0); ThisThread::sleep_for(10ms);
    Write_2bytes(0x1A, 0x40, 0xB0); ThisThread::sleep_for(10ms);
    Write_2bytes(0x33, 0x33, 0xB0); ThisThread::sleep_for(10ms);
        
    ThisThread::sleep_for(100ms);
    //tick.start();
    //Queue_ToF.call_every(60ms, ToF_Function);

    //Thread_ToF.start(callback(&Queue_ToF, &EventQueue::dispatch_forever));
    while(true){
    uint8_t buf[9] = {0}; // An array that holds data
        if (lidarSerial.readable()) {
            lidarSerial.read(buf, 9); // Read 9 bytes of data
            cout<<"HERE"<<endl;
            if (buf[0] == 0x59 && buf[1] == 0x59) {
                cout<<"NEVER ACTUALLY HERE"<<endl;
                uint16_t distance = buf[2] + buf[3] * 256;
                if(distance != 0){
                    averageDistance += distance;
	                j++;
                }
            if(j == sampleSize){
		        averageDistance /= sampleSize;
                if(ignoreReads<1){
                    ignoreReads++;
                }else{
		            cout<<averageDistance<<endl;
                }
                j = 0;
		        averageDistance = 0;
	        }
        }
    }
    memset(buf,0,sizeof(buf));
    wait_us(10000);
    }
    //Queue_Turret1.call_every(49ms, Turret1_Function);

    //Thread_Turret1.start(callback(&Queue_Turret1, &EventQueue::dispatch_forever));
    

    // IR sensor initialize

}
/*
    while (1) {
        int Ix[4];
        int Iy[4];
        int IRAddress = 0xB0;
        int s;
        char data_buf[16];

        i2c1.write(IRAddress, "\x36", 1);  // Send the register address to read
        i2c1.read(IRAddress, data_buf, 16);  // Read 16 bytes
        //}
        Ix[0] = data_buf[1];
        Iy[0] = data_buf[2];
        s     = data_buf[3];
        Ix[0] += (s & 0x30) << 4;
        Iy[0] += (s & 0xC0) << 2;
    
        Ix[1] = data_buf[4];
        Iy[1] = data_buf[5];
        s     = data_buf[6];
        Ix[1] += (s & 0x30) << 4;
        Iy[1] += (s & 0xC0) << 2;

        Ix[2] = data_buf[7];
        Iy[2] = data_buf[8];
        s     = data_buf[9];
        Ix[2] += (s & 0x30) << 4;
        Iy[2] += (s & 0xC0) << 2;

        Ix[3] = data_buf[10];
        Iy[3] = data_buf[11];
        s     = data_buf[12];
        Ix[3] += (s & 0x30) << 4;
        Iy[3] += (s & 0xC0) << 2;

        cout<<Ix[0]<<"    "<<Iy[]<<endl;
        ThisThread::sleep_for(1000ms);  // Add a sleep to avoid busy-waiting
    }

    // No need for manual deletion of Turret1 and i2c, they will be automatically handled by shared pointers

    return 0;
}
*/