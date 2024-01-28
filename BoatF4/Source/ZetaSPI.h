#ifndef ZETASPI_H
#define ZETASPI_H

#include "mbed.h"
#include "SPI.h"


SPI zetaspi(PA_7, PA_6, PA_5);
DigitalOut CS(PA_4); // software CS


void init_ZetaSPI(void);




#endif
