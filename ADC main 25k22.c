#include <p18cxxx.h>
#include <stdio.h>
#include <stdlib.h>
#include <timers.h>
#include <adc.h>
#include <delays.h>
//include "fft.h"
//include "classifier.h"

/* CONFIGURATION */
#pragma config WDTEN = OFF
#pragma config FOSC = INTIO67
#pragma config PLLCFG = OFF
#pragma config PRICLKEN = OFF
#pragma config PBADEN = ON
#pragma config LVP = OFF

void main(void) {
    unsigned int adc_value;
    TRISC = 0x00;
    LATC = 0x00;
    OSCCONbits.IRCF = 0b111;
    OSCCONbits.SCS = 0b11;
    OSCTUNEbits.TUN = 0b01111;
    OpenADC(ADC_FOSC_RC &
            ADC_RIGHT_JUST &
            ADC_0_TAD,
            ADC_CH0 &
            ADC_INT_OFF &
            ADC_REF_VDD_VREFPLUS &
            ADC_REF_VDD_VSS,
            0b1110);
//    OSCCONbits.IRCF2 = 1;
//    OSCCONbits.IRCF1 = 1;
//    OSCCONbits.IRCF0 = 1;


    while(1){
        Delay10TCYx(5);
        ConvertADC(); // Start conversion
        while(BusyADC()); // Wait for completion
        adc_value = ReadADC() >> 2; // Read result, remove 2 LSBs
        LATC = adc_value;
    }
}