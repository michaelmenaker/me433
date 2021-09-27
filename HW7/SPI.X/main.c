#include "SPI.h"
#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <proc/p32mx170f256b.h>
#include<stdio.h>
#include<math.h>
#define PI 3.14159
void set_voltage(unsigned char channel, unsigned short voltage);
void delay(unsigned long int time);

int main() {
    __builtin_disable_interrupts();
    initSPI();
    __builtin_enable_interrupts();
    unsigned long dt = 48000;//each cycle is 0.5sec / 50 = 0.01 sec
    double t = 0;//time
    int f = 2;//2hz sine wave
    while(1){
        set_voltage(0, 200*sin(2*PI*f*t) + 200);
        set_voltage(1, 2*200/PI*asin(sin(PI*f*t)) + 200);
        t += 0.01*4;
        delay(dt);
    }
}

void set_voltage(unsigned char channel, unsigned short voltage){
    unsigned short p = (channel<<15);
    p |= (0b111<<12);
    p |= voltage << 2;
    LATAbits.LATA0 = 0; //bring cs low
    spi_io(p>>8);
    spi_io(p);
    LATAbits.LATA0 = 1; //bring cs high
}

void delay(unsigned long int time){
     _CP0_SET_COUNT(0);
      while(_CP0_GET_COUNT() < time){}
     _CP0_SET_COUNT(0);
}