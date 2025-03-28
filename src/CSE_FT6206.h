
//============================================================================================//
/*
  Filename: CSE_FT6206.h
  Description: Main header file for the CSE_FT6206 Arduino library.
  Framework: Arduino, PlatformIO
  Author: Vishnu Mohanan (@vishnumaiea, @vizmohanan)
  Maintainer: CIRCUITSTATE Electronics (@circuitstate)
  Version: 0.0.3
  License: MIT
  Source: https://github.com/CIRCUITSTATE/CSE_FT6206
  Last Modified: +05:30 19:33:20 PM 28-03-2025, Friday
 */
//============================================================================================//

#ifndef CSE_FT6206_H // CSE_FT6206_LIBRARY
#define CSE_FT6206_H

#include "Arduino.h"
#include <Wire.h>
#include <CSE_Touch.h>

#define DEBUG_SERIAL Serial  // Select the serial port to use for debug output

// #define CSE_FT6206_DEBUG  // Uncomment this line to enable debug output
// #define CSE_FT6206_I2C_DEBUG  // Uncomment this line to enable I2C debug output

#define FT6206_MAX_TOUCH_POINTS           2

#define FT62XX_I2C_ADDR                   0x38  // I2C address of FT6206 controller
#define FT62XX_NUM_X                      0x33  // Touch X position
#define FT62XX_NUM_Y                      0x34  // Touch Y position

#define FT62XX_REG_DEV_MODE               0x00  // Device mode, either WORKING or FACTORY
#define FT62XX_REG_GEST_ID                0x01  // Gesture ID
#define FT62XX_REG_TD_STATUS              0x02  // Number of touch points

#define FT62XX_REG_P1_XH                  0x03  // Point 1 touch X position low byte
#define FT62XX_REG_P1_XL                  0x04  // Point 1 touch X position high byte
#define FT62XX_REG_P1_YH                  0x05  // Point 1 touch Y position low byte
#define FT62XX_REG_P1_YL                  0x06  // Point 1 touch Y position high byte
#define FT62XX_REG_P1_WEIGHT              0x07  // Point 1 touch weight
#define FT62XX_REG_P1_MISC                0x08  // Point 1 touch area

#define FT62XX_REG_P2_XH                  0x09  // Point 2 touch X position low byte
#define FT62XX_REG_P2_XL                  0x0A  // Point 2 touch X position high byte
#define FT62XX_REG_P2_YH                  0x0B  // Point 2 touch Y position low byte
#define FT62XX_REG_P2_YL                  0x0C  // Point 2 touch Y position high byte
#define FT62XX_REG_P2_WEIGHT              0x0D  // Point 2 touch weight
#define FT62XX_REG_P2_MISC                0x0E  // Point 2 touch area

#define FT62XX_REG_TH_GROUP               0x80  // Threshold for touch detection
#define FT62XX_REG_TH_DIFF                0x85  // Filter function coefficient
#define FT62XX_REG_CTRL                   0x86  // Automatic mode swtiching control
#define FT62XX_REG_TIMEENTERMONITOR       0x87  // Time between entering "Monitor" mode
#define FT62XX_REG_PERIODACTIVE           0x88  // Report rate in "Active" mode
#define FT62XX_REG_PERIODMONITOR          0x89  // Report rate in "Monitor" mode
#define FT62XX_REG_RADIAN_VALUE           0x91  // The value of the minimum allowed angle while Rotating gesture mode
#define FT62XX_REG_OFFSET_LEFT_RIGHT      0x92  // Maximum offset while Moving Left and Moving Right gesture
#define FT62XX_REG_OFFSET_UP_DOWN         0x93  // Maximum offset while Moving Up and Moving Down gesture
#define FT62XX_REG_DISTANCE_LEFT_RIGHT    0x94  // Minimum distance while Moving Left and Moving Right gesture
#define FT62XX_REG_DISTANCE_UP_DOWN       0x95  // Minimum distance while Moving Up and Moving Down gesture
#define FT62XX_REG_DISTANCE_ZOOM          0x96  // Minimum distance while Zoom In and Zoom Out gesture
#define FT62XX_REG_LIB_VERSION_H          0xA1  // High 8-bit of LIB Version info
#define FT62XX_REG_LIB_VERSION_L          0xA2  // Low 8-bit of LIB Version info
#define FT62XX_REG_CIPHER                 0xA3  // Chip vendor ID
#define FT62XX_REG_G_MODE                 0xA4  // Interrupt operating mode
#define FT62XX_REG_PWR_MODE               0xA5  // Current power mode
#define FT62XX_REG_FIRMID                 0xA6  // Firmware version
#define FT62XX_REG_FOCALTECH_ID           0xA8  // Focal Tech Panel ID
#define FT62XX_REG_RELEASE_CODE_ID        0xAF  // Release Code version
#define FT62XX_REG_STATE                  0xBC  // Current operating mode

