#include <p18cxxx.h>
#include <stdio.h>
#include <stdlib.h>
#include <timers.h>
#include <adc.h>
#include <delays.h>
#include "fft.h"

/* CONFIGURATION */
#pragma config WDTEN = OFF
#pragma config FOSC = INTIO67
#pragma config PLLCFG = OFF
#pragma config PRICLKEN = OFF
#pragma config PBADEN = ON
#pragma config LVP = OFF

// FFT
short imaginaryNumbers[64];
short realNumbers[64];
#define _XTAL_FREQ 20000000 // set frequency here

// END FFT

void main(void) {
    unsigned int adc_value;
    TRISC = 0x00;
    LATC  = 0x00;
    OSCCONbits.IRCF = 0b111;
    OSCCONbits.SCS  = 0b11;
    OSCTUNEbits.TUN = 0b01111;
    OpenADC(ADC_FOSC_RC &
            ADC_RIGHT_JUST &
            ADC_0_TAD,
            ADC_CH0 &
            ADC_INT_OFF &
            ADC_REF_VDD_VREFPLUS &
            ADC_REF_VDD_VSS,
            0b1110);

    while(1){

        short i = 0;
        for (i = 0; i < 64; i++) {

            ConvertADC();

            // Wait for the ADC conversion to complete
            while(BusyADC());

//            adc_value = ReadADC(); // Read result
//            LATC = adc_value;

            // Get the 10-bit ADC result and adjust the virtual ground of 2.5V
            // back to 0Vs to make the input wave centered correctly around 0
            // (i.e. -512 to +512)
//            realNumbers[i] = ((short)(ADRESH << 8) + (short)ADRESL) - 512;
            realNumbers[i] = ReadADC();
            // Set the imaginary number to zero
            imaginaryNumbers[i] = 0;

            __delay_us(7);

        }
        
        fix_fft(realNumbers, imaginaryNumbers, 6);


        long place, root;
        for (int k=0; k < 32; k++) {
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

        for(i = 0; i < 64; i++) {
            LATC = realNumbers[i];
            Delay10TCYx(5);
        }

//        Delay10TCYx(5);
//        ConvertADC(); // Start conversion
//        while(BusyADC()); // Wait for completion
//        adc_value = ReadADC() >> 2; // Read result, remove 2 LSBs
//        LATC = adc_value;
    }
}