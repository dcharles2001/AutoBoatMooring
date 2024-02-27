#include "mbed.h"
#include "Servo.h"
#include <cstdio>
#include <string>
#include <cmath>
#include <memory>


float STEP = 0.005;
int sampleSize = 75;


I2C i2c1(PB_9, PB_8);
BufferedSerial pc(USBTX, USBRX);
BufferedSerial lidarSerial1(D1, D0);

Servo servoX1(D9);
Servo servoY1(D6);

Timer tick;
Thread Thread_Turret1;
EventQueue Queue_Turret1(1 * EVENTS_EVENT_SIZE);

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
        Write_2bytes(0x30, 0x01, 0xB0);
        ThisThread::sleep_for(10ms);
        Write_2bytes(0x30, 0x01, 0xB0);
        ThisThread::sleep_for(10ms);
        Write_2bytes(0x30, 0x01, 0xB0);
        ThisThread::sleep_for(10ms);
        Write_2bytes(0x30, 0x01, 0xB0);
        ThisThread::sleep_for(10ms);
        Write_2bytes(0x30, 0x01, 0xB0);
        ThisThread::sleep_for(10ms);
        Write_2bytes(0x30, 0x01, 0xB0);
        ThisThread::sleep_for(10ms);
        ThisThread::sleep_for(100ms);
        }
    }
    // The I2C object is automatically deleted when i2c is deleted
    ~Turret(){
    }  

    // Copy constructor (deleted)
    Turret(const Turret&) = delete;

    // Copy assignment operator (deleted)
    Turret& operator=(const Turret&) = delete;

    void sweep(void) {
        if (flip == 1) {
            posY = 0.80;
            posX = posX + STEP;
        } else {
            posX = posX - STEP;
            posY = 0.50;
        }

        if (posX <= 0.0) {
            flip = 1;
            printf("No Buoy Found\n");
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

    int Distance() {
        uint8_t buf[9] = {0}; // An array that holds data
        if (turretID == 1) {
            if (lidarSerial1.readable()) {
                for (int j = 0; j < sampleSize; j++) {
                    lidarSerial1.read(buf, 9); // Read 9 bytes of data
                    if (buf[0] == 0x59 && buf[1] == 0x59) {
                        uint16_t distance = buf[2] + buf[3] * 256;
                        if (distance != 0) {
                            averageDistance += distance;
                        } else {
                            return 0;
                        }
                        j++;
                        if (j == sampleSize) {
                            if (ignoreReads < 1) {
                                ignoreReads++;
                            } else {
                                averageDistance /= sampleSize;
                                lastDistance = averageDistance;
                            }
                            j = 0;
                            averageDistance = 0;
                        }
                    }
                }
            } else {
                lastDistance = 0;
            }
        }
        memset(buf, 0, sizeof(buf));
        wait_us(1000);
        return (averageDistance);
    }

    int combineNumbers(int num1, int num2) {
        int num2Digits = log10(num2) + 1;

        if (num2Digits != 4) {
            return -1; // Error code
        }

        int combinedNumber = num1 * pow(10, num2Digits) + num2;

        return combinedNumber;
    }

    int IR_Sensor(int IRAddress) {
        if (turretID == 1) {
            i2c1.write(IRAddress, "\x36", 1);  // Send the register address to read
            i2c1.read(IRAddress, data_buf, 16);  // Read 16 bytes
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

        int coordinates = combineNumbers(Ix[0], Iy[0]);
        return coordinates;
    }
};

extern Turret Turret1;
class Turret;
Turret Turret1(1);

void Turret1_Function() {
    int Coordinates = Turret1.IR_Sensor(0xB0);
    int X = 0;
    int Y = 0;
    static int fail;

    for (int i = 7; i >= 4; --i) {
        if (i >= 4) {
            X += Coordinates % 10;
            Coordinates /= 10;
        } else {
            Y = Coordinates;
            Coordinates /= 10;
        }
    }

    if (X == 0 && Y == 0) {
        printf("Error: No IR Sensor Detected \n");
    } else if (X == 1024 && Y == 1024) {
        if (fail <= 199) {
            fail++;
        } else if (fail > 200) {
            printf("No Buoy Found");
            Turret1.sweep();
        } else {
            printf("%d , %d \n", X, Y);
            Turret1.Track(X, Y, 5);
            int distance = Turret1.Distance();
            if (distance == 0) {
                printf("IR Unreadable \n");
            }
            printf("Distance = %d \n", distance);
        }
    }
}


//void Turret1_Function();

int main() {
    pc.set_baud(115200);
    i2c1.frequency(100000);
    tick.start();

    i2c1.frequency(115200);

    Queue_Turret1.call_every(2ms, Turret1_Function);

    Thread_Turret1.start(callback(&Queue_Turret1, &EventQueue::dispatch_forever));

    while (1) {
        ThisThread::sleep_for(1000ms);  // Add a sleep to avoid busy-waiting
    }

    // No need for manual deletion of Turret1 and i2c, they will be automatically handled by shared pointers

    return 0;
}
