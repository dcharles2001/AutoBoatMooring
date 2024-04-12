#include "Sensors.h"

//Pin Setup Sensor Turrets
I2C i2c1(PB_9, PB_8); //IR1
I2C i2c2(PB_11, PB_10); //IR2

BufferedSerial pc(USBTX, USBRX);
BufferedSerial lidarSerial1(D1, D0);
BufferedSerial lidarSerial2(PE_8, PE_7);


Servo servoX1(D5);
Servo servoY1(D6);

Servo servoX2(D9);
Servo servoY2(D10);

int IRAddress1 = 0xB0;
int IRAddress2 = 0xB0;

int tolerance = 5;
int locate1 = 0;
int locate2 = 0;
//Servo servoX1(PA_5);
//Servo servoY1(PA_6);
//Servo servoX2(PD_14);
//Servo servoY2(PD_15);

void Sensors::Setup(){
    servoX1 = 0.5;
    servoY1 = 0.5;
    pc.set_baud(115200); // Set baud rate for PC serial communication
    lidarSerial1.set_baud(115200); // Set baud rate for Lidar serial communication
    lidarSerial2.set_baud(115200);
    Write_2bytes(0x30, 0x01,  IRAddress1); ThisThread::sleep_for(10ms);
    Write_2bytes(0x30, 0x08,  IRAddress1); ThisThread::sleep_for(10ms);
    Write_2bytes(0x06, 0x90,  IRAddress1); ThisThread::sleep_for(10ms);
    Write_2bytes(0x08, 0xC0,  IRAddress1); ThisThread::sleep_for(10ms);
    Write_2bytes(0x1A, 0x40,  IRAddress1); ThisThread::sleep_for(10ms);
    Write_2bytes(0x33, 0x33,  IRAddress1); ThisThread::sleep_for(10ms);
}
int SWAP = 0;

void Sensors::Write_2bytes(char d1, char d2, int IRsensorAddress) {
    char data[2] = {d1, d2};
    i2c1.write(IRAddress1, data, 2);
    i2c2.write(IRAddress2, data, 2);
}

void Sensors::sweep(void) {
    if (flip == 1) {
        posY = 0.6;
        posX = posX + STEP;
    } else {
        posX = posX - STEP;
        posY = 0.4;
    }

    if (turretID == 1) {
        if (posX <= 0.2) {
            flip = 1;
        } else if (posX >= 0.8) {
            flip = -1;
            if(locate1 == locate2 && locate1 != 0){
                SWAP = 2;
            }else{
                printf("Turret 1 has no Buoy\n");
            }
    
        }
            servoX1 = posX;
            servoY1 = posY;
    }else{
        if (posX <= 0.2) {
            flip = 1;
        } else if (posX >= 0.8) {
            flip = -1;
            if(locate1 == locate2 && locate2 != 0){
                SWAP = 1;
            }else{
                printf("Turret 2 has no Buoy\n");
            }
        }
            servoX2 = posX;
            servoY2 = posY;
    }
}

