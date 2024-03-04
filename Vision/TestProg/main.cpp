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

Thread Thread_ToF;
EventQueue Queue_ToF(1 * EVENTS_EVENT_SIZE);

Thread Thread_Dist;
EventQueue Queue_Dist(1 * EVENTS_EVENT_SIZE);


float STEP = 0.002;
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

int avgDist = 0;

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
    int dist = 0;
    int lastDist;

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

    void ToF_Function(){
    if(avgDist !=0){
        lastDist = avgDist;
        printf("Distance is: %d\n", lastDist);
    }else{
        printf("Last Distance was: %d\n",lastDist);
    }
}
};

int Distance(){
    uint8_t buf[9] = {0}; // An array that holds data
    int distance = 0;
    if (lidarSerial.readable()) {
        lidarSerial.read(buf, 9); // Read 9 bytes of data
        if (buf[0] == 0x59 && buf[1] == 0x59) {
            distance = buf[2] + buf[3] * 256;
            if (distance != 0) {
                //printf("Distance: %d\n", distance);
            }
        }
    }
    memset(buf, 0, sizeof(buf));
    return distance;
}

int dist = 0;

void Dist_Avg() {
    int missed = 0;
    avgDist = 0;

    for(int i = 0; i<sampleSize; i++){
        dist = Distance();
        if (dist != 0){
            avgDist += dist;
        }else{
           missed++;
        }
    }
    avgDist /=(sampleSize - missed);
}



extern Turret Turret1;
class Turret;
Turret Turret1(1);

void Turret1_Function() {

    int* Coordinates = Turret1.IR_Sensor(0xB0);

    int X = Coordinates[0];
    int Y = Coordinates[1];
    static int fail = 0;
    static int reading = 0;

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
        Turret1.Track(X,Y,5);
        if(reading == 200){
        Turret1.ToF_Function();
        reading = 0;
        }else{
            reading++;
        }
        ThisThread::sleep_for(1ms);
    }   //Turret1.ToF_Function();
}


int main(){
    
    pc.set_baud(115200); // Set baud rate for PC serial communication
    lidarSerial.set_baud(115200); // Set baud rate for Lidar serial communication
    tick.start();
    Queue_Turret1.call_every(2ms, Turret1_Function);
    Thread_Turret1.start(callback(&Queue_Turret1, &EventQueue::dispatch_forever));

    Queue_Dist.call_every(10ms, Dist_Avg);
    Thread_Dist.start(callback(&Queue_Dist, &EventQueue::dispatch_forever));
}
