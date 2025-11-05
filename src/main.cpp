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
gcodeWriter gantryController; // An Instance of a gcodeWriter to control the gantry

// Constants for FSR Array
constexpr double vref_mV = 3300.0/2;
constexpr double valueMax = 22072;
#define ADC_CLOCK_FREQ 2000 // in KHz [LTC2439 Data sheet allows upto 2000KHz]
#define ADC_CLOCK_PIN 2
// LUT for mapping ADCs physical channels to FSR position on array
//                                    0   1   2   3   4
const uint ADC_array_channel [25] = { 4,  26, 19, 27, 22,
                                      12, 17, 25, 18, 29,
                                      5,  14, 24, 30, 21,
                                      13, 1,  8,  0,  28,
                                      6,  10, 2,  9,  20 };

// Wiring and Constants for Load Cell
#define LOADCELL_DOUT_PIN 14
#define LOADCELL_SCK_PIN 15
#define LOADCELL_OFFSET 1
#define LOADCELl_DIVIDER 1
#define LOADCELL_THRESHOLD 300 // Used to determine when to stop pressing on object
constexpr float calibration_factor = -15000;
HX711 loadCell;

// General Constants for Pins
#define DEBUG_LED 13 // Debug LED on Teensy
#define STAMP_PIN 3 // Button Pin to Begin Stamping


/**
 * Process to flicker the Debug LED on the teensy.
 */
void flickerDebugLED() {
    digitalWrite(DEBUG_LED, LOW);
    delay(500);
    digitalWrite(DEBUG_LED, HIGH);
}

/**
 * Process to read and send FSR Data to Serial.
 */
void readAndPrintFsrArray() {
    for(int channel = 0; channel < 32 ; channel++) {
        long value;

        if(channel < 16) {
            value = adc_lo.readADC(LTC_CHANNEL_COMMON | channel);
        } else {
            value = adc_hi.readADC(LTC_CHANNEL_COMMON | (channel - 16));
        }

        double voltage_mV = (value * vref_mV / valueMax)+vref_mV;

        Serial.print("ch");
        Serial.print(channel);

        Serial.print(" : ");
        Serial.print(value);

        Serial.print(" = ");
        Serial.print(voltage_mV);
        Serial.print("mV | ");

        if(value == 32768)
            Serial.print(" OVERFLOW | ");

        //Serial.println();

    }
    Serial.println();
    flickerDebugLED();
}


/**
 * Process to stamp down on the object and read the FSR array.
 */
void stamp() {

    int moves = 0;

    // Move the gantry down until the threshold limit on loadCell is reached
    while (loadCell.get_units(10) < LOADCELL_THRESHOLD) {
        gantryController.writeZMove(0.01);
        moves++;
        delay(1000);
    }
    flickerDebugLED();

    // Get and Send FSR Readings
    readAndPrintFsrArray();

    // Move the gantry back to original position
    for (int i = 0; i < moves; i++) {
        gantryController.writeZMove(-0.01);
        delay(500);
    }
    flickerDebugLED();
}


/**
 * Intended setup for normal for operation.
 */
void mainSetup() {
    // Initialize Miscellaneous parameters
    pinMode(STAMP_PIN, INPUT);

    // Initialize FSR Array
    SPI.begin();
    analogWriteFrequency(ADC_CLOCK_PIN, ADC_CLOCK_FREQ*1000);
    analogWriteResolution(10);
    analogWrite(ADC_CLOCK_PIN, 512);

    // Initialize gantry
    if (gantryController.init() == false) {
        digitalWrite(DEBUG_LED, LOW);
        while (true) {}
    }

    // Initialize LoadCell
    loadCell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    loadCell.set_scale(calibration_factor);
    loadCell.tare();
}


/**
 * Intended loop for normal operation.
 */
void mainLoop() {
    // This assumes gantry and object are positioned correctly PRIOR to powering the system
    while (digitalRead(STAMP_PIN) == 0) {}
    stamp();
}


/**
 * Start up code for the teensy.
 * It is structured into different sections for different needs following the first lines.
 * Blocks are labeled by left most comments.
 * So, if you want to run the program as normal, ONLY uncomment mainSetup
 * and leave everything else commented.
 */
void setup() {
    Serial.begin(9600);
    pinMode(DEBUG_LED, OUTPUT);
    digitalWrite(DEBUG_LED, HIGH);

// Setup for normal operation
    // mainSetup();


// Debug for gCode Writer
// If it works, it will move the X axis forward and backward one step
    // if (gantryController.init() == false) {
    //     while (true) {}
    // }
    // gantryController.writeXMove(1);
    // gantryController.writeXMove(-1);

// Debug/ Calibration for Load Cell
    calibrateLoadCellSetup(loadCell, LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}


/**
 * Forever loop for the teensy.
 */
void loop() {

// Main Code
    // mainLoop();

// Debug Code for gCode gantryController
    // if (gantryController.init() == false) {
    //     Serial.println("Init Failed");
    // } else {
    //     Serial.println("Init OK");
    //     flickerDebugLED();
    //     delay(1000);
    // }

// Messing around with using LoadCell to move Gantry
    // long loadCellReading = loadCell.get_units(10);
    // if (loadCellReading > 300) {
    //     gantryController.writeZMove(0.01);
    //     delay(1000);
    //     flickerDebugLED();
    // }

// Debug/ Calibration loop for load cell
    calibrateLoadCellLoop(loadCell);

    // delay(1000);
}