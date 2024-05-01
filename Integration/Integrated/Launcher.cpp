#include "Launcher.hpp"
#include <chrono>

PwmIn Cha1(PE_6);
PwmIn Cha2(PE_5);
PwmIn Cha5(PF_7);
//PwmIn Cha7(D3);
PwmIn Cha8(PF_8);

DigitalOut dirPin(D7);
DigitalOut stepPin(D8);

Servo Yservo(D6);
Servo Trigger(D5);
Servo Saftey(PF_9);

float map(float in, float inMin, float inMax, float outMin, float outMax) {
  // check it's within the range
  if (inMin<inMax) { 
    if (in <= inMin) 
      return outMin;
    if (in >= inMax)
      return outMax;
  } else {  // cope with input range being backwards.
    if (in >= inMin) 
      return outMin;
    if (in <= inMax)
      return outMax;
  }
  // calculate how far into the range we are
  float scale = (in-inMin)/(inMax-inMin);
  // calculate the output.
  return outMin + scale*(outMax-outMin);
}

void Launcher::stepperRCControl(){

    Cha1Read = Cha1.pulsewidth();
    //printf("%f\n", Cha1Read);
    if (Cha1Read >1600){
        stepperx= 0;
        delayTime = map(Cha1Read, 1500, 1990, 1000, 2000);
    }else if(Cha1Read<1400){
        stepperx= 1;
        delayTime = map(Cha1Read, 1000, 1500, 1000, 2000);
    }else{
        stepperx= 3;
    }
    float delayTime1 = (3000 - delayTime)/1000;
    delayTime = delayTime/1000;
    if(stepperx< 2){
        if(stepperx== 1){
            dirPin = 1;
            stepPin = 1;
            ThisThread::sleep_for(delayTime1);
            stepPin = 0;
            ThisThread::sleep_for(delayTime1);
        }else if(stepperx== 0){
            dirPin = 0;
            stepPin = 1;
            ThisThread::sleep_for(delayTime);
            stepPin = 0;
            ThisThread::sleep_for(delayTime);
        }
    }else{
        
    }
}

void Launcher::servoRCControl(){
    Cha2Read = Cha2.pulsewidth();
    servoMove = map(Cha2Read, 1000, 1990, 0.05, 0.4);
    float difference = servoMove - Yservo;
    Yservo = Yservo + (difference/80);
    servoLocation();
    ThisThread::sleep_for(2ms);
}

float Launcher::servoLocation(){
    float location = servoMove;
    return location;
}

void Launcher::triggerRCControl(){
    Cha8Read = Cha8.pulsewidth();
    if(Cha8Read>1800){
        Trigger = 0;
    }else if(Cha8Read<1100){
        Trigger = 1;
    }
}

void Launcher::safetyRCControl(){
    Cha5Read = Cha5.pulsewidth();
    if(Cha5Read>1800){
        Saftey = 1;
    }else if(Cha5Read<1100){
        Saftey = 0;
    }
}

//Sensor Control
void Launcher::stepperSControl(int XPos){
    if (XPos == 1){
        stepperx= 1;
    }else if(XPos == 0){
        stepperx= 0;
    }else{
        stepperx= 3;
    }
    float delayTime1 = 2;
    delayTime = 2;
    if(stepperx< 2){
        if(stepperx== 1){
            dirPin = 1;
            stepPin = 1;
            ThisThread::sleep_for(delayTime1);
            stepPin = 0;
            ThisThread::sleep_for(delayTime1);
        }else if(stepperx== 0){
            dirPin = 0;
            stepPin = 1;
            ThisThread::sleep_for(delayTime);
            stepPin = 0;
            ThisThread::sleep_for(delayTime);
        }
    }else{

    }
}

void Launcher::servoSControl(float YPos){
    Yservo = YPos;
    //ThisThread::sleep_for(2ms);
}

void Launcher::triggerSControl(int fire){
    if(fire == 1){
        Trigger = 1;
    }else if(fire == 0){
        Trigger = 0;
    }
}

void Launcher::safetySControl(int safe){
    if(safe == 1){
        Saftey = 1;
    }else if(safe == 0){
        Saftey = 0;
    }
}


void Launcher::commsCheck(Buoycmd_t newcmd)
{
    bool packetresp = 1;

    //Buoycmd_t newcmd = {ON, 255}; //turn on for 50 seconds
    unsigned char TestMessage[RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH]; //new message
    Boat.InstructionConfigurator(newcmd, TestMessage, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH);
    
    while(packetresp)
    {
        
        unsigned char buoyresponse[RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH];
        
        Boat.MessageWaitResponse(TestMessage);
        //still in receive mode
        ThisThread::sleep_for(100ms); //essential delay
        for(int i=0; i<2; i++)
        {
            if(Boat.ReceiveAndRead(buoyresponse, RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH))
            {
                PrintQueue.call(printf,"No response\n\r");
                packetresp = 1;
                break;
                
            }else
            {
                for(int j=0; j<RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH; j++)
                {
                    PrintQueue.call(printf,"RX: %c\n\r", buoyresponse[j]);
                }
                if(!Boat.InterpretResponse(buoyresponse)) //good packet check
                {
                    PrintQueue.call(printf,"Resp success\n\r");
                    packetresp = 0;
                }
            }
            ThisThread::sleep_for(2s); //wait for second buoy delay
        }
    }

    Boat.ChangeState(SI4455_CMD_CHANGE_STATE_ARG_NEW_STATE_ENUM_SLEEP); //go back to sleep
    BuoysTimer.reset();
    BuoysTimer.start(); //start tracking buoys on time
}

std::chrono::seconds Launcher::getbuoysTime(void) //get elapsed time in chrono::seconds
{
   return std::chrono::duration_cast<std::chrono::seconds>(BuoysTimer.elapsed_time()); //yuck
}

bool Launcher::checkbuoysTime(void)
{
    if(getbuoysTime() >= newtime) //has the established LED on time frame passed?
    {
        BuoysTimer.stop();
        //BuoysTimer.reset();
        return true; //yes, yes it has
    }else {
        return false; //nothing to see here
    }
}