// Method to move the turret to a specific X-Y position
void Sensors::Track(int X, int Y, int tolerance) {
    float trackSpeedX = (((415.000000 - X)) / 90000);
    float trackSpeedY = (((512.000000 - Y)) / 90000);

    if (trackSpeedX > 0.004) {
        trackSpeedX = 0.008;
    } else if (trackSpeedX < -0.004) {
        trackSpeedX = -0.008;
    }
    if (trackSpeedY > 0.004) {
        trackSpeedY = 0.008;
    } else if (trackSpeedY < -0.004) {
        trackSpeedY = -0.008;
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

void Sensors::ToF_Function(int BuoyID){
    if(turretID == 1){
        printf("Turret1 locked on to Buoy %d\n", BuoyID);
        if(avgDist1 !=0 && lockON1 == 1){
            lastDist1 = avgDist1 - 30; //30cm offset
            //printf("Buoy %d is %d cm away from Turret 1\n",BuoyID, lastDist1);
        }else{
            //printf("Last Buoy %d Distance was %d cm away from Turret 1\n",BuoyID,lastDist1);
        }
    }else{
        printf("Turret2 locked on to Buoy %d\n", BuoyID);
        if(avgDist2 !=0 && lockON2 == 1){

            lastDist2 = avgDist2;
            //printf("Buoy %d is %d cm away from Turret 2\n",BuoyID, lastDist2);
        }else{
            //printf("Last Buoy %d Distance was %d cm away from Turret 2\n",BuoyID,lastDist2);
        }

    }
}

int Sensors::Distance1(){
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

int Sensors::Distance2(){
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

void Sensors::Dist_Avg1() {
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

void Sensors::Dist_Avg2() {
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

int* combineNumbers1(int num1, int num2) {
    static int Temp1[2];
    Temp1[1] = num1;
    Temp1[0] = num2;

    return Temp1;
}

int* combineNumbers2(int num1, int num2) {
    static int Temp2[2];
    Temp2[1] = num1;
    Temp2[0] = num2;

    return Temp2;
}

void Sensors::IR_Sensor1() {
    int IRAddress = IRAddress1;
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
    int* coordinates = combineNumbers1(Ix1[0], Iy1[0]);
    if(Ix1[0] == 0 && Iy1[0] == 0){
        printf("ERROR: IR 1 not found\n");
    }
    if(locate1 == 0){// || locate1 == locate2){
        for(int i = 0; i < 3; i++){
            flashCount1[i] = 0;
        }      
        for(int measurements = 0; measurements<13; measurements++){
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
        //printf("flash 1 %d\n",flashCount1[0]);
        if(flashCount1[0]>1 && flashCount1[0] <= 4){
            locate1 = 1;
            lost1 = 0;
            //printf("Slow Buoy Found 1st     ");
        }else if(flashCount1[0] >= 5 && flashCount1[0] < 12){
            locate1 = 2;
            lost1 = 0;
            //printf("Fast Buoy Found 1st     ");
        }else{
            locate1 = 0;
            if(flashCount1[1]>1 && flashCount1[1] <= 3){
            //printf("Slow Buoy Found 2nd\n");
                locate1 = 1;
                lost1 = 0;
            }else if(flashCount1[1] >= 4 && flashCount1[1] < 12){
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
}

void Sensors::IR_Sensor2() {
    int IRAddress = IRAddress2;
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

        
    s2     = data_buf2[9];
    Ix2[2] += (s2 & 0x30) << 4;
    Iy2[2] += (s2 & 0xC0) << 2;

    Ix2[3] = data_buf2[10];
    Iy2[3] = data_buf2[11];
    s2     = data_buf2[12];
    Ix2[3] += (s2 & 0x30) << 4;
    Iy2[3] += (s2 & 0xC0) << 2;
    int* coordinates = combineNumbers2(Ix2[0], Iy2[0]);
    if(Ix2[0] == 0 && Iy2[0] == 0){
        printf("ERROR: IR 2 not found\n");
    }
    //printf("%d  :   %d\n", Ix2[0],Iy2[0]);
    if(locate2 == 0){// || locate1 == locate2){
            for(int i = 0; i < 3; i++){
                flashCount2[i] = 0;
            } 
        for(int measurements = 0; measurements<13; measurements++){  
            for(int i = 0; i < 3; i++){
                if((Ix2[i] == 1023 && Ix_prev2[i] != 1023) || ((Ix2[i] != 1023 && Ix_prev2[i] == 1023))){
                flashCount2[i] = flashCount2[i] + 1;
                }
                Ix_prev2[i] = Ix2[i];
                Iy_prev2[i] = Iy2[i];

                i2c2.write(IRsensorAddress, "\x36", 1);  // Send the register address to read
                i2c2.read(IRsensorAddress, data_buf2, 16);  // Read 16 bytes

                Ix2[0] = data_buf2[1];
                Iy2[0] = data_buf2[2];
                s2   = data_buf2[3];
                Ix2[0] += (s2 & 0x30) <<4;
                Iy2[0] += (s2 & 0xC0) <<2;

                Ix2[1] = data_buf2[4];
                Iy2[1] = data_buf2[5];
                s2   = data_buf2[6];
                Ix2[1] += (s2 & 0x30) <<4;
                Iy2[1] += (s2 & 0xC0) <<2;

                Ix2[2] = data_buf2[7];
                Iy2[2] = data_buf2[8];
                s2   = data_buf2[9];
                Ix2[2] += (s2 & 0x30) <<4;
                Iy2[2] += (s2 & 0xC0) <<2;

                Ix2[3] = data_buf2[10];
                Iy2[3] = data_buf2[11];
                s2   = data_buf2[12];
                Ix2[3] += (s2 & 0x30) <<4;
                Iy2[3] += (s2 & 0xC0) <<2;
                ThisThread::sleep_for(30ms);
            }
        }
        //printf("flash 2 %d\n",flashCount2[0]);
        if(flashCount2[0]>1 && flashCount2[0] <= 4){
            locate2 = 1;
            lost2 = 0;
            //printf("Slow Buoy Found 1st     ");
        }else if(flashCount2[0] >=5 && flashCount2[0] < 12){
            locate2 = 2;
            lost2 = 0;
            //printf("Fast Buoy Found 1st     ");
        }else{
            locate2 = 0;
            if(flashCount2[1]>1 && flashCount2[1] <= 4){
                //printf("Slow Buoy Found 2nd\n");
                locate2 = 1;
                lost2 = 0;
            }else if(flashCount2[1] >= 5 && flashCount2[1] < 12){
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
}
/*
extern Sensors Turret1;
Sensors Turret1(1);

extern Sensors Turret2;
Sensors Turret2(2);
*/
void Sensors::Turret_Function1() {
    static int fail = 0;
    static int reading = 0;
    if(locate1 == locate2 && locate1 != 0){
        if(SWAP == 1){
            locate1 = 0;
            sweep();
        }
    }else{
        SWAP = 0;
        if (locate1 == 0 || lost1>600) {
            if (fail <= 199) {
                fail++;
            } else if (fail == 200) {
                fail++;
            } else {
                fail++;
                lockON1 = 0;
                locate1 = 0;   
                sweep();
            }
        }else{
            fail = 0;
            int X1 = coordinates1[0];
            int Y1 = coordinates1[1];
            if (X1 != 1023 && Y1 != 1023){
                lost1 = 0;
                Track(X1,Y1,tolerance);
            }else{
                lost1++;
                //printf("%d\n", lost1);
            }
            if(lockON1 == 1){
                if(lost1<600){
                    if(reading == 200){
                        ToF_Function(locate1);
                        reading = 0;
                    }else{
                        reading++;
                    }
                    ThisThread::sleep_for(1ms);
                }else{
                    lockON1 = 0;
                    locate1 = 0;
                    lost1 = 0;
                }
            }
        }
    }
}

void Sensors::Turret_Function2() {
    static int fail = 0;
    static int reading = 0;
    if(locate1 == locate2 && locate2 != 0){
        if(SWAP == 2){
            locate2 = 0;
            sweep();
        }
    }else{
        SWAP = 0;
        if (locate2 == 0 || lost2>600) {
            if (fail <= 199) {
                fail++;
            } else if (fail == 200) {
                fail++;
            } else {
                fail++;
                lockON2 = 0;
                locate2 = 0;   
                sweep();
            }
        }else{
            fail = 0;
            int X2 = coordinates2[0];
            int Y2 = coordinates2[1];
            if (X2 != 1023 && Y2 != 1023){
                lost2 = 0;
                Track(X2,Y2,tolerance);
            }else{
                lost2++;
                //printf("%d\n", lost1);
            }
            if(lockON2 == 1){
                if(lost2<600){
                    if(reading == 200){
                        ToF_Function(locate2);
                        reading = 0;
                    }else{
                        reading++;
                    }
                    ThisThread::sleep_for(1ms);
                }else{
                    lockON2 = 0;
                    locate2 = 0;
                    lost2 = 0;
                }
            }
        }
    }
}