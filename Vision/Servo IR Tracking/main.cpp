#include "mbed.h"
#include "Servo.h"
#include <cstdio>
#include <string>

I2C i2c(PB_9, PB_8);  // Set I2C pins

using namespace std;

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
int j = 0;
int sampleSize = 75;
int averageDistance = 0;
int lastDistance = 0;
int fail = 201;
int ignoreReads = 0;
int centerDist = 0;
int tolerance = 5;

float step = 0.008;
float trackSpeedX = 0.001;
float trackSpeedY = 0.001;

float posX = 0.5;
float posY = 0.5;

//Servo servoX(D9);
//Servo servoY(D6);

Timer tick;
Thread Thread_Turret1;
Thread Thread_IR1;
Thread Thread_Distance1;

Thread Thread_Turret2;

PinName convertPinName(const std::string& pinString) {
    if (pinString == "D6") {
        return D6;
    } else if (pinString == "D9") {
        return D9;
    }else if (pinString == "D0") {
        return D0;
    }else if (pinString == "D1") {
        return D1;
    }  // Add more cases for other pins as needed
    // Handle the case when pinString doesn't match any known pin names
    return NC; // NC is a special PinName value indicating "no connection"
}


class Turret {
private:
    int xPin;
    int yPin;
    int irPin;
    int tofPin1;
    int tofPin2;

    Servo servoX;
    Servo servoY;
    BufferedSerial lidarSerial;
public:
    // Constructor
    Turret(int xPin, int yPin, int irPin, int tofPin1, int tofPin2) 
        : xPin(xPin), yPin(yPin), irPin(irPin), tofPin1(tofPin1), tofPin2(tofPin2), servoX(servoX),servoY(servoY),lidarSerial(lidarSerial){
        // Attach the servos to the specified pins

        std::string combinedStringX = "D" + std::to_string(xPin);

    // Convert the combined string back to PinName
        PinName pinX = convertPinName(combinedStringX);

    // Now you can use pinX in the Servo constructor
        Servo servoX(pinX);
        Servo servoY(D6);
        // Additional setup for IR camera and ToF sensor can be added here
        }

    // Destructor
    ~Turret() {
        // Clean up or release any resources if needed
    }
    void sweep(void){
        averageDistance = 0;
        ignoreReads = 0;
        if (X == 1){
            posY = 0.80;
            posX = posX + step;
        } else{
            posX = posX - step;
            posY = 0.50;
        }

        if (posX<=0.0){
            X=1;
            printf("No Buoy Found\n");
        }else if(posX>=1){
            X=-1;
        }
        servoX = posX;
        servoY = posY;
    }

    // Method to move the turret to a specific X-Y position
    void Track(int X,int Y,int tolerance){                                          //Ix[i],Iy[i], how close can the coordinate be to central
        if (X != 1023 && Y != 1023){        
            fail = 0;                                        //Determines if IR source Located

            trackSpeedX = (((512.000000-X))/150000);
            trackSpeedY = (((512.000000-Y))/150000);

            if(trackSpeedX > 0.003){
                trackSpeedX = 0.005;
            }else if (trackSpeedX < -0.003){
                trackSpeedX = -0.005;
            }
            if(trackSpeedY > 0.003){
                trackSpeedY = 0.005;
            }else if (trackSpeedY < -0.003){
                trackSpeedY = -0.005;
            }
        posX-=trackSpeedX; 
        posY-=trackSpeedY;

        if(abs(512-X)>tolerance){
            servoX = posX; 
        }
        if(abs(512-Y)> tolerance){          
            servoY = posY;
        }
        if(abs(trackSpeedX)<0.001 && abs(trackSpeedY) < 0.001 && fail<1){
            Distance();
        }
        }else{
            if (fail==198){
                printf("Last Known Distance: %d cm\n", lastDistance);
                fail++;
            } else if (fail<200){
                fail++;
            }else{
                sweep();
            }                                                                 //If nothing found, sweep
        }
    }
    int Distance(){
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
                    lastDistance = averageDistance;
                    return lastDistance;
                    if(ignoreReads<1){
                        ignoreReads++;
                    }else{
		                printf("Distance = %d cm \n",averageDistance);
                    }
                    j = 0;
		            averageDistance = 0;
	            }
            }
        }
        memset(buf,0,sizeof(buf));
        wait_us(1000);
        return 0;
    }
    int IR_Sensor(int IRAddress){
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

        if(Ix[0] == 0 && Iy[0] == 0){
        //printf("Error: No IR Sensor Detected \n");
            return 0;
        }else{
        //printf("%d , %d\n",Iy[0],Ix[0]);
        return Ix[0];
        return Iy[0];
        }
    }
    // Other methods, if needed, can be added based on the turret's functionality
};


