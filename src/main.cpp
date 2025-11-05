#include <Arduino.h>
#include <sys/types.h>
#include "SPI.h"
#include "ltcadc.h"
#include "HX711.h"
#include "gcodeWriter.h"
#include "calibrateLoadCell.h"
#include <PWM.h>

LTCADC adc_hi(LTC2439_1, 10);  // create an instance of the library with the LTC2439-1 as ADC and digital pin 10 as chip select
LTCADC adc_lo(LTC2439_1, 9);   // create an instance of the library with the LTC2439-1 as ADC and digital pin 9 as chip select

gcodeWriter gantryController;

constexpr double vref_mV = 3300.0/2;
constexpr double valueMax = 22072;

#define ADC_CLOCK_FREQ 2000 // in KHz [LTC2439 Data sheet allows upto 2000KHz]
#define ADC_CLOCK_PIN 2


// Wiring and Constants for Load Cell
#define LOADCELL_DOUT_PIN 14
#define LOADCELL_SCK_PIN 15
#define LOADCELL_OFFSET 1
#define LOADCELl_DIVIDER 1
float calibration_factor = -15000;
HX711 loadCell;

// Debug LED on Teensy
#define DEBUG_LED 13

// LUT for mapping ADCs physical channels to FSR position on array
//                                    0   1   2   3   4
const uint ADC_array_channel [25] = { 4,  26, 19, 27, 22,
                                      12, 17, 25, 18, 29,
                                      5,  14, 24, 30, 21,
                                      13, 1,  8,  0,  28,
                                      6,  10, 2,  9,  20};

void setup() {
    Serial.begin(115200);  // Start the serial port at 9600 baud
    // SPI.begin();
    // Serial.println("Hello");

    // analogWriteFrequency(ADC_CLOCK_PIN, ADC_CLOCK_FREQ*1000);
    // analogWriteResolution(10);
    // analogWrite(ADC_CLOCK_PIN, 512);

    pinMode(DEBUG_LED, OUTPUT);
    digitalWrite(DEBUG_LED, HIGH);

    // Debug for gCode Writer
    // If it works, it will move the X axis forward and backward one step
    if (gantryController.init() == false) {
        while (true) {}
    }
    gantryController.writeXMove(1);
    gantryController.writeXMove(-1);

    calibrateLoadCellSetup(loadCell, LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}

void loop() {
    calibrateLoadCellLoop(loadCell);

    // Read SensoryArray
    // for(int channel = 0; channel < 25 ; channel++) {
    //     long value;
    //
    //     if(ADC_array_channel[channel] < 16) {
    //         value = adc_lo.readADC(LTC_CHANNEL_COMMON | ADC_array_channel[channel]);
    //     } else {
    //         value = adc_hi.readADC(LTC_CHANNEL_COMMON | (ADC_array_channel[channel] - 16));
    //     }
    //
    //     double voltage_mV = (value * vref_mV / valueMax) + vref_mV;
    //
    //     Serial.print(voltage_mV);
    //     Serial.print(", ");
    // }

    // Debug Code for gCode gantryController
    // if (gantryController.init() == false) {
    //     Serial.println("Init Failed");
    // } else {
    //     digitalWrite(DEBUG_LED, LOW);
    //     Serial.println("Init OK");
    //     delay(1000);
    //     digitalWrite(DEBUG_LED, HIGH);
    // }

    // Messing around with using LoadCell to move Gantry
    // long loadCellReading = loadCell.get_units(10);
    // if (loadCellReading > 300) {
    //     digitalWrite(DEBUG_LED, LOW);
    //     gantryController.writeZMove(-1);
    //     delay(1000);
    //     digitalWrite(DEBUG_LED, HIGH);
    // }

    // delay(1000);
}