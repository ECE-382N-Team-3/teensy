//
// Created by yahir on 10/28/2025.
//

// Uncomment below to make the code be available to use in an Arduino-based hardware/software
#define ARDUINO_MODE

#include "gcodeWriter.h"
#include <iostream>
#include <ostream>
#ifdef ARDUINO_MODE
#include "Arduino.h"
#define MAIN_SERIAL Serial1 // This class will initialize and close this serial
#define DEBUG_SERIAL Serial // This is expected to be initialized else where
#endif

#define GRBL_WAKEUP "\r\n\r\n"
#define GRBL_ABSOLUTE "G90" // Sets grbl machine to work in absolute coordinates
#define GRBL_MEASUREMENT "G21" // Sets grbl machine to work in metric system
#define GRBL_WORKSPACE "G54" // Sets grbl to Workspace 1 (current position = origin)
#define GRBL_WORKSPACE_PLANES "G17" // Sets Plane to XY
#define GRBL_MOVE "G00" // Moves grbl quickly to defined point
#define DEBUG 1
#define MAX_X 100
#define MAX_Y 100
#define MAX_Z 100
#define MIN_X -100
#define MIN_Y -100
#define MIN_Z -100

/**
 * Constructor.
 * @param mode 0 for default or 1 for debug
 */
gcodeWriter::gcodeWriter() {
    this->xPos = 0;
    this->yPos = 0;
    this->zPos = 0;
#ifdef ARDUINO_MODE
    MAIN_SERIAL.begin(115200);
    MAIN_SERIAL.setTimeout(5000);
#endif
}


/**
* Initializing the grbl receiver.
* @result success(true) or failure(false)
*/
bool gcodeWriter::init() {
#ifdef ARDUINO_MODE
    clearInputBuffer();
    MAIN_SERIAL.print(GRBL_WAKEUP);
    delay(2);
    MAIN_SERIAL.flush();
#else
    std::cout << GRBL_WAKEUP << std::endl;
#endif
    std::string setUpMessages[] = {GRBL_ABSOLUTE, GRBL_MEASUREMENT};
    for (std::string message : setUpMessages) {
        if (!this->send(message)) return false;
    }
    return true;
}


/**
 * Construct a message to move in the x-axis
 * relative to current position.
 * @param x the number of units to move relative to current position
 * @result success(true) or failure(false)
 */
bool gcodeWriter::writeXMove(const double x) {
    this->xPos += x;
    if (this->xPos > MAX_X) this->xPos = MAX_X;
    else if (this->xPos < MIN_X) this->xPos = MIN_X;
    return this->send(GRBL_MOVE " X" + std::to_string(this->xPos));
}


/**
 * Construct a message to move in the y-axis
 * relative to current position.
 * @param y the number of units to move relative to current position
 * @result success(true) or failure(false)
 */
bool gcodeWriter::writeYMove(const double y) {
    this->yPos += y;
    if (this->yPos > MAX_Y) this->yPos = MAX_Y;
    else if (this->yPos < MIN_Y) this->yPos = MIN_Y;
    return this->send(GRBL_MOVE" Y" + std::to_string(this->yPos));
}


/**
 * Construct a message to move in the z-axis
 * relative to current position.
 * @param z the number of units to move relative to current position
 * @result success(true) or failure(false)
 */
bool gcodeWriter::writeZMove(const double z) {
    this->zPos += z;
    if (this->zPos > MAX_Z) this->zPos = MAX_Z;
    else if (this->zPos < MIN_Z) this->zPos = MIN_Z;
    return this->send(GRBL_MOVE" Z" + std::to_string(this->zPos));
}


gcodeWriter::~gcodeWriter() {
#ifdef ARDUINO_MODE
    MAIN_SERIAL.end();
#endif
}


/**
 * To send a gcode message.
 * @param message Message to be sent
 * @return 0 for success or 1 for failure
 */
bool gcodeWriter::send(const std::string &message) {
    bool ok= false;
#ifdef ARDUINO_MODE
    clearInputBuffer();
    MAIN_SERIAL.print((message+'\n').c_str());
    MAIN_SERIAL.flush();
    DEBUG_SERIAL.println(message.c_str());
    DEBUG_SERIAL.flush();
    String input = MAIN_SERIAL.readStringUntil('\n').trim();
    if (input.equals("ok")) {
        ok = true;
        DEBUG_SERIAL.println(input);
        DEBUG_SERIAL.flush();
    } else  {
        DEBUG_SERIAL.println(input);
        DEBUG_SERIAL.flush();
    }
#else
    std::cout << message << std::endl;
    ok = true;
#endif
    return ok;
}

/**
 * Clears the Serial input buffer of any
 * waiting values.
 */
void gcodeWriter::clearInputBuffer() {
#ifdef ARDUINO_MODE
    while (MAIN_SERIAL.available()) MAIN_SERIAL.read();
#endif
}
