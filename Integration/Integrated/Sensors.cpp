#include "Sensors.h"

//Pin Setup Sensor Turrets
I2C i2c1(PB_9, PB_8); //IR1
I2C i2c2(PB_11, PB_10); //IR2

BufferedSerial pc(USBTX, USBRX);
BufferedSerial lidarSerial1(D1, D0);
BufferedSerial lidarSerial2(PE_8, PE_7);


Servo servoX1(D13);
Servo servoY1(D12);

Servo servoX2(D10);
Servo servoY2(D9);

//Sensors Turret1(1);
//Sensors Turret2(2);
//Servo servoX1(PA_5);
//Servo servoY1(PA_6);
//Servo servoX2(PD_14);
//Servo servoY2(PD_15);
int loc1 = 0;
int loc2 = 0;

void Sensors::Setup(){
    servoX1 = 0.5;
    servoY1 = 0.5;
    servoX2 = 0.5;
    servoY2 = 0.5;
    pc.set_baud(115200); // Set baud rate for PC serial communication
    lidarSerial1.set_baud(115200); // Set baud rate for Lidar serial communication
    lidarSerial2.set_baud(115200);
    Write_2bytes(0x30, 0x01,  IRAddress); ThisThread::sleep_for(10ms);
    Write_2bytes(0x30, 0x08,  IRAddress); ThisThread::sleep_for(10ms);
    Write_2bytes(0x06, 0x90,  IRAddress); ThisThread::sleep_for(10ms);
    Write_2bytes(0x08, 0xC0,  IRAddress); ThisThread::sleep_for(10ms);
    Write_2bytes(0x1A, 0x40,  IRAddress); ThisThread::sleep_for(10ms);
    Write_2bytes(0x33, 0x33,  IRAddress); ThisThread::sleep_for(10ms);
}


void Sensors::Write_2bytes(char d1, char d2, int IRAddress) {
    char data[2] = {d1, d2};
    if(turretID == 1){
        i2c1.write(0xB0, data, 2);
    }else{
        i2c2.write(0xB0, data, 2);
    }
}

void Sensors::sweep(void) {
    if (flip == 1) {
        posY = 0.55;
        if(turretID == 1){
            posX = posX - STEP; 
        }else{
            posX = posX + STEP;
        }
    }else {
        posY = 0.45;
        if(turretID == 1){
            posX = posX + STEP; 
        }else{
            posX = posX - STEP;
        }
    }

    if (turretID == 1) {
        if (posX <= 0) {
            flip = -1;
        } else if (posX >= 1) {
            flip = 1;
            printf("Turret 1 has no Buoy\n");
        }
            servoX1 = posX;
            servoY1 = posY;
    }else{
        if (posX >= 1) {
            flip = -1;
        } else if (posX <= 0) {
            flip = 1;
                printf("Turret 2 has no Buoy\n");
        }
            servoX2 = posX;
            servoY2 = posY;
    }
    Angle = posX;
}

// Method to move the turret to a specific X-Y position
void Sensors::Track(int X, int Y) {
    if(turretID == 1){
        centreX = 352;
        centreY = 532;
    }else{
        centreX = 407;
        centreY = 452;
    }
    float trackSpeedX = (((centreX - X)) / 80000);
    float trackSpeedY = (((centreY - Y)) / 80000);
    if (trackSpeedX > 0.004) {
        trackSpeedX = 0.01;
    } else if (trackSpeedX < -0.004) {
        trackSpeedX = -0.008;
    }
    if (trackSpeedY > 0.004) {
        trackSpeedY = 0.01;
    } else if (trackSpeedY < -0.004) {
        trackSpeedY = -0.01;
    }

    posX -= trackSpeedX; 
    posY += trackSpeedY;
    if (abs(centreX - X) > tolerance) {
        if (turretID == 1) {
            servoX1 = posX;
        }else{
            servoX2 = posX;
        }
    }
    if (abs(centreY - Y) > tolerance) {          
            if (turretID == 1) {
                servoY1 = posY;
            }else{
                servoY2 = posY;
            }     
    }
    
    if (abs(centreX - X) <= 20*tolerance && abs(centreY- Y) <= 20*tolerance){
            lockON = 1;
            lost = 0;
    }
    Angle = posX;
}

