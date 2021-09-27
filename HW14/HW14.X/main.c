#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include<math.h>
#define PI 3.14159

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = FRCPLL // internal oscillator
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = OFF // internal rcq
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

void delay(unsigned long int time){
     _CP0_SET_COUNT(0);
      while(_CP0_GET_COUNT() < time){}
     _CP0_SET_COUNT(0);
}

int main(void) {
  // call initializations
  __builtin_disable_interrupts(); // disable interrupts while initializing things

  // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
  __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

  // 0 data RAM access wait states
  BMXCONbits.BMXWSDRM = 0x0;

  // enable multi vector interrupts
  INTCONbits.MVEC = 0x1;

  // disable JTAG to get pins back
  DDPCONbits.JTAGEN = 0;
  
   // set the pin you want to use to OCx
  RPB6Rbits.RPB6R = 0b0110;

  T2CONbits.TCKPS = 2;     // set the timer prescaler so that you can use the largest PR2 value as possible without going over 65535 and the frequency is 50Hz
  // possible values for TCKPS are 0 for 1:1, 1 for 1:2, 2 for 1:4, 3 for 1:8, 4 for 1:16, 5 for 1:32, ...
  PR2 = 50000;              // max value for PR2 is 65535
  TMR2 = 0;                // initial TMR2 count is 0
  OC5CONbits.OCM = 0b110;  // PWM mode without fault pin; other OCxCON bits are defaults
  OC5RS = 0;             // duty cycle = OCxRS/(PR2+1)
  OC5R = 0;              // initialize before turning OCx on; afterward it is read-only
  T2CONbits.ON = 1;        // turn on Timer2
  OC5CONbits.ON = 1;       // turn on OCx
    
  __builtin_enable_interrupts();
    
 
  
  // the rest of your code
  int zeroDeg = 4000;
  int oneEightyDeg = 29000;
  unsigned long dt = 48000;//each cycle is 0.5sec / 50 = 0.01 sec
  //OC5RS = oneEightyDeg;
  // set OCxRS to get between a 0.5ms and 2.5ms pulse out of the possible 20ms (50Hz)
  float f = 0.5;
  double t = 0;
    
  while(1){
      OC5RS = (oneEightyDeg-zeroDeg)* (sin(2*PI*f*t) + 0.5);
      t += 0.01*4;
      delay(dt);
  }
}