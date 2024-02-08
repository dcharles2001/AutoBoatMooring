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
}
