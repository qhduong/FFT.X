#include <p24Hxxxx.h>
#include <stdio.h>
#include <stdlib.h>
#include <timer.h>
#include <adc.h>
#include <delay.h>
#include <uart.h>
#include <pps.h>
#include <xc.h>
#include <libpic30.h>
#include "fft.h"


/* CONFIGURATION */
#pragma config FNOSC = FRC
#pragma config FWDTEN = OFF
#pragma config POSCMD = NONE


/*
#pragma config WDTEN = OFF
#pragma config FOSC = INTIO67
#pragma config PLLCFG = OFF
#pragma config PRICLKEN = OFF
#pragma config PBADEN = ON
#pragma config LVP = OFF
*/

// FFT
short imaginaryNumbers[64];
short realNumbers[64];
#define _XTAL_FREQ 4000000 // set frequency here

// END FFT

// RS 232
char input;
int i;
char data[5];
void sendString(char *);
void sendChar(char);
void sendCharArray(char *, unsigned int);
void sendIntArray(short *, unsigned int);

// END RS 232

int main(void) {
    //unsigned int adc_value;
    //TRISC = 0x00;
    //LATC  = 0x00;

    int k;          //for fft loop
    unsigned int adc_value;
    unsigned int config1;
    unsigned int config2;
    unsigned int config3;
    unsigned int config4;
    unsigned int configport_h;
    unsigned int configport_l;
    unsigned int configscan_h;
    unsigned int configscan_l;
    //TRISC = 0x00;
    //LATC  = 0x00;
    OSCCONbits.COSC = 0b000;
    OSCCONbits.NOSC  = 0b000;
    //OSCCONbits.CLKLOCK = 0b1;
    CLKDIVbits.FRCDIV = 0b000;

    //OSCTUNbits.TUN = 0b010111;

config1 =   ADC_MODULE_ON & ADC_IDLE_STOP &
            ADC_ADDMABM_ORDER & ADC_AD12B_12BIT &
            ADC_FORMAT_INTG & ADC_CLK_AUTO &
            ADC_AUTO_SAMPLING_ON & ADC_MULTIPLE ;
config2 =   ADC_VREF_AVDD_AVSS & ADC_SCAN_ON &
            ADC_SELECT_CHAN_0 & ADC_DMA_ADD_INC_1 ;
config3 =   ADC_CONV_CLK_SYSTEM & ADC_SAMPLE_TIME_3 &
            ADC_CONV_CLK_2Tcy;
config4 =   ADC_DMA_BUF_LOC_32;
configport_h = ENABLE_ALL_DIG_16_31;
configport_l = ENABLE_AN0_ANA;
configscan_l = SCAN_ALL_0_15;
configscan_h = SCAN_ALL_16_31;

OpenADC1(config1,config2,config3,config4,configport_l,configport_h, configscan_h,configscan_l);

    // Sets the TX and RX pins on the chip
    PPSUnLock;
    PPSOutput(OUT_FN_PPS_U1TX, OUT_PIN_PPS_RP7);        // Tx on RP7 (Pin 16)
    PPSInput(IN_FN_PPS_U1RX, IN_PIN_PPS_RP8);           // Rx on RP8 (Pin 17)
    PPSLock;

    // IMPORTANT: Set Baud Rate on Terminal to 9600
    long baud_rate = (_XTAL_FREQ / 9600 / 16) - 1;
    CloseUART1();

    OpenUART1(  UART_EN &
                UART_BRGH_FOUR &
                UART_EVEN_PAR_8BIT &
                UART_1STOPBIT,
                UART_TX_INT_DIS,
                baud_rate);
    TRISAbits.TRISA3 = 0;
    LATAbits.LATA3 = 0;


    while(1){
        short i = 0;
        LATAbits.LATA3 = 1;
        for (i = 0; i < 64; i++) {

            ConvertADC1();
            

            // Wait for the ADC conversion to complete
            while(BusyADC1());

            // Get the 10-bit ADC result and adjust the virtual ground of 2.5V
            // back to 0Vs to make the input wave centered correctly around 0
            // (i.e. -512 to +512)
            adc_value = ReadADC1(0) >> 2;
            realNumbers[i] = adc_value;
            
            // Set the imaginary number to zero
            imaginaryNumbers[i] = 0;
        }

        fix_fft(realNumbers, imaginaryNumbers, 6);


        long place, root;
        for (k=0; k < 32; k++) {
            realNumbers[k] = (realNumbers[k] * realNumbers[k] +
                             imaginaryNumbers[k] * imaginaryNumbers[k]);

            place = 0x40000000;
            root = 0;

            if (realNumbers[k] >= 0) { // Ensure we don't have a negative number

                while (place > realNumbers[k]) place = place >> 2;

                while (place) {
                    if (realNumbers[k] >= root + place) {
                        realNumbers[k] -= root + place;
                        root += place * 2;
                    }

                    root = root >> 1;
                    place = place >> 2;
                }
            }

            realNumbers[k] = root;
        }
        LATAbits.LATA3= 0;
        sendIntArray(realNumbers, 64);

        for(i = 0; i < 64; i++) {
//            LATC = realNumbers[i];
            __delay32(50);
        }
    }
    return 0;
}

// RS 232
void sendCharArray(char *array, unsigned int size) {
    for(i = 0; i < size; i++) {
        sendChar(array[i]);
    }
}

// Sends a string to terminal when ready to send
void sendString(char *string) {
    char *temp = string;
    while(*temp)
    {
        sendChar(*temp++);
    }
}

// Sends a character to terminal when ready to send
void sendChar(char character) {
    while(BusyUART1());
    putcUART1(character);
    __delay32(12);
}

void sendIntArray(short *array, unsigned int size) {
    for(i = 0; i < size; i++) {
        //sprintf(data, "%d", array[i]);
        putcUART1(array[i]);
        //sendChar(67);
    }
}