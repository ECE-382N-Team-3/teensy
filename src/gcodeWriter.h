//
// Created by yahir on 10/28/2025.
//

#ifndef GCODEWRITER_GCODEWRITER_H
#define GCODEWRITER_GCODEWRITER_H
#include <string>

/**
 * Class to construct and write gcode messages
 */
class gcodeWriter {
    public:
    /**
     * Constructor
     */
    gcodeWriter();

    /**
    * Initializing the grbl receiver.
    * @result success(true) or failure(false)
    */
    bool init();

    /**
     * Construct a message to move in the x-axis
     * relative to current position
     * @param x the number of units to move relative to current position
     * @result success(true) or failure(false)
     */
    bool writeXMove(double x);

    /**
     * Construct a message to move in the y-axis
     * relative to current position
     * @param y the number of units to move relative to current position
     * @result success(true) or failure(false)
     */
    bool writeYMove(double y);

    /**
     * Construct a message to move in the z-axis
     * relative to current position
     * @param z the number of units to move relative to current position
     * @result success(true) or failure(false)
     */
    bool writeZMove(double z);

    ~gcodeWriter();

    private:
    int xPos, yPos, zPos;

    static bool send(const std::string& message);
    static void clearInputBuffer();

};


#endif //GCODEWRITER_GCODEWRITER_H