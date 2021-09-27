#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include "i2c_master_noint.h"

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

const unsigned char Wadd = 0b01000000;
const unsigned char Radd = 0b01000001;
const unsigned char readReg = 19;//GPIOB
const unsigned char writeReg = 20;//OLATA

unsigned char getMC23017pin(unsigned char reg);
void setMC23017pin(unsigned char reg, unsigned char val);
void setup();

void setup(){
    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISAbits.TRISA4 = 0;//set A4 as output
    LATAbits.LATA4 = 0;//set A4 initially off
    
    i2c_master_setup();
    
    __builtin_enable_interrupts();
    
    //chip setup
    setMC23017pin(0, 0);// IODIRA = 0x00
    setMC23017pin(1, 255);// IODIRA = 0x00
}

int main(){
    setup();
    while(1){
        
        //check button press
        if(getMC23017pin(readReg) == 0)//if GPB0 is 1
            setMC23017pin(writeReg, 255);
        else
            setMC23017pin(writeReg, 0);
        //blink LED
        LATAbits.LATA4 = 1;
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 48000000 / 2 / 10){}
        LATAbits.LATA4 = 0;
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 48000000 / 2 / 10){}
    }
}

unsigned char getMC23017pin(unsigned char reg){
    i2c_master_start();//send start bit
    i2c_master_send(Wadd);//send write address
    i2c_master_send(reg);//send register
    i2c_master_restart();//send restart bit
    i2c_master_send(Radd);//send read address
    unsigned char data = i2c_master_recv(); //read data
    i2c_master_ack(1);//acknowledge
    i2c_master_stop();//send stop bit
    return data;
}


void setMC23017pin(unsigned char reg, unsigned char val){
    i2c_master_start();//send start bit
    i2c_master_send(Wadd);//send write address
    i2c_master_send(reg);//send register
    i2c_master_send(val);//send data
    i2c_master_stop();//send stop bit
}