#include "mbed.h"
#include "Servo.h"
#include "stdio.h"
#include <cstdio>
#include <iostream>
#include "PwmIn.h"
#include "Launcher.h"

using namespace std;

Launcher Launcher;

//Thread Creation and Event queue for Sensor Turrets
Thread Thread_ToF1;
EventQueue Queue_ToF1(1 * EVENTS_EVENT_SIZE);

Thread Thread_Dist1;
EventQueue Queue_Dist1(1 * EVENTS_EVENT_SIZE);

Thread Thread_Cord1;
EventQueue Queue_Cord1(1 * EVENTS_EVENT_SIZE);

Thread Thread_ToF2;
EventQueue Queue_ToF2(1 * EVENTS_EVENT_SIZE);

Thread Thread_Dist2;
EventQueue Queue_Dist2(1 * EVENTS_EVENT_SIZE);

Thread Thread_Turret1;
EventQueue Queue_Turret1(1 * EVENTS_EVENT_SIZE);

Thread Thread_Turret2;
EventQueue Queue_Turret2(1 * EVENTS_EVENT_SIZE);

//Thread Creation and Event queue for Launcher
Thread Thread_Launcher;
EventQueue Queue_Launcher(1 * EVENTS_EVENT_SIZE);

//Pin Setup Sensor Turrets
I2C i2c1(PB_9, PB_8); //IR1
I2C i2c2(PB_11, PB_10); //IR2

BufferedSerial pc(USBTX, USBRX);
BufferedSerial lidarSerial1(D1, D0);
BufferedSerial lidarSerial2(PE_8, PE_7);

Servo servoX1(PA_5);
Servo servoY1(PA_6);
Servo servoX2(PD_14);
Servo servoY2(PD_15);

//Pin Setup Launcher Turret
PwmIn Cha7(D3);

//Setup for Sensor Turrets
Timer tick;
int dist1 = 0,dist2 = 0,avgDist1 = 0,avgDist2 = 0,lastDist1,lastDist2 = 0,lockON1 = 0;
int lockON2 = 0,sampleSize = 5,IRsensorAddress = 0xB0,tolerance = 5,flashHz = 20,sweep = 0;
int Ix1[4],Iy1[4],Ix_prev1[4],Iy_prev1[4],flashCount1[4] = {0,0,0,0},s1,locate1 = 0,lost1 = 0;
int Ix2[4],Iy2[4],Ix_prev2[4],Iy_prev2[4],flashCount2[4] = {0,0,0,0},s2,locate2 = 0,lost2 = 0;
char data_buf1[16],data_buf2[16];
int* coordinates1;
int* coordinates2;
float STEP = 0.0005;

//Setup for Launcher Turret
float Cha7Read = 1500;
/////TEMP REPLACE WITH SESNOR VALUES/////
int xAxisControl = 0,readyToFire = 0,safteyControl = 0;
float yAxisControl = 0.5;
/////////////////////////////////////////

