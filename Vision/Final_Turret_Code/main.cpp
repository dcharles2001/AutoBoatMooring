
#include "mbed.h"
#include "Servo.h"
#include "stdio.h"
#include <cstdio>
#include <iostream>

Thread Thread_ToF1;
EventQueue Queue_ToF1(1 * EVENTS_EVENT_SIZE);

Thread Thread_Dist1;
EventQueue Queue_Dist1(1 * EVENTS_EVENT_SIZE);

Thread Thread_ToF2;
EventQueue Queue_ToF2(1 * EVENTS_EVENT_SIZE);

Thread Thread_Dist2;
EventQueue Queue_Dist2(1 * EVENTS_EVENT_SIZE);

I2C i2c1(PB_9, PB_8);
I2C i2c2(PB_11, PB_10);
BufferedSerial pc(USBTX, USBRX);
BufferedSerial lidarSerial1(D1, D0);
BufferedSerial lidarSerial2(PE_8, PE_7);


Servo servoX1(D5);
Servo servoY1(D6);

Servo servoX2(D9);
Servo servoY2(D10);

Timer tick;
Thread Thread_Turret1;
EventQueue Queue_Turret1(1 * EVENTS_EVENT_SIZE);

Thread Thread_Turret2;
EventQueue Queue_Turret2(1 * EVENTS_EVENT_SIZE);

using namespace std;

int dist1 = 0;
int avgDist1 = 0;
int lastDist1;
int lockON1 = 0;
int lockON2 = 0;
int dist2 = 0;
int avgDist2 = 0;
int lastDist2 = 0;

int sampleSize = 5;
int IRsensorAddress = 0xB0;
int tolerance = 5;
int flashHz = 20; 

float STEP = 0.004;


void Write_2bytes(char d1, char d2, int IRsensorAddress) {
    char data[2] = {d1, d2};
        i2c1.write(IRsensorAddress, data, 2);
        i2c2.write(IRsensorAddress, data, 2);
}

class Turret {
private:
    int turretID;
    int flip = 1;
    int fail = 0;
    float posX = 0.5; 
    float posY = 0.5;
    char data_buf[16];
    int Ix[4];
    int Iy[4];
    int s;
    int dist = 0;

public:
    // Constructor
    Turret(int turretID) : turretID(turretID){
            Write_2bytes(0x30, 0x01,  IRsensorAddress); ThisThread::sleep_for(10ms);
            Write_2bytes(0x30, 0x08,  IRsensorAddress); ThisThread::sleep_for(10ms);
            Write_2bytes(0x06, 0x90,  IRsensorAddress); ThisThread::sleep_for(10ms);
            Write_2bytes(0x08, 0xC0,  IRsensorAddress); ThisThread::sleep_for(10ms);
            Write_2bytes(0x1A, 0x40,  IRsensorAddress); ThisThread::sleep_for(10ms);
            Write_2bytes(0x33, 0x33,  IRsensorAddress); ThisThread::sleep_for(10ms);
            ThisThread::sleep_for(100ms);
    }
    // The I2C object is automatically deleted when i2c is deleted
    ~Turret(){
    } 

void sweep(void) {
        if (flip == 1) {
            posY = 0.6;
            posX = posX + STEP;
        } else {
            posX = posX - STEP;
            posY = 0.4;
        }

        if (turretID == 1) {
            if (posX <= 0.0) {
                flip = 1;
            } else if (posX >= 1) {
                flip = -1;
                printf("Turret 1 has no Buoy\n");
            }
                servoX1 = posX;
                servoY1 = posY;
        }else{
            if (posX <= 0.0) {
                flip = 1;
            } else if (posX >= 1) {
                flip = -1;
                printf("Turret 2 has no Buoy\n");
            }
                servoX2 = posX;
                servoY2 = posY;
        }
}


    // Method to move the turret to a specific X-Y position
    void Track(int X, int Y, int tolerance) {
        float trackSpeedX = (((415.000000 - X)) / 150000);
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

        if (abs(415 - X) > tolerance) {
            if (turretID == 1) {
                servoX1 = posX;
            }else{
                servoX2 = posX;
            }
        }else{
            if (abs(512 - Y) > tolerance) {          
                if (turretID == 1) {
                    servoY1 = posY;
                }else{
                    servoY2 = posY;
                }     
            }
        }
        if (abs(415 - X) <= tolerance && abs(512 - Y) <= tolerance){
            if(turretID == 1){
                lockON1 = 1;
            }else{
                lockON2 = 1;
            }
        }
    }
 int* combineNumbers(int num1, int num2) {
        static int Temp[2];
        Temp[1] = num1;
        Temp[0] = num2;

        return Temp;
    }

