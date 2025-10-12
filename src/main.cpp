#include <Arduino.h>
#include <sys/types.h>
#include "SPI.h"
#include "ltcadc.h"
#include <PWM.h>

LTCADC adc_hi(LTC2439_1, 10);  // create an instance of the library with the LTC2439-1 as ADC and digital pin 10 as chip select
LTCADC adc_lo(LTC2439_1, 9);   // create an instance of the library with the LTC2439-1 as ADC and digital pin 9 as chip select

constexpr double vref_mV = 3300.0/2;
constexpr double valueMax = 22072;

#define ADC_clock_freq 2000 // in KHz [LTC2439 Data sheet allows upto 2000KHz]
#define ADC_clock_pin 2

// LUT for mapping ADCs physical channels to FSR position on array
//                                    0   1   2   3   4
const uint ADC_array_channel [25] = { 4,  26, 19, 27, 22,
                                      12, 17, 25, 18, 29,
                                      5,  14, 24, 30, 21,
                                      13, 1,  8,  0,  28,
                                      6,  10, 2,  9,  20};

void setup() {
    Serial.begin(115200);  // Start the serial port at 9600 baud
    SPI.begin();
    Serial.println("Hello");

    analogWriteFrequency(ADC_clock_pin, ADC_clock_freq*1000);
    analogWriteResolution(10);
    analogWrite(ADC_clock_pin, 512);

}

void loop() {
    for(int channel = 0; channel < 25 ; channel++) {
        long value;

        if(ADC_array_channel[channel] < 16) {
            value = adc_lo.readADC(LTC_CHANNEL_COMMON | ADC_array_channel[channel]);
        } else {
            value = adc_hi.readADC(LTC_CHANNEL_COMMON | (ADC_array_channel[channel] - 16));
        }

        double voltage_mV = (value * vref_mV / valueMax) + vref_mV;

        Serial.print(voltage_mV);
        Serial.print(", ");

    }
    Serial.println();
}