//Functions for Sesnor Turrets
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
    int dist = 0;
    int lockBuoy = 0;
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
    int locatedBuoy = 0;
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
                //servoX2 = posX;
                //servoY2 = posY;
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
        }

        if (abs(512 - Y) > tolerance) {          
                if (turretID == 1) {
                    servoY1 = posY;
                }else{
                    servoY2 = posY;
                }     
        }
        
        if (abs(415 - X) <= 2*tolerance && abs(512 - Y) <= 2*tolerance){
            if(turretID == 1){
                lockON1 = 1;
            }else{
                lockON2 = 1;
            }
        }
    }

    void ToF_Function(int BuoyID){
        if(turretID == 1){
            if(avgDist1 !=0){
                lastDist1 = avgDist1;
                printf("Buoy %d is %d cm away\n",BuoyID, lastDist1);
            }else{
                printf("Last Buoy %d Distance was %d cm away\n",BuoyID,lastDist1);
            }
        }else{
            if(avgDist2 !=0){
                lastDist2 = avgDist2;
                printf("Buoy %d is %d cm away\n",BuoyID, lastDist2);
            }else{
                printf("Last Buoy %d Distance was %d cm away\n",BuoyID,lastDist2);
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

int* combineNumbers(int num1, int num2) {
    static int Temp[2];
    Temp[1] = num1;
    Temp[0] = num2;

    return Temp;
}   

void IR_Sensor1() {
    int IRAddress = 0xB0;
    i2c1.write(IRAddress, "\x36", 1);  // Send the register address to read
    i2c1.read(IRAddress, data_buf1, 16);  // Read 16 bytes

    Ix1[0] = data_buf1[1];
    Iy1[0] = data_buf1[2];
    s1     = data_buf1[3];
    Ix1[0] += (s1 & 0x30) << 4;
    Iy1[0] += (s1 & 0xC0) << 2;
    
    Ix1[1] = data_buf1[4];
    Iy1[1] = data_buf1[5];
    s1     = data_buf1[6];
    Ix1[1] += (s1 & 0x30) << 4;
    Iy1[1] += (s1 & 0xC0) << 2;

    Ix1[2] = data_buf1[7];
    Iy1[2] = data_buf1[8];
    s1     = data_buf1[9];
    Ix1[2] += (s1 & 0x30) << 4;
    Iy1[2] += (s1 & 0xC0) << 2;

    Ix1[3] = data_buf1[10];
    Iy1[3] = data_buf1[11];
    s1     = data_buf1[12];
    Ix1[3] += (s1 & 0x30) << 4;
    Iy1[3] += (s1 & 0xC0) << 2;
    int* coordinates = combineNumbers(Ix1[0], Iy1[0]);
    if(Ix1[0] == 0 && Iy1[0] == 0){
        printf("ERROR: IR 1 not found\n");
    }
    if(locate1 == 0){
        for(int measurements = 0; measurements<12; measurements++){
            for(int i = 0; i < 3; i++){
                if((Ix1[i] == 1023 && Ix_prev1[i] != 1023) || ((Ix1[i] != 1023 && Ix_prev1[i] == 1023))){
                flashCount1[i] = flashCount1[i] + 1;
            }
            Ix_prev1[i] = Ix1[i];
            Iy_prev1[i] = Iy1[i];

            i2c1.write(IRsensorAddress, "\x36", 1);  // Send the register address to read
            i2c1.read(IRsensorAddress, data_buf1, 16);  // Read 16 bytes

            Ix1[0] = data_buf1[1];
            Iy1[0] = data_buf1[2];
            s1   = data_buf1[3];
            Ix1[0] += (s1 & 0x30) <<4;
            Iy1[0] += (s1 & 0xC0) <<2;

            Ix1[1] = data_buf1[4];
            Iy1[1] = data_buf1[5];
            s1   = data_buf1[6];
            Ix1[1] += (s1 & 0x30) <<4;
            Iy1[1] += (s1 & 0xC0) <<2;

            Ix1[2] = data_buf1[7];
            Iy1[2] = data_buf1[8];
            s1   = data_buf1[9];
            Ix1[2] += (s1 & 0x30) <<4;
            Iy1[2] += (s1 & 0xC0) <<2;

            Ix1[3] = data_buf1[10];
            Iy1[3] = data_buf1[11];
            s1   = data_buf1[12];
            Ix1[3] += (s1 & 0x30) <<4;
            Iy1[3] += (s1 & 0xC0) <<2;
            ThisThread::sleep_for(30ms);
            }
        }
        if(flashCount1[0]>3 && flashCount1[0] < 8){
            locate1 = 1;
            lost1 = 0;
            //printf("Slow Buoy Found 1st     ");
        }else if(flashCount1[0] > 6 && flashCount1[0] < 12){
            locate1 = 2;
            lost1 = 0;
            //printf("Fast Buoy Found 1st     ");
        }else{
            locate1 = 0;
            if(flashCount1[1]>3 && flashCount1[1] < 8){
            //printf("Slow Buoy Found 2nd\n");
                locate1 = 1;
                lost1 = 0;
            }else if(flashCount1[1] > 6 && flashCount1[1] < 12){
            //printf("Fast Buoy Found 2nd\n");
                locate1 = 2;
                lost1 = 0;
            }else{
                //printf("Reflection Found 2nd\n");
                locate1 = 0;
                lockON1 = 0;
            }
            for(int i = 0; i < 3; i++){
                flashCount1[i] = 0;
            }      
            coordinates1 = coordinates;
        }
    }else{
        if(lost1>600){
            locate1 = 0;
            for(int i = 0; i < 3; i++){
                flashCount1[i] = 0;
            }      
        }
    }
     //printf("%d :   %d\n", locate1,lost1);
}

void IR_Sensor2() {
    int IRAddress = 0xB0;
    i2c2.write(IRAddress, "\x36", 1);  // Send the register address to read
    i2c2.read(IRAddress, data_buf2, 16);  // Read 16 bytes

    Ix2[0] = data_buf2[1];
    Iy2[0] = data_buf2[2];
    s2     = data_buf2[3];
    Ix2[0] += (s2 & 0x30) << 4;
    Iy2[0] += (s2 & 0xC0) << 2;
    
    Ix2[1] = data_buf2[4];
    Iy2[1] = data_buf2[5];
    s2     = data_buf2[6];
    Ix2[1] += (s2 & 0x30) << 4;
    Iy2[1] += (s2 & 0xC0) << 2;

    Ix2[2] = data_buf2[7];
    Iy2[2] = data_buf2[8];

        
    s2     = data_buf1[9];
    Ix2[2] += (s1 & 0x30) << 4;
    Iy2[2] += (s1 & 0xC0) << 2;

    Ix2[3] = data_buf1[10];
    Iy2[3] = data_buf1[11];
    s2     = data_buf1[12];
    Ix2[3] += (s1 & 0x30) << 4;
    Iy2[3] += (s1 & 0xC0) << 2;
    int* coordinates = combineNumbers(Ix1[0], Iy1[0]);
    if(Ix2[0] == 0 && Iy1[0] == 0){
        printf("ERROR: IR 1 not found\n");
    }
    if(locate2 == 0){
        for(int measurements = 0; measurements<12; measurements++){
            for(int i = 0; i < 3; i++){
                if((Ix2[i] == 1023 && Ix_prev1[i] != 1023) || ((Ix1[i] != 1023 && Ix_prev1[i] == 1023))){
                flashCount2[i] = flashCount1[i] + 1;
                }
                Ix_prev2[i] = Ix1[i];
                Iy_prev2[i] = Iy1[i];

                i2c2.write(IRsensorAddress, "\x36", 1);  // Send the register address to read
                i2c2.read(IRsensorAddress, data_buf1, 16);  // Read 16 bytes

                Ix2[0] = data_buf1[1];
                Iy2[0] = data_buf1[2];
                s2   = data_buf1[3];
                Ix2[0] += (s1 & 0x30) <<4;
                Iy2[0] += (s1 & 0xC0) <<2;

                Ix2[1] = data_buf1[4];
                Iy2[1] = data_buf1[5];
                s2   = data_buf1[6];
                Ix2[1] += (s1 & 0x30) <<4;
                Iy2[1] += (s1 & 0xC0) <<2;

                Ix2[2] = data_buf1[7];
                Iy2[2] = data_buf1[8];
                s2   = data_buf1[9];
                Ix2[2] += (s1 & 0x30) <<4;
                Iy2[2] += (s1 & 0xC0) <<2;

                Ix2[3] = data_buf1[10];
                Iy2[3] = data_buf1[11];
                s2   = data_buf1[12];
                Ix2[3] += (s1 & 0x30) <<4;
                Iy2[3] += (s1 & 0xC0) <<2;
                ThisThread::sleep_for(30ms);
            }
        }

        if(flashCount2[0]>3 && flashCount1[0] < 8){
            locate2 = 1;
            lost2 = 0;
            //printf("Slow Buoy Found 1st     ");
        }else if(flashCount2[0] > 6 && flashCount1[0] < 12){
            locate2 = 2;
            lost2 = 0;
            //printf("Fast Buoy Found 1st     ");
        }else{
            locate2 = 0;
            if(flashCount2[1]>3 && flashCount1[1] < 8){
                //printf("Slow Buoy Found 2nd\n");
                locate2 = 1;
                lost2 = 0;
            }else if(flashCount2[1] > 6 && flashCount1[1] < 12){
                //printf("Fast Buoy Found 2nd\n");
                locate2 = 2;
                lost2 = 0;
            }else{
                //printf("Reflection Found 2nd\n");
                locate2 = 0;
                lockON2 = 0;
            }
       
            for(int i = 0; i < 3; i++){
                flashCount2[i] = 0;
            }      
            coordinates2 = coordinates;
        }
    }else{
        if(lost2>600){
            locate2 = 0;
            for(int i = 0; i < 3; i++){
                flashCount2[i] = 0;
            }      
        }
    }
    //printf("%d :   %d\n", locate1,lost1);
}

extern Turret Turret1;
class Turret;
Turret Turret1(1);

extern Turret Turret2;
class Turret;
Turret Turret2(2);

void Turret1_Function() {
    static int fail = 0;
    static int reading = 0;
    if (locate1 == 0 || lost1>600) {
        if (fail <= 199) {
            fail++;
        } else if (fail == 200) {
            fail++;
        } else {
            fail++;
            lockON1 = 0;
            locate1 = 0;      
            //printf("HERE");
            Turret1.sweep();
            //printf("%d  :   %d  :   %d  :   %d\n", locate1, lockON1, lost1,fail);
        }
    }else{
        fail = 0;
        int X = coordinates1[0];
        int Y = coordinates1[1];
        //printf("%d  :   %d\n",X,Y);
        if (X != 1023 && Y != 1023){
            lost1 = 0;
            Turret1.Track(X,Y,tolerance);
        }else{
            lost1++;
        }
        //printf("THERE");
        if(lockON1 == 1){
            if(lost1<600){
                if(reading == 200){
                //printf("%d  :   %d      %d  :   %d\n", X,Y, locate1, lockON1);
                    Turret1.ToF_Function(locate1);
                    reading = 0;
                }else{
                    reading++;
                }
                ThisThread::sleep_for(1ms);
            }else{
                lockON1 = 0;
                locate1 = 0;
                lost1 = 0;
                //printf("EVERYWHERE");
            }
        }
    }
}

/*void Turret2_Function() {
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
        Turret2.ToF_Function();
        reading = 0;
        }else{
            reading++;
        }
        ThisThread::sleep_for(1ms);
    }   
}*/

//Functions for Launcher Turret
void LauncherMain(){
    Cha7Read = Cha7.pulsewidth();
    if((Cha7Read>1300)&&(Cha7Read<1800)){
        Launcher.stepperSControl(xAxisControl);
        Launcher.servoSControl(yAxisControl);
        Launcher.triggerSControl(readyToFire);
        Launcher.safetySControl(safteyControl);
    }else if(Cha7Read>1800){
        Launcher.stepperRCControl();
        Launcher.servoRCControl();
        Launcher.triggerRCControl();
        Launcher.safetyRCControl();
    }else{
        //printf("Stopped\n");
        Launcher.servoSControl(yAxisControl);
        Launcher.triggerSControl(readyToFire);
        Launcher.safetySControl(safteyControl);
    }
}

//MAIN
int main(){
    //Sesnor Code
    servoX1 = 0.5;
    servoY1 = 0.5;

    pc.set_baud(115200); // Set baud rate for PC serial communication
    lidarSerial1.set_baud(115200); // Set baud rate for Lidar serial communication
    lidarSerial2.set_baud(115200);

    Queue_Turret1.call_every(2ms, Turret1_Function);
    Thread_Turret1.start(callback(&Queue_Turret1, &EventQueue::dispatch_forever));

    Queue_Cord1.call_every(50ms, IR_Sensor1);
    Thread_Cord1.start(callback(&Queue_Cord1, &EventQueue::dispatch_forever));

    //Queue_Turret2.call_every(2ms, Turret2_Function);
    //Thread_Turret2.start(callback(&Queue_Turret2, &EventQueue::dispatch_forever));

    Queue_Dist1.call_every(10ms, Dist_Avg1);
    Thread_Dist1.start(callback(&Queue_Dist1, &EventQueue::dispatch_forever));

    Queue_Dist2.call_every(10ms, Dist_Avg2);
    Thread_Dist2.start(callback(&Queue_Dist2, &EventQueue::dispatch_forever));

    //Launcher Code
    Queue_Launcher.call_every(10ms, LauncherMain);
    Thread_Launcher.start(callback(&Queue_Launcher, &EventQueue::dispatch_forever));
}
