#include "mbed.h"
#include "DRV8825.h"

DRV8825 stpr_mtr(D8,D6);

int main()
{
    while (true) {
        stpr_mtr.settings(1,0,400);
    }
}