EventQueue Queue_Turret1(1 * EVENTS_EVENT_SIZE);
EventQueue Queue_Distance1(1 * EVENTS_EVENT_SIZE);
EventQueue Queue_IR1(1 * EVENTS_EVENT_SIZE);
EventQueue Queue_Turret2(1 * EVENTS_EVENT_SIZE);

/*
int Distance(){
    uint8_t buf[9] = {0}; // An array that holds data
        if (lidarSerial.readable()) {
            lidarSerial.read(buf, 9); // Read 9 bytes of data
            if (buf[0] == 0x59 && buf[1] == 0x59) {
                uint16_t distance = buf[2] + buf[3] * 256;
                if(distance != 0){
                    averageDistance += distance;
	                j++;
                }
            if (j == sampleSize) {
                averageDistance /= sampleSize;
                lastDistance = averageDistance;
                if (ignoreReads < 1) {
                    ignoreReads++;
                }else{
                    printf("Distance = %d cm \n", averageDistance);
                 }
            j = 0;
            averageDistance = 0;
            return lastDistance;  // Moved the return statement here
            }
        }
    }
    memset(buf,0,sizeof(buf));
    wait_us(1000);
}

void Write_2bytes(char d1, char d2)
{
    char data[2] = {d1, d2};
    i2c.write(IRsensorAddress, data, 2);
}


void sweep(void){
    averageDistance = 0;
    ignoreReads = 0;
    if (X == 1){
        posY = 0.80;
        posX = posX + step;
    } else{
        posX = posX - step;
        posY = 0.50;
    }

    if (posX<=0.0){
        X=1;
        printf("No Buoy Found\n");
    }else if(posX>=1){
        X=-1;
    }

    servoX = posX;
    servoY = posY;
}

void Track(int X,int Y,int tolerance){                                          //Ix[i],Iy[i], how close can the coordinate be to central
    if (X != 1023 && Y != 1023){        
        fail = 0;                                        //Determines if IR source Located

        trackSpeedX = (((512.000000-X))/150000);
        trackSpeedY = (((512.000000-Y))/150000);

        if(trackSpeedX > 0.003){
            trackSpeedX = 0.005;
        }else if (trackSpeedX < -0.003){
            trackSpeedX = -0.005;
        }
        if(trackSpeedY > 0.003){
            trackSpeedY = 0.005;
        }else if (trackSpeedY < -0.003){
            trackSpeedY = -0.005;
        }
        posX-=trackSpeedX; 
        posY-=trackSpeedY;

        if(abs(512-X)>tolerance){
        servoX = posX; 
        }
        if(abs(512-Y)> tolerance){          
        servoY = posY;
        }
        if(abs(trackSpeedX)<0.001 && abs(trackSpeedY) < 0.001 && fail<1){
            Distance();
        }
   }else{
       if (fail==198){
            printf("Last Known Distance: %d cm\n", lastDistance);
           fail++;
       } else if (fail<200){
           fail++;
       }else{
            sweep();
       }                                                                 //If nothing found, sweep
   }
}


void IR_Sensor1(){
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

    if(Ix[0] == 0 && Iy[0] == 0){
        printf("Error: No IR Sensor Detected \n");
        ThisThread::sleep_for(200ms);
    }else{
        //printf("%d , %d\n",Iy[0],Ix[0]);
    Track(Iy[0],Ix[0],5);
    ThisThread::sleep_for(2ms);
    }
}

void Turret1(){
    IR_Sensor1();
}
*/
void Write_2bytes(char d1, char d2)
{
    char data[2] = {d1, d2};
    i2c.write(IRsensorAddress, data, 2);
}

int main()
{
    tick.start();
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

    Turret Turret1 = Turret(9, 6, 0xB0, D1, D0); //ServoX_Pin, ServoY_Pin, IR_Address, ToF_Pin_1, ToF_Pin_2
    Queue_Turret1.call_every(2ms,Turret1);
    //Queue_Turret2.call_every(50ms,Turret2);

    Thread_Turret1.start(callback(&Queue_Turret1, &EventQueue::dispatch_forever));
    //Thread_Turret2.start(callback(&Queue_Turret1, &EventQueue::dispatch_forever));
}
 /*   while (1) {

    }
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

        if(Ix[0] == 0 && Iy[0] == 0){
            printf("Error: No IR Sensor Detected \n");
            ThisThread::sleep_for(200ms);
        }else{
        //printf("%d , %d\n",Iy[0],Ix[0]);
        Track(Iy[0],Ix[0],5);
        ThisThread::sleep_for(2ms);
        }
    }
}
*/