#ifndef BUOY_H_
#define BUOY_H_

#include "ZetaSPI.h"



class Buoy: public zetaspi //Buoy inherits public methods of zetaspi
{


    public:

    Buoy(SPIConfig_t Pins, DigitalOut sdn, DigitalIn gpio1, DigitalIn nirq);



    private:





};





#endif