    int* IR_Sensor(int IRAddress) {
        if (turretID == 1) {
            i2c1.write(IRAddress, "\x36", 1);  // Send the register address to read
            i2c1.read(IRAddress, data_buf, 16);  // Read 16 bytes
        }else{
            i2c2.write(IRAddress, "\x36", 1);  // Send the register address to read
            i2c2.read(IRAddress, data_buf, 16);  // Read 16 bytes
        }
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

    void ToF_Function(int buoyID){
        if(turretID == 1){
            if(avgDist1 !=0){
                lastDist1 = avgDist1;
                printf("Buoy %d is %d cm away\n", buoyID, lastDist1);
            }else{
                printf("Last Buoy %d Distance was %d cm away\n",buoyID,lastDist1);
            }
        }else{
            if(avgDist2 !=0){
                lastDist2 = avgDist2;
                printf("Buoy %d is %d cm away\n",buoyID, lastDist2);
            }else{
                printf("Last Buoy %d Distance was %d cm away\n",buoyID,lastDist2);
            }

        }
    }
};

int Distance1(){
    uint8_t buf[9] = {0}; // An array that holds data
    int distance1 = 0;
    if (lidarSerial1.readable()) {
        lidarSerial1.read(buf, 9); // Read 9 bytes of data
        if (buf[0] == 0x59 && buf[1] == 0x59) {
            distance1 = buf[2] + buf[3] * 256;
        }
    }
    memset(buf, 0, sizeof(buf));
    return distance1;
}

int Distance2(){
    uint8_t buf[9] = {0}; // An array that holds data
    int distance2 = 0;
    if (lidarSerial2.readable()) {
        lidarSerial2.read(buf, 9); // Read 9 bytes of data
        if (buf[0] == 0x59 && buf[1] == 0x59) {
            distance2 = buf[2] + buf[3] * 256;
            if (distance2 != 0) {
            }
        }
    }
    memset(buf, 0, sizeof(buf));
    return distance2;
}


void Dist_Avg1() {
    int missed = 0;
    avgDist1 = 0;

    for(int i = 0; i<sampleSize; i++){
        dist1 = Distance1();
        if (dist1 != 0){
            avgDist1 += dist1;
        }else{
           missed++;
        }
    }
    avgDist1 /=(sampleSize - missed);
}

void Dist_Avg2() {
    int missed = 0;
    avgDist2 = 0;

    for(int i = 0; i<sampleSize; i++){
        dist2 = Distance2();
        if (dist2 != 0){
            avgDist2 += dist2;
        }else{
           missed++;
        }
    }
    avgDist2 /=(sampleSize - missed);
}



extern Turret Turret1;
class Turret;
Turret Turret1(1);

extern Turret Turret2;
class Turret;
Turret Turret2(2);


void Turret1_Function() {
    int* Coordinates = Turret1.IR_Sensor(IRsensorAddress);

    int X = Coordinates[0];
    int Y = Coordinates[1];
    static int fail = 0;
    static int reading = 0;

    if (X == 0 && Y == 0) {
        cout<<"Error:IR Sensor 1 Not Detected"<<endl;
        ThisThread::sleep_for(1000ms);
    } else if (X == 1023 && Y == 1023) {
        if (fail <= 199) {
            fail++;
        } else if (fail == 200) {
            fail++;
        } else {
            lockON1 = 0;
            Turret1.sweep();
        }
    }else{
        Turret1.Track(X,Y,tolerance);
        fail = 0;
        if(lockON1 == 1){
            int flashCount = 0;
            static int reflectCount = 0;
            for(int flash = 0; flash<3; flash++){
                int* Coordinates = Turret1.IR_Sensor(IRsensorAddress);
                int X = Coordinates[0];
                int Y = Coordinates[1];
                //printf("X = %d\n", X);
                //printf("Y = %d\n", Y);
	            if(abs(X - 415) <= (tolerance*10) && abs(Y - 512) <= (tolerance*10)){	//target found
		            flashCount++;
                }
                ThisThread::sleep_for(flashHz);
            }
            if(flashCount>2){
                if (reflectCount<3){
	                printf("Reflection Detected       %d\n",flashCount);
                    lockON1 == 0;
                }else if(reflectCount>3){
                    
                }
                reflectCount++;
            }else{
                if(reading == 200){
                    printf("Turret 1 found Buoy %d\n", flashCount);
                    Turret1.ToF_Function(flashCount);
                    reading = 0;
                }else{
                reading++;
                }
            }
            ThisThread::sleep_for(1ms);
        }   
    }
}


void Turret2_Function() {
    int* Coordinates = Turret2.IR_Sensor(IRsensorAddress);

    int X = Coordinates[0];
    int Y = Coordinates[1];
    static int fail = 0;
    static int reading = 0;

    if (X == 0 && Y == 0) {
        cout<<"Error: IR Sensor 2 Not Detected"<<endl;
        ThisThread::sleep_for(1000ms);
    } else if (X == 1023 && Y == 1023) {
        if (fail <= 199) {
            fail++;
        } else if (fail == 200) {
            fail++;
        } else {
            Turret2.sweep();
        }
    }else{
        Turret2.Track(X,Y,tolerance);
        fail = 0;
        if(reading == 200){
        Turret2.ToF_Function(1);
        reading = 0;
        }else{
            reading++;
        }
        ThisThread::sleep_for(1ms);
    }   
}

int main(){
    
    pc.set_baud(115200); // Set baud rate for PC serial communication
    lidarSerial1.set_baud(115200); // Set baud rate for Lidar serial communication
    lidarSerial2.set_baud(115200);

    Queue_Turret1.call_every(2ms, Turret1_Function);
    Thread_Turret1.start(callback(&Queue_Turret1, &EventQueue::dispatch_forever));

    //Queue_Turret2.call_every(2ms, Turret2_Function);
    //Thread_Turret2.start(callback(&Queue_Turret2, &EventQueue::dispatch_forever));

    Queue_Dist1.call_every(10ms, Dist_Avg1);
    Thread_Dist1.start(callback(&Queue_Dist1, &EventQueue::dispatch_forever));

    Queue_Dist2.call_every(10ms, Dist_Avg2);
    Thread_Dist2.start(callback(&Queue_Dist2, &EventQueue::dispatch_forever));
}

  

