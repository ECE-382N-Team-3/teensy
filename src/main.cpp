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
#define LOADCELL_THRESHOLD 60 // Used to determine when to stop pressing on object
constexpr float calibration_factor = -1500;
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
 * Press e while moving object up to EJECT, i.e. stop moving up
 * Press r while moving object up to RESET, i.e. move gantry to original position
 * Press d to move down after gantry has stop moving up either from load cell or pressing e
 *      Note: Pressing r will not wait for the user to press d, and immediately reposition the gantry
 * @return bool Determine if operation was successful (only reason would fail if rest was called for)
 */
bool stamp() {
    bool reset = false;

    int moves = 0;

    double loadCellReading = loadCell.get_units(10);

    String input;
    bool abort = false;
    // Move the gantry down until the threshold limit on loadCell is reached
    while (loadCellReading < LOADCELL_THRESHOLD && !abort && !reset) {
        gantryController.writeZMove(0.5);
        moves++;
        delay(250);
        loadCellReading = loadCell.get_units(10);
        Serial.print("Double Method: ");
        Serial.println(loadCellReading);
        input = Serial.readStringUntil('\n').trim();
        if (input.equals("e")) abort = true;
        else if (input.equals("r")) reset = true;
    }
    flickerDebugLED();

    while (!input.equals("d") && !reset) {
        input = Serial.readStringUntil('\n').trim();
    }

    // Get and Send FSR Readings
    // if (!debug) {
    //     readAndPrintFsrArray();
    // }

    // Move the gantry back to original position
    for (int i = 0; i < moves; i++) {
        gantryController.writeZMove(-0.5);
        delay(300);
    }
    flickerDebugLED();

    return !reset;
}


/**
 * Intended setup for normal for operation.
 */
void mainSetup() {
    // Initialize Miscellaneous parameters
    // pinMode(STAMP_PIN, INPUT);

    // Initialize FSR Array
    // SPI.begin();
    // analogWriteFrequency(ADC_CLOCK_PIN, ADC_CLOCK_FREQ*1000);
    // analogWriteResolution(10);
    // analogWrite(ADC_CLOCK_PIN, 512);

    // Initialize gantry
    if (gantryController.init() == false) {
        digitalWrite(DEBUG_LED, LOW);
        Serial.println ("Failed Gantry Initialization");
        while (true) {
        }
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
    const int gridDelay = 5000;

    Serial.println("Starting Main Loop");
    // This assumes gantry and object are positioned correctly PRIOR to powering the system
    if (stamp()) { // 0,0
        gantryController.writeXMove(-62.5);
        delay(gridDelay);
        loadCell.tare();
        if (stamp()) { // 1,0
            gantryController.writeYMove(62.5);
            delay(gridDelay);
            loadCell.tare();
            if (stamp()) { // 1, 1
                gantryController.writeXMove(62.5);
                delay(gridDelay);
                loadCell.tare();
                // No Reset checked need bc it will always move it back to the OG positon
                stamp(); // 0, 1
                gantryController.writeYMove(-62.5);
                delay(gridDelay);
            } else { // Reset called at 1,1 so move X and Y back
                gantryController.writeXMove(62.5);
                delay(gridDelay);
                gantryController.writeYMove(-62.5);
                delay(gridDelay);
            }
        } else { // Reset called at 1,0 so move X back
            gantryController.writeXMove(62.5);
            delay(gridDelay);
        }
    }

    loadCell.tare();
    Serial.println("Finished Main Loop: Press S to start again");
    String input;
    while (!input.equals("s")) {
        input = Serial.readStringUntil('\n').trim();
    }
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
    delay(2000);

// Setup for normal operation
    mainSetup();


// Debug for gCode Writer: If it works, it will move the X axis forward and backward one step
    // if (gantryController.init() == false) {
    //     Serial.println ("Failed Gantry Initialization");
    //     while (true) {
    //     }
    // }
    // gantryController.writeXMove(20);
    // gantryController.writeXMove(-20);


// Debug for Load Cell
    // loadCell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    // loadCell.set_scale(calibration_factor);
    // loadCell.tare();

//  Calibration for Load Cell
    // calibrateLoadCellSetup(loadCell, LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN, calibration_factor);

    delay(1000); // To allow for users to connect via Serial in time
}


/**
 * Forever loop for the teensy.
 */
void loop() {

// Main Code
    mainLoop();

// Debug Code for gCode gantryController
    // if (gantryController.init() == false) {
    //     Serial.println("Init Failed");
    // } else {
    //     Serial.println("Init OK");
    //     flickerDebugLED();
    //     delay(1000);
    // }

// LoadCell Threshold Debugging
    // double loadCellReading = loadCell.get_units(10);
    // Serial.print("LoadCell Reading: ");
    // Serial.println(loadCellReading);
    // if (loadCellReading > LOADCELL_THRESHOLD) {
    //     while (true) {
    //         Serial.println("Yippy");
    //         delay(200);
    //         flickerDebugLED();
    //     }
    // }

// LoadCell and Gantry Debugging
    // stamp(); // 0,0
    // Serial.println("Finished Stamping");
    // gantryController.writeXMove(-62.5);
    // delay(5000);
    // loadCell.tare();
    // stamp(); // 1,0
    // Serial.println("Finished Stamping");
    // gantryController.writeYMove(62.5);
    // delay(5000);
    // loadCell.tare();
    // stamp(); // 1, 1
    // Serial.println("Finished Stamping");
    // gantryController.writeXMove(62.5);
    // delay(5000);
    // loadCell.tare();
    // stamp(); // 0, 1
    // gantryController.writeYMove(-62.5);
    // Serial.println("Finished Stamping");
    // while (true) {
    //
    // }

// Calibration loop for load cell
    // calibrateLoadCellLoop(loadCell);

    // delay(1000);
}