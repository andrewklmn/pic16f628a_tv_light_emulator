/*
 * File:   main.c
 * Author: user
 *
 * Created on 15 May 2018, 9:35
 */

#include <xc.h>
#include <pic16f628.h>
#include <stdlib.h>


#define _XTAL_FREQ 6000000

#define DELAY_PERIOD 0          // delay before tv blinking after sunset in seconds

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


unsigned char time = 0;
unsigned char b1 = 0;
unsigned char e1 = 0;
unsigned char b2 = 0;
unsigned char e2 = 0;
unsigned char b3 = 0;
unsigned char e3 = 0;
unsigned char b4 = 0;
unsigned char e4 = 0;
unsigned int i = 0;
char cycle_was_ended= 0;

unsigned int duration = 0;
unsigned int duration_in_seconds = 0;

unsigned int night = 43200;                 // default night duration 
unsigned int work_cycle = 17280;            // default work cycle duration 

#define MIN_NIGHT_DURATION 22000
#define MAX_NIGHT_DURATION 64000

void main(void) {
    
    
    // Setup PORTS
    TRISA = 0b00000111;  // RA0-1-2 like input   
    TRISB = 0b00000000;
    PORTB = 0b00000000;
    
    
    //запуск таймера TMR0  
    INTCON = 0b10100000;        // INT setup
    OPTION_REG = 0b00000111;    // TMR0 setup
    TMR0=0;

    
    VRCON = 0b11001111;         // Vref setup
    CMCON = 0b00000010;         // CMP setup
    //CMCON = 0b00000111;         // Disable Comparators
    
    
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
            
            // day is pending...
            VRCON = 0b11001111;         // set Vref higher for hysteresis
            
            // disable PWM
            CCPR1L = 0b00000000 ;
            CCP1CON = 0b00000000 ; 
            
            LED_GREEN = 0;
            LED_BLUE = 0;
            LED_RED = 0;
            LED_WHITE = 0;
                
            duration = 0;
            duration_in_seconds = 0;
            
            LED_INFO = 1;
            __delay_ms(20);
            LED_INFO = 0;
            __delay_ms(980);
            
            cycle_was_ended = 0;
            
        } else {
            // night is pending 
            
            VRCON = 0b11000011;         // set Vref lower for hysteresis
            
            if (duration_in_seconds < ( work_cycle + DELAY_PERIOD) 
                    && duration_in_seconds > DELAY_PERIOD) {

                //enable_PWM
                CCP1CON = 0b00111100 ; 
                // потемнело
                // генерим параметры мигания
                time = rand() % 255;                    // длина рабочего цикла
                b2 = rand() % 80;                       // green
                e2 = 80 + rand() % 165;
                b3 = rand() % 10;                       // blue
                e3 = 148 + rand() % 107;
                b4 = rand() % 138;                      // red
                e4 = 138 + rand() % 117;     
                // change PWM pulse width randomly
                CCPR1L = 120 + rand() % 135 ;            //white
                LED_INFO = 0;
                
                // blinking cycle
                for (i = 0; i < time; i++) {

                    //RB1 = (i>b1 && i<e1)?1:0;
                    LED_GREEN = (i>b2 && i<e2)?1:0;
                    LED_BLUE = (i>b3 && i<e3)?1:0;
                    LED_RED = (i>b4 && i<e4)?1:0;

                    __delay_ms(33);
                };
                LED_INFO = 1;
                
            } else {
                
                night = duration_in_seconds;            // define Current night duration
                
                if (night < MIN_NIGHT_DURATION) night = MIN_NIGHT_DURATION;
                if (night > MAX_NIGHT_DURATION) night = MAX_NIGHT_DURATION;
                work_cycle = night / 2.5;               // define new duration of work cycle
                if (duration_in_seconds > 65500 ) duration_in_seconds = 65500;
                
                LED_INFO = 1;
                __delay_ms(20);
                LED_INFO = 0;
                __delay_ms(100);
                LED_INFO = 1;
                __delay_ms(20);
                LED_INFO = 0;
                __delay_ms(860);
                
                LED_GREEN = 0;
                LED_BLUE = 0;
                LED_RED = 0;
                
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
            // day is pending...
            duration = 0;
            duration_in_seconds = 0;
            
        } else {
            // night is pending
            duration++;
            if (duration > 14) { 
                duration_in_seconds++;
                duration = 0;
            };
        };
	}
}

