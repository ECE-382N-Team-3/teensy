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
#define GRBL_RELATIVE "G91" // Sets grbl machine to work in relative coordinates
#define GRBL_MEASUREMENT "G21" // Sets grbl machine to work in metric system
#define GRBL_WORKSPACE "G54" // Sets grbl to Workspace 1 (current position = origin)
#define GRBL_WORKSPACE_PLANES "G17" // Sets Plane to XY
// #define GRBL_MOVE "G00" // Moves quickly to defined point
#define GRBL_MOVE "G01" // Moves quickly to defined point with defined feed-rate
#define GRBL_SPEED "F100" // Rate at which motors move
#define MAX_X 100
#define MAX_Y 100
#define MAX_Z 100
#define MIN_X (-100)
#define MIN_Y (-100)
#define MIN_Z (-100)

/**
 * Constructor.
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
    std::string setUpMessages[] = {GRBL_RELATIVE, GRBL_MEASUREMENT};
    for (const std::string& message : setUpMessages) {
        if (!send(message)) return false;
    }
    return true;
}


/**
 * Construct a message to move in the x-axis.
 * @param x the number of units to move
 * @result success(true) or failure(false)
 */
bool gcodeWriter::writeXMove(const double x) {
    this->xPos += x;
    if (this->xPos > MAX_X)
        this->xPos = MAX_X;
    else if (this->xPos < MIN_X)
        this->xPos = MIN_X;
    else return sendMove("X" + std::to_string(x));

    DEBUG_SERIAL.println("Message not sent: Requested X move reached MAX or MIN value");
    return false;
}


/**
 * Construct a message to move in the y-axis
 * @param y the number of units to move
 * @result success(true) or failure(false)
 */
bool gcodeWriter::writeYMove(const double y) {
    this->yPos += y;
    if (this->yPos > MAX_Y)
        this->yPos = MAX_Y;
    else if (this->yPos < MIN_Y)
        this->yPos = MIN_Y;
    else
        return sendMove("Y" + std::to_string(y));

    DEBUG_SERIAL.println("Message not sent: Requested Y Move reached MAX or MIN value");
    return false;
}


/**
 * Construct a message to move in the z-axis
 * @param z the number of units to move
 * @result success(true) or failure(false)
 */
bool gcodeWriter::writeZMove(const double z) {
    this->zPos += z;
    if (this->zPos > MAX_Z)
        this->zPos = MAX_Z;
    else if (this->zPos < MIN_Z)
        this->zPos = MIN_Z;
    else
        return sendMove("Z"+ std::to_string(z));

    DEBUG_SERIAL.println("Message not sent: Reqeust Z Move reached MAX or MIN Value");
    return false;
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
    DEBUG_SERIAL.flush();
    String input = MAIN_SERIAL.readStringUntil('\n').trim();
    DEBUG_SERIAL.print("Sent Message: ");
    DEBUG_SERIAL.println(message.c_str());
    if (input.equals("ok")) {
        ok = true;
        DEBUG_SERIAL.print("Success:");
        DEBUG_SERIAL.println(input);
        DEBUG_SERIAL.flush();
    } else  if (input == nullptr) {
        DEBUG_SERIAL.println("Timeout Error");
        DEBUG_SERIAL.flush();
    } else {
        DEBUG_SERIAL.print("ERROR: ");
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
 * Small wrapper to send a move message.
 * @param move_message ex. "X0.0"
 * @return success(true) or failure(false)
 */
bool gcodeWriter::sendMove(const std::string& move_message) {
    return send(
        GRBL_MOVE
        " " + move_message +
        " " + GRBL_SPEED
        );
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
