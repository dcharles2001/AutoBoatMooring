/*
#include "mbed.h"

typedef struct {
  int distance;
  int strength;
  int temp;
  bool receiveComplete;
} TF;

TF Lidar = {0, 0, 0, false};

static BufferedSerial pc(D1, D0); // Define serial object with USBTX and USBRX

void getLidarData(TF* lidar) 
{
  static char i = 0;
  char j = 0;
  int checksum = 0;
  static int rx[9];
  static int buff[9];
  while (pc.readable()) 
  {
    pc.read(buff,sizeof(buff));
    rx[i] = buff[i];
    //printf("%d\t%d\t%d\n",rx[i],buff[0],i);
    if (rx[0] != 0x59) 
    {
      i = 0;
    } 
    else if (i == 1 && rx[1] != 0x59) 
    {
      i = 0;
    } 
    else if (i == 8) 
    {
      for (j = 0; j < 8; j++) 
      {
        checksum += rx[j];
      }
      if (rx[8] == (checksum % 256)) 
      {
        lidar->distance = rx[2] + rx[3] * 256;
        lidar->strength = rx[4] + rx[5] * 256;
        lidar->temp = (rx[6] + rx[7] * 256) / 8 - 256;
        lidar->receiveComplete = true;
      }
      i = 0;
    } 
    else 
    {
      i++;
    }
  }
}

int main() 
{
  pc.set_baud(9600); // Set baud rate
  pc.set_format(8,BufferedSerial::None,1);
  while (1) 
  {
    getLidarData(&Lidar); // Acquisition of radar data
    //printf("attempt\n");
    if (Lidar.receiveComplete) 
    {
      Lidar.receiveComplete = false;
      printf("Distance: %d cm\t", Lidar.distance);
      printf("Strength: %d\t", Lidar.strength);
      printf("Temp: %d\n", Lidar.temp);
    }
  }
}
*/

#include "mbed.h"
#include "Servo.h"
#include "stdio.h"
#include <cstdio>
//#include <string>
//#include <cmath>
//#include <memory>
#include <iostream>

Thread Thread_ToF1;
EventQueue Queue_ToF1(1 * EVENTS_EVENT_SIZE);

Thread Thread_Dist1;
EventQueue Queue_Dist1(1 * EVENTS_EVENT_SIZE);

Thread Thread_ToF2;
EventQueue Queue_ToF2(1 * EVENTS_EVENT_SIZE);

Thread Thread_Dist2;
EventQueue Queue_Dist2(1 * EVENTS_EVENT_SIZE);


float STEP = 0.002;
using namespace std;

//int Cord1X1,Cord1X2,Cord1X3
int sampleSize = 5;

I2C i2c1(PB_9, PB_8);
I2C i2c2(PB_11, PB_10);
BufferedSerial pc(USBTX, USBRX);
BufferedSerial lidarSerial1(D1, D0);
BufferedSerial lidarSerial2(PE_8, PE_7);


Servo servoX1(D9);
Servo servoY1(D6);

//Servo servoX2(D9);
//Servo servoY2(D6);

Timer tick;
Thread Thread_Turret1;
EventQueue Queue_Turret1(1 * EVENTS_EVENT_SIZE);

Thread Thread_Turret2;
EventQueue Queue_Turret2(1 * EVENTS_EVENT_SIZE);



int dist1 = 0;
int avgDist1 = 0;
int lastDist1;
int dist2 = 0;
int avgDist2 = 0;
int lastDist2 = 0;

