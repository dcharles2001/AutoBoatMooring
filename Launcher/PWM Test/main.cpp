// Example using PwmIn interface library, sford
//  - Note: uses InterruptIn, so not available on p19/p20

#include "mbed.h"
#include "PwmIn.h"

//PwmOut x(p21);
//PwmOut y(p22);

PwmIn a(D9);
//PwmIn b(p6);

int main() {
    //float x = 0.5;
    //float y = 0.2;
    while(1) {
        printf("a: pw = %f, period = %f\n", a.pulsewidth(), a.period());
        //printf("b: pw = %f, period = %f\n", b.pulsewidth(), b.period());
        wait_us(2000);
    }
}
