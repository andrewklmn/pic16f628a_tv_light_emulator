/*
 * File:   main.c
 * Author: user
 *
 * Created on 15 May 2018, 9:35
 */

#define _XTAL_FREQ 4000000

#define WORK_PERIOD 17400         // tv blinking time after sunset in seconds

#define LED_INFO    RB1            // control pin of info LED

#define LED_WHITE   RB3            // control pin of white LED  (PWM) pin9
#define LED_GREEN   RB2            // control pins of color LEDs      pin8
#define LED_BLUE    RB4             //                                pin10
#define LED_RED     RB5             //                                pin11


#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSC oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is digital input, MCLR internally tied to VDD)
#pragma config BOREN = OFF      // Brown-out Detect Enable bit (BOD disabled)
#pragma config LVP = OFF         // Low-Voltage Programming Enable bit (RB4/PGM pin has PGM function, low-voltage programming enabled)
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Data memory code protection off)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#include <xc.h>
#include <pic16f628.h>
#include <stdlib.h>;

unsigned char time = 0;
unsigned char b1 = 0;
unsigned char e1 = 0;
unsigned char b2 = 0;
unsigned char e2 = 0;
unsigned char b3 = 0;
unsigned char e3 = 0;
unsigned char b4 = 0;
unsigned char e4 = 0;
unsigned char i;

unsigned char duration = 0;
unsigned int duration_min = 0;

void main(void) {
    
    // Setup PORTS
    TRISA = 0b00000111;         
    TRISB = 0b00000000;
    PORTB = 0b00000000;
    
    
    //запуск таймера TMR0  
    INTCON = 0b10100000;        // INT setup
    OPTION_REG = 0b00000111;    // TMR0 setup
    TMR0=0;

    
    VRCON = 0b11001111;         // Vref setup
    CMCON = 0b00000010;         // CMP setup
    
    
    __delay_ms(500);
    LED_INFO = 1;
    __delay_ms(100);
    LED_INFO = 0;
    __delay_ms(100);
    LED_INFO = 1;
    __delay_ms(100);
    LED_INFO = 0;
    __delay_ms(100);
    LED_INFO = 1;
    __delay_ms(100);
    LED_INFO = 0;
    __delay_ms(500);

    
    // PWM setup
    PR2 = 0b11111001 ;
    T2CON = 0b00000100 ;
    CCPR1L = 0b00000000 ;
    //CCP1CON = 0b00111100 ; 
    CCP1CON = 0b00000000 ; 
    
    
    srand (TMR0);
    
    while(1) {
        
        if( C1OUT ) { 
            // sunrise
            
            // disable PWM
            CCPR1L = 0b00000000 ;
            CCP1CON = 0b00000000 ; 
            
            PORTB = 0;
            duration = 0;
            
            LED_INFO = 1;
            __delay_ms(50);
            LED_INFO = 0;
            __delay_ms(950);
            
        } else {
            if (duration_min <= WORK_PERIOD) {
                
                //enable_PWM
                CCP1CON = 0b00111100 ; 
                
                // потемнело
                // генерим параметры мигания
                time = rand() % 256;
                
                //b1 = 0; e1 = rand();
                b2 = rand() % 80; 
                e2 = 80 + rand() % 176;
                
                b3 = rand() % 60; 
                e3 = 148 + rand() % 107;
                
                b4 = rand() % 100; 
                e4 = 118 + rand() % 137;     

                // change PWM pulse width randomly
                CCPR1L = 50 + rand() % 205 ;

                // мигаем некторое время
                for (i = 0; i < time; i++) {

                    //RB1 = (i>b1 && i<e1)?1:0;
                    LED_GREEN = (i>b2 && i<e2)?1:0;
                    LED_BLUE = (i>b3 && i<e3)?1:0;
                    LED_RED = (i>b4 && i<e4)?1:0;

                    __delay_ms(30);
                };
            } else {
                
                duration_min = WORK_PERIOD + 1;
                
                LED_GREEN = 0;
                LED_BLUE = 0;
                LED_RED = 0;
                LED_INFO = 1;
                
                // disable PWM
                CCPR1L = 0b00000000 ;
                CCP1CON = 0b00000000 ; 
                
                LED_WHITE = 0;
            };
        };
    };
    return;
}

void interrupt isr(void)
{
	if(T0IF)
	{
        T0IF=0; 
        
        if(C1OUT) {
            duration = 0;
            duration_min = 0;
        } else {
            duration++;
            if (duration > 14) { 
                duration_min++;
                duration = 0;
            };
        };
	}
}

