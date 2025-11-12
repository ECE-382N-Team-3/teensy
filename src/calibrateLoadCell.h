//
// Created by yahir on 11/4/2025.
//
#include <Arduino.h>
#include "HX711.h"

/**
 * This file is used to hold the setup and loop instructions
 * to calibrate the load cell.
 * Code is adapted from: https://github.com/sparkfun/HX711-Load-Cell-Amplifier/blob/master/firmware/SparkFun_HX711_Calibration/SparkFun_HX711_Calibration.ino
 */
#ifndef TEENSY_CALIBRATELOADCELL_H
#define TEENSY_CALIBRATELOADCELL_H
namespace calibrateLoadCell {
    inline float calibration_factor = -1500;
}

/**
 * Setup for load cell calibration.
 * @param loadCell LoadCell class instance to calibrate
 * @param dout pin number
 * @param sck pin number
 * @param calibration_factor Optional parameter to pass the calibration_factor (default: -1500)
 */
inline void calibrateLoadCellSetup(HX711 &loadCell, byte dout, byte sck, const float calibration_factor = -1500) {
    Serial.begin(9600);
    Serial.println("HX711 calibration sketch");
    Serial.println("Remove all weight from scale");
    Serial.println("After readings begin, place known weight on scale");
    Serial.println("Press + or a to increase calibration factor");
    Serial.println("Press - or z to decrease calibration factor");

    loadCell.begin(dout, sck);
    loadCell.set_scale();
    loadCell.tare(); // Reset the loadCell to 0
    calibrateLoadCell::calibration_factor = calibration_factor;

    long zero_factor = loadCell.read_average(); // Get a baseline reading
    Serial.print("Zero factor: "); // This can be used to remove the need to tare the scale. Useful in permanent scale projects.
    Serial.println(zero_factor);
}


/**
 * Loop for load cell calibration.
 * Use a or + to increase the calibration factor.
 * Use z or - to decrease the calibration factor.
 * @param loadCell LoadCell to read from
 */
inline void calibrateLoadCellLoop(HX711 &loadCell) {

    loadCell.set_scale(calibrateLoadCell::calibration_factor); // Adjust to this calibration factor

    Serial.print("Reading: ");
    Serial.print(loadCell.get_units(), 1);
    Serial.print(" lbs"); // Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
    Serial.print(" calibration_factor: ");
    Serial.print(calibrateLoadCell::calibration_factor);
    Serial.println();

    if(Serial.available())
    {
        char temp = Serial.read();
        if(temp == '+' || temp == 'a')
            calibrateLoadCell::calibration_factor += 10;
        else if(temp == '-' || temp == 'z')
            calibrateLoadCell::calibration_factor -= 10;
    }
}

#endif // TEENSY_CALIBRATELOADCELL_H