void Sensors::ToF_Function(int BuoyID){
    if(lastDist !=0 && lockON == 1){
        Target = BuoyID;
        Dist = lastDist;
        if(turretID == 1){
            loc1 = BuoyID;
        }else{
            loc2 = BuoyID;
        }
        printf("Buoy %d is %d cm away from Turret %d\n",BuoyID, Dist,turretID);
    }else{
        printf("Last Buoy %d Distance was %d cm away from Turret %d\n",BuoyID,lastDist,turretID);
    }
}

int Sensors::Distance(){
    uint8_t buf[9] = {0}; // An array that holds data
    int distance = 0;
    if(turretID == 1){
        if (lidarSerial1.readable()) {
            lidarSerial1.read(buf, 9); // Read 9 bytes of data
            if (buf[0] == 0x59 && buf[1] == 0x59) {
                distance = buf[2] + buf[3] * 256;
            }
        }
    }else{
        if (lidarSerial2.readable()) {
            lidarSerial2.read(buf, 9); // Read 9 bytes of data
            if (buf[0] == 0x59 && buf[1] == 0x59) {
                distance = buf[2] + buf[3] * 256;
            }
        }
    }
        memset(buf, 0, sizeof(buf));
        return distance;
}

void Sensors::Dist_Avg() {
    int missed = 0;
    for(int i = 0; i<sampleSize; i++){
        dist = Distance();
        if (dist != 0){
            avgDist += dist;
        }else{
           missed++;
        }
    }
    if(avgDist != 0 && sampleSize>missed){
        avgDist /=(sampleSize - missed);
        if(turretID == 1){
            avgDist -= 50;      //50cm offset
        }
        lastDist = avgDist;
        avgDist = 0;
    }
}


int* Sensors::combineNumbers1(int num1, int num2){
    static int Temp[2];
    Temp[1] = num1;
    Temp[0] = num2;
    //printf("%d: Temp %d         %d\n",turretID,Temp[0],Temp[1]);
    return Temp;
}

int* Sensors::combineNumbers2(int num1, int num2){
    static int Temp[2];
    Temp[1] = num1;
    Temp[0] = num2;
    //printf("%d: Temp %d         %d\n",turretID,Temp[0],Temp[1]);
    return Temp;
}