void Write_2bytes(char d1, char d2, int IRsensorAddress) {
    char data[2] = {d1, d2};
    if (IRsensorAddress == 0xB0) {
        i2c1.write(IRsensorAddress, data, 2);
    }else{
        i2c2.write(IRsensorAddress, data, 2);
    }
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
        if(turretID == 1){
            Write_2bytes(0x30, 0x01, 0xB0); ThisThread::sleep_for(10ms);
            Write_2bytes(0x30, 0x08, 0xB0); ThisThread::sleep_for(10ms);
            Write_2bytes(0x06, 0x90, 0xB0); ThisThread::sleep_for(10ms);
            Write_2bytes(0x08, 0xC0, 0xB0); ThisThread::sleep_for(10ms);
            Write_2bytes(0x1A, 0x40, 0xB0); ThisThread::sleep_for(10ms);
            Write_2bytes(0x33, 0x33, 0xB0); ThisThread::sleep_for(10ms);
            ThisThread::sleep_for(100ms);
        }else{
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
            cout<<"Buoy 1 Not Found"<<endl;
        } else if (posX >= 1) {
            flip = -1;

        //if (turretID == 1) {
        //}else{
            //servoX2 = posX;
            //servoY2 = posY;
            //cout<<"Buoy 2 Not Found"<<endl;
        //}
    }
        servoX1 = posX;
        servoY1 = posY;
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
            }
        }
        if (abs(512 - Y) > tolerance) {          
            if (turretID == 1) {
                servoY1 = posY;
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

    void ToF_Function(){
        if(turretID == 1){
            if(avgDist1 !=0){
                lastDist1 = avgDist1;
                printf("Distance1 is: %d\n", lastDist1);
            }else{
                printf("Last Distance1 was: %d\n",lastDist1);
            }
        }else{
            if(avgDist2 !=0){
                lastDist2 = avgDist2;
                printf("Distance2 is: %d\n", lastDist2);
            }else{
                printf("Last Distance2 was: %d\n",lastDist2);
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
            if (distance1 != 0) {
                //printf("Distance: %d\n", distance);
            }
        }
    }
    memset(buf, 0, sizeof(buf));
    //ThisThread::sleep_for(1ms);
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
                //printf("Distance: %d\n", distance);
            }
        }
    }
    memset(buf, 0, sizeof(buf));
    //ThisThread::sleep_for(1ms);
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
    int* Coordinates = Turret1.IR_Sensor(0xB0);

    int X = Coordinates[0];
    int Y = Coordinates[1];
    static int fail = 0;
    static int reading = 0;

    if (X == 0 && Y == 0) {
        cout<<"Error:IR Sensor 1 Not Detected"<<endl;
    } else if (X == 1023 && Y == 1023) {
        if (fail <= 199) {
            fail++;
        } else if (fail == 200) {
            cout<<"Buoy 1 Not Found"<<endl;
            fail++;
        } else {
            Turret1.sweep();
        }
    }else{
        Turret1.Track(X,Y,5);
        if(reading == 200){
        Turret1.ToF_Function();
        reading = 0;
        }else{
            reading++;
        }
        ThisThread::sleep_for(1ms);
    }   
}


void Turret2_Function() {
    int* Coordinates = Turret2.IR_Sensor(0xB0);

    int X = Coordinates[0];
    int Y = Coordinates[1];
    static int fail = 0;
    static int reading = 0;

    if (X == 0 && Y == 0) {
        cout<<"Error: IR Sensor 2 Not Detected"<<endl;
    } else if (X == 1023 && Y == 1023) {
        if (fail <= 199) {
            fail++;
        } else if (fail == 200) {
            cout<<"Buoy 2 Not Found"<<endl;
            fail++;
        } else {
            Turret2.sweep();
        }
    }else{
        Turret2.Track(X,Y,5);
        if(reading == 200){
        Turret2.ToF_Function();
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

    Queue_Turret2.call_every(2ms, Turret2_Function);
    Thread_Turret2.start(callback(&Queue_Turret2, &EventQueue::dispatch_forever));

    Queue_Dist1.call_every(10ms, Dist_Avg1);
    Thread_Dist1.start(callback(&Queue_Dist1, &EventQueue::dispatch_forever));

    Queue_Dist2.call_every(10ms, Dist_Avg2);
    Thread_Dist2.start(callback(&Queue_Dist2, &EventQueue::dispatch_forever));
}

  
