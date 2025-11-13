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
     * Constructor.
     */
    gcodeWriter();

    /**
    * Initializing the grbl receiver.
    * @result success(true) or failure(false)
    */
    static bool init();

    /**
     * Construct a message to move in the x-axis.
     * @param x the number of units to move
     * @result success(true) or failure(false)
     */
    bool writeXMove(double x);

    /**
     * Construct a message to move in the y-axis
     * @param y the number of units to move
     * @result success(true) or failure(false)
     */
    bool writeYMove(double y);

    /**
     * Construct a message to move in the z-axis
     * @param z the number of units to move
     * @result success(true) or failure(false)
     */
    bool writeZMove(double z);

    ~gcodeWriter();

    private:
    double xPos, yPos, zPos;

    static bool send(const std::string& message);
    static bool sendMove(const std::string& move_message);
    static void clearInputBuffer();

};


#endif //GCODEWRITER_GCODEWRITER_H