void Sensors::IR_Sensor() {
        for(int i = 0; i < 3; i++){
            flashCount[i] = 0;
        }    
        if(turretID == 1){
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
        if(turretID == 1){
            coordinates = combineNumbers1(Ix[0], Iy[0]);
        }else{
            coordinates = combineNumbers2(Ix[0],Iy[0]);
        }
        //printf("%d: X %d        Y %d\n",turretID,coordinates[0],coordinates[1]);
        if(Ix[0] == 0 && Iy[0] == 0){
            if(turretID == 1){
                printf("ERROR: IR 1 not found\n");
            }else{
                printf("ERROR: IR 2 not found\n");
            }
        }
        if(Ix[0]<1025 && Iy[0]<1025){
            if(locate == 0){// || locate1 == locate2){
                for(int i = 0; i < 3; i++){
                    flashCount[i] = 0;
                }      
                for(int measurements = 0; measurements<20; measurements++){
                    for(int i = 0; i < 3; i++){
                        if((Ix[i] == 1023 && Ix_prev[i] != 1023) || ((Ix[i] != 1023 && Ix_prev[i] == 1023))){
                        flashCount[i] = flashCount[i] + 1;
                        }
                        Ix_prev[i] = Ix[i];
                        Iy_prev[i] = Iy[i];
                        if(turretID == 1){
                            i2c1.write(IRAddress, "\x36", 1);  // Send the register address to read
                            i2c1.read(IRAddress, data_buf, 16);  // Read 16 bytes
                        }else{
                            i2c2.write(IRAddress, "\x36", 1);  // Send the register address to read
                            i2c2.read(IRAddress, data_buf, 16);  // Read 16 bytes
                        }
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
                        s  = data_buf[9];
                        Ix[2] += (s & 0x30) <<4;
                        Iy[2] += (s & 0xC0) <<2;

                        Ix[3] = data_buf[10];
                        Iy[3] = data_buf[11];
                        s   = data_buf[12];
                        Ix[3] += (s & 0x30) <<4;
                        Iy[3] += (s & 0xC0) <<2;
                        ThisThread::sleep_for(20ms);
                    }
                }
                if(flashCount[0]>2 && flashCount[0] <= 8){
                    locate = 1;
                    lost = 0;
                }else if(flashCount[0] >= 9){
                    locate = 2;
                    lost = 0;
                }else{
                    locate = 0;
                    lockON = 0;
                }
                for(int i = 0; i < 3; i++){
                    flashCount[i] = 0;
                }      
            }else{
                for(int i = 0; i < 3; i++){
                    flashCount[i] = 0;
                } 
                for(int measurements = 0; measurements<20; measurements++){
                    for(int i = 0; i < 3; i++){
                        if((Ix[i] == 1023 && Ix_prev[i] != 1023) || ((Ix[i] != 1023 && Ix_prev[i] == 1023))){
                        flashCount[i] = flashCount[i] + 1;
                        }
                        Ix_prev[i] = Ix[i];
                        Iy_prev[i] = Iy[i];

                        if(turretID == 1){
                            i2c1.write(IRAddress, "\x36", 1);  // Send the register address to read
                            i2c1.read(IRAddress, data_buf, 16);  // Read 16 bytes
                        }else{
                            i2c2.write(IRAddress, "\x36", 1);  // Send the register address to read
                            i2c2.read(IRAddress, data_buf, 16);  // Read 16 bytes
                        }
                        Ix[0] = data_buf[1];
                        Iy[0] = data_buf[2];
                        s  = data_buf[3];
                        Ix[0] += (s & 0x30) <<4;
                        Iy[0] += (s & 0xC0) <<2;
                    }
                }
                if(flashCount[0]>1 && flashCount[0] <= 8){
                    locate = 1;
                }else if(flashCount[0] >= 9){
                    locate = 2;
                }
                //printf("%d  :   %d\n",Ix[0],Iy[0]);
                if(lost>300){
                    locate = 0;
                    if(turretID == 1){
                        loc1 = 0;
                    }else{
                        loc2 = 0;
                    }

                    for(int i = 0; i < 3; i++){
                        flashCount[i] = 0;
                    }      
                }
            }
        }
}
   

void Sensors::Turret_Function() {
    static int fail = 0;
    static int reading = 0;
          if(loc1 == loc2 && loc1 != 0){
            locCount++;
            if(locCount > 20){
                posX = 0.5;
                posY = 0.6;
                flip = 1;

                Target = 0;
                Dist = 0;
                while(loc1 == loc2 && loc1 != 0){
                    sweep();
                    if(reading == 200){
                        if(turretID == 1){
                            loc1 = locate;
                        }else{
                            loc2 = locate;
                        }
                        reading = 0;
                    }else{
                        reading++;
                    }
                ThisThread::sleep_for(1ms);
                }  
            }
        }else{
            locCount = 0;
            if (locate == 0 || lost>300) {
                    lockON = 0;
                    locate = 0;  
                    if(turretID == 1){
                        loc1 = 0; 
                    }else{
                        loc2 = 0;
                    }
                    sweep();
            }else{
                int X = coordinates[0];
                int Y = coordinates[1];
                if(X>1025 || Y>1025){
                    if(turretID == 1){
                        servoY1 = 0.3;
                    }else{
                        servoY2 = 0.3;
                    } 
                    Target = 0;
                    Dist = 0;
                }
                if (X != 1023 && Y != 1023 && X < 1025 && Y < 1025){
                    lost = 0;
                    Track(X,Y);
                }else{
                    lost++;
                }
                if(lockON == 1){
                    if(lost<300){
                        if(reading == 200){
                            ToF_Function(locate);
                            reading = 0;
                        }else{
                            reading++;
                        }

                        ThisThread::sleep_for(1ms);
                    }else{
                        lockON = 0;
                        locate = 0;
                        lost = 0;
                        Target = 0;
                        Dist = 0;
                        if(turretID == 1){
                            loc1 = 0;
                        }else{
                            loc2 = 0;
                        }
                    }
                }
            }
        }
    }
