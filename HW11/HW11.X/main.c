#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include<math.h>
#include "i2c_master_noint.h"
#include "spi.h"
#include "font.h"
#include "ST7789.h"

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = FRCPLL // use internal oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = OFF // Internal RC
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

void delay(unsigned long t);
void write_i2c(unsigned char reg, unsigned char value);
unsigned char read_i2c(unsigned char reg);
void read_multiple_i2c(unsigned char reg, unsigned char * data, int length);
void drawHorizontalBar(unsigned char x, unsigned char y, short amount, unsigned char color);
void drawVerticalBar(unsigned char x, unsigned char y, short amount, unsigned char color);

int main() {
    __builtin_disable_interrupts(); // disable interrupts while initializing things
    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);
    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;
    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;
    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
       
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 1; //start with LED on
    
    i2c_master_setup();
    initSPI();
    LCD_init();

    write_i2c(0x10,0b10000010);//Accelerometer
    write_i2c(0x11,0b10001000);//Gyroscope
    
    unsigned char data[14];
    signed short processedData[7];
    char message[15];
    
    __builtin_enable_interrupts();
    
    //WHO_AM_I test
    /*  
    signed short temp = read_i2c(0x0F);
  
     if(temp == 105){
       LATAbits.LATA4 = 0; 
       while(1){
            
        }
   }*/
    
    while (1) {
        LATAbits.LATA4 = 0;
        LATAbits.LATA4 = 1;
        
        read_multiple_i2c(0x20, &data, 14);
        int i;
        for(i = 0; i<13; i+=2){
            processedData[i/2] = data[i] | (data[i+1] << 8);
        }
        drawHorizontalBar(120,120,(1/100)*(processedData[4]), RED);
        drawVerticalBar(120,120,(1/100)*(processedData[5]), BLUE);
        LCD_clearScreen(BLACK);
        //Write Data to Screen
        /*for(i = 0; i<7; i++){
            sprintf(message,"%d",data2[i]);
            draw_string(28, i*10, WHITE, message);    
        }*/
    }
}

void delay(unsigned long t){
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT() < t){
    }
}

void write_i2c(unsigned char reg, unsigned char value){
    i2c_master_start();
    i2c_master_send(0b11010100);
    i2c_master_send(reg);
    i2c_master_send(value);
    i2c_master_stop();
}

unsigned char read_i2c(unsigned char reg){
    i2c_master_start();
    i2c_master_send(0b11010100);
    i2c_master_send(reg);
    i2c_master_restart();
    i2c_master_send(0b11010101);
    unsigned char temp = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop();
    return temp;
}

void read_multiple_i2c(unsigned char reg, unsigned char * data, int length){
    i2c_master_start();
    i2c_master_send(0b11010100);
    i2c_master_send(reg);
    i2c_master_restart();
    i2c_master_send(0b11010101);
    int i;
    for(i = 0; i < length - 1; i++){
        data[i] = i2c_master_recv();
        i2c_master_ack(0);
    }
    data[length-1] = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop();
}

void drawHorizontalBar(unsigned char x, unsigned char y, short amount, unsigned char color){
    int i;
    int j;

    if(amount > 120){
        amount = 120;
    }
    else if(amount < -120){
        amount = -120;
    }
    
    if(amount >= 0){
        for(i = 0; i < amount; i++){
            for(j = 0; j < 5; j++){
                LCD_drawPixel(x - j, y - i, color);
            }
        }
    }
    else if (amount <= 0){
        for(i = 0; i > amount; i--){
            for(j = 0; j < 5; j++){
                LCD_drawPixel(x - j, y - i, color);
            }
        }
    }
}

void drawVerticalBar(unsigned char x, unsigned char y, short amount, unsigned char color){
    int i;
    int j;

    if(amount > 120){
        amount = 120;
    }
    else if(amount < -120){
        amount = -120;
    }
    
    if(amount >= 0){
        for(i = 0; i < amount; i++){
            for(j = 0; j < 5; j++){
                LCD_drawPixel(x + i, y + j, color);
            }
        }
    }
    else if (amount <= 0){
        for(i = 0; i > amount; i--){
            for(j = 0; j < 5; j++){
                LCD_drawPixel(x + i, y + j, color);
            }
        }
    }
}