#define FT62XX_VALUE_CHIP_ID              0x06  // Default Chip ID
#define FT62XX_VALUE_PANEL_ID             0x11  // Default Panel ID
#define FT62XX_VALUE_RELEASE_CODE         0x01  // Default Release Code ID

// FT6206 Gesture IDs
#define FT62XX_GESTURE_NONE               0x00  // Gesture: None
#define FT62XX_GESTURE_MOVE_UP            0x10  // Gesture: Move Up
#define FT62XX_GESTURE_MOVE_RIGHT         0x14  // Gesture: Move Right
#define FT62XX_GESTURE_MOVE_DOWN          0x18  // Gesture: Move Down
#define FT62XX_GESTURE_MOVE_LEFT          0x1C  // Gesture: Move Left
#define FT62XX_GESTURE_ZOOM_IN            0x48  // Gesture: Zoom In
#define FT62XX_GESTURE_ZOOM_OUT           0x49  // Gesture: Zoom Out

// FT6206 Touch Events
#define FT62XX_TOUCH_DOWN                 0x00  // Touch Event: Press Down
#define FT62XX_TOUCH_UP                   0x01  // Touch Event: Lift Up
#define FT62XX_TOUCH_CONTACT              0x02  // Touch Event: Contact
#define FT62XX_TOUCH_NONE                 0x03  // Touch Event: No event

#define FT62XX_INTERRUPT_POLLING          0x00  // Polling mode
#define FT62XX_INTERRUPT_TRIGGER          0x01  // Trigger mode

#define FT62XX_DEFAULT_THRESHOLD          128 // Default threshold for touch detection


//============================================================================================//
/*!
  @brief  Class that stores state and functions for interacting with FT6206
  capacitive touch chips.
*/
class CSE_FT6206 {
  public:
    // These values are common to both P1 and P2 touch points
    uint8_t touches;  // Number of touches registered
    uint8_t gestureID;  // The gesture ID of the touch
    uint16_t defWidth, defHeight; // Default width and height of the touch screen
    uint16_t width, height; // The size of the touch screen

    // FT6206 can detect two touch points, P1 and P2 at the same time.
    // So have two sets of values for each point.
    uint8_t touchArea [FT6206_MAX_TOUCH_POINTS];
    uint8_t rotation;

    CSE_TouchPoint touchPoints [FT6206_MAX_TOUCH_POINTS];

    CSE_FT6206 (uint16_t width, uint16_t height, TwoWire *i2c = &Wire, int8_t pinRst = -1, int8_t pinIrq = -1);

    // Initialization functions.
    
    bool begin();

    // Configuration functions.

    bool setThreshold (uint8_t threshold = FT62XX_DEFAULT_THRESHOLD);
    uint8_t getActiveScanRate (void);
    uint8_t getMonitorScanRate (void);
    bool setMonitorScanRate (uint8_t rate = 6);
    bool setActiveScanRate (uint8_t rate = 6);
    uint8_t getInterruptMode (void);
    bool setInterruptMode (uint8_t mode = 1);

    // Data functions.

    void readData (void);
    void fastReadData (uint8_t id = 0);
    bool isTouched (void); // Returns true if there are any touches detected
    bool isTouched (uint8_t id); // Returns true if there are any touches detected
    CSE_TouchPoint getPoint (uint8_t n = 0);  // By default, P1 touch point is returned
    uint8_t getTouches (void);  // Returns the number of touches detected
    uint8_t getGestureID (void);
    String getGestureName (void);
    
    // Utility functions.

    uint8_t setRotation (uint8_t r = 0);  // Set the rotation of the touch panel (0-3
    uint8_t getRotation();  // Set the rotation of the touch panel (0-3
    uint16_t getWidth();
    uint16_t getHeight();

    // Data transfer functions.

    void writeRegister8 (uint8_t reg, uint8_t val); // Write an 8 bit value to a register
    uint8_t readRegister8 (uint8_t reg);  // Read an 8 bit value from a register

  private:
    TwoWire *wireInstance; // Touch panel I2C
    int8_t pinReset;  // Touch panel reset pin
    int8_t pinInterrupt;  // Touch panel reset pin
    bool inited;
};

//============================================================================================//

#endif // CSE_FT6206_LIBRARY
