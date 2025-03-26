
//============================================================================================//
/*
  Filename: CSE_FT6206.cpp
  Description: Main source file for the CSE_FT6206 Arduino library.
  Framework: Arduino, PlatformIO
  Author: Vishnu Mohanan (@vishnumaiea, @vizmohanan)
  Maintainer: CIRCUITSTATE Electronics (@circuitstate)
  Version: 0.0.2
  License: MIT
  Source: https://github.com/CIRCUITSTATE/CSE_FT6206
  Last Modified: +05:30 00:00:00 AM 27-03-2025, Thursday
 */
//============================================================================================//

#include "CSE_FT6206.h"

//============================================================================================//
/*!
  @brief  Instantiates a new FT6206 class.
*/
CSE_FT6206:: CSE_FT6206 (uint16_t width, uint16_t height, TwoWire *i2c, int8_t pinRst, int8_t pinIrq) {
  wireInstance = i2c;
  pinReset = pinRst;
  pinInterrupt = pinIrq;

  // Store the default width and height.
  // This will be later used when performing rotations.
  defWidth = width;
  defHeight = height;

  // Set the current width and height (will be adjusted for rotation).
  this->width = width;
  this->height = height;

  // Initialize the touch points.
  for (int i = 0; i < FT6206_MAX_TOUCH_POINTS; i++) {
    touchPoints [i] = CSE_TouchPoint (0, 0, 0, i);
  }

  touches = 0;

  // Initialize other variables.
  rotation = 0;
  inited = false;
}

//============================================================================================//
/*!
  @brief  Setups the I2C interface and hardware, identifies if chip is found.

  @returns True if an FT6206 is found, false on any failure.
*/
bool CSE_FT6206:: begin () {
  if (inited) {
    return true;
  }
  
  // Initialize I2C if not already done.
  wireInstance->begin();

  if (pinReset >= 0) {
    pinMode (pinReset, OUTPUT);
    digitalWrite (pinReset, HIGH);
    delay (10);
    digitalWrite (pinReset, LOW);
    delay (10);
    digitalWrite (pinReset, HIGH);
    delay (10);
  }

  delay (300);  // FT6206 needs a delay after reset

  if (pinInterrupt != -1) {
    pinMode (pinInterrupt, INPUT_PULLUP);
  }

  #ifdef CSE_FT6206_DEBUG
    DEBUG_SERIAL.print ("Vendor ID: 0x");
    DEBUG_SERIAL.println (readRegister8 (FT62XX_REG_FOCALTECH_ID), HEX);
    DEBUG_SERIAL.print ("Chip ID: 0x");
    DEBUG_SERIAL.println (readRegister8 (FT62XX_REG_CIPHER), HEX);
    DEBUG_SERIAL.print ("Firmware Version: ");
    DEBUG_SERIAL.println (readRegister8 (FT62XX_REG_FIRMID));
    DEBUG_SERIAL.print ("Sample Rate Hz: ");
    DEBUG_SERIAL.println (readRegister8 (FT62XX_REG_PERIODACTIVE));
    DEBUG_SERIAL.print ("Threshold: ");
    DEBUG_SERIAL.println (readRegister8 (FT62XX_REG_TH_GROUP));

    // dump all registers
    for (int16_t i = 0; i < 0x10; i++) {
      DEBUG_SERIAL.print ("I2C $");
      DEBUG_SERIAL.print (i, HEX);
      DEBUG_SERIAL.print (" = 0x");
      DEBUG_SERIAL.println (readRegister8 (i), HEX);
    }

    if (readRegister8 (FT62XX_REG_CIPHER) != FT62XX_VALUE_CHIP_ID) {
      DEBUG_SERIAL.print ("Returned Chip ID 0x");
      DEBUG_SERIAL.print (readRegister8 (FT62XX_REG_CIPHER), HEX);
      DEBUG_SERIAL.print (" is different from expected 0x");
      DEBUG_SERIAL.println (FT62XX_VALUE_CHIP_ID, HEX);
      // return false;
    }

    if (readRegister8 (FT62XX_REG_FOCALTECH_ID) != FT62XX_VALUE_PANEL_ID) {
      DEBUG_SERIAL.print ("Returned Vendor ID 0x");
      DEBUG_SERIAL.print (readRegister8 (FT62XX_REG_FOCALTECH_ID), HEX);
      DEBUG_SERIAL.print (" is different from expected 0x");
      DEBUG_SERIAL.println (FT62XX_VALUE_PANEL_ID, HEX);
      // return false;
    }
  #endif

  return setThreshold();
}

//============================================================================================//
/**
 * @brief Set the rotation of the touch panel.
 * 
 * @param r  Rotation (0-3, where 0=0°, 1=90°, 2=180°, 3=270°).
 * @return uint8_t Current rotation setting.
 */
uint8_t CSE_FT6206:: setRotation (uint8_t r) {
  rotation = r % 4; // Ensure rotation is 0-3
  
  // Update width and height based on rotation.
  switch (rotation) {
    case 0:
    case 2:
      width = defWidth;
      height = defHeight;
      break;
    case 1:
    case 3:
      width = defHeight;
      height = defWidth;
      break;
  }
  
  return rotation;
}

//============================================================================================//
/**
 * @brief Get the current rotation setting.
 * 
 * @return `uint8_t` Current rotation (0-3).
 */
uint8_t CSE_FT6206:: getRotation() {
  return rotation;
}

//============================================================================================//
/**
 * @brief Get the current width considering rotation.
 * 
 * @return `uint16_t` Width in pixels.
 */
uint16_t CSE_FT6206:: getWidth() {
  return width;
}

//============================================================================================//
/**
 * @brief Get the current height considering rotation.
 * 
 * @return `uint16_t` Height in pixels.
 */
uint16_t CSE_FT6206:: getHeight() {
  return height;
}

//============================================================================================//
/*!
  @brief  Sets the threshold for when a touch is detected.

  @param  threshold Optional threshhold-for-touch value, default is
  FT6206_DEFAULT_THRESSHOLD but you can try changing it if your screen is
  too/not sensitive.
*/
bool CSE_FT6206:: setThreshold (uint8_t threshold) {
  // change threshhold to be higher/lower
  writeRegister8 (FT62XX_REG_TH_GROUP, threshold);
  return true;
} 

//============================================================================================//
/*!
  @brief  Determines if there are any touches detected.

  @returns Number of touches detected, can be 0, 1 or 2.
*/
uint8_t CSE_FT6206:: getTouches (void) {
  uint8_t n = readRegister8 (FT62XX_REG_TD_STATUS);

  if (n > 2) {
    n = 0;
  }
  return n;
}

//============================================================================================//
/**
 * @brief Checks if the finger id is being touched right now.
 * 
 * @param id The id of the finger.
 * @return `bool` True if touched, false if not.
 */
bool CSE_FT6206:: isTouched (uint8_t id) {
  readData();
  
  // Check if the touch id is greater than supported.
  if (id >= FT6206_MAX_TOUCH_POINTS) {
    return false;
  }

  if (touchPoints [id].state == 1) { // Check if the point is touched.
    return true;
  }

  return false;
}

//============================================================================================//
/**
 * @brief Check if the screen is being touched by checking all touch points.
 * @returns  True if touched, false if not.
 */
bool CSE_FT6206:: isTouched (void) {
  return (getTouches() > 0);
}

//============================================================================================//
/*!
  @brief  Queries the chip and retrieves a point data.

  @param  n The # index (0 or 1) to the points we can detect. In theory we can
  detect 2 points but we've found that you should only use this for
  single-touch since the two points cant share the same half of the screen.

  @returns CSE_TouchPoint object that has the x and y coordinets set. If the
  z coordinate is 0 it means the point is not touched. If z is 1, it is
  currently touched.
*/
CSE_TouchPoint CSE_FT6206:: getPoint (uint8_t n) {
  readData();

  if (n >= FT6206_MAX_TOUCH_POINTS) {
    return CSE_TouchPoint();
  }
  
  return touchPoints [n];
}

//============================================================================================//
/*!
  @brief  Reads the report rate or scan rate in the active mode. The default value
  is usually 6 Hz and max is 14 hz. You can modify this with setMonitorScanRate().

  @returns True if the data was read successfully, false on any failure.
*/
uint8_t CSE_FT6206:: getActiveScanRate() {
  return readRegister8 (FT62XX_REG_PERIODACTIVE);
}

//============================================================================================//
/*!
  @brief  Reads the report rate or scan rate in the monitor/sleeping mode. The default value
  is usually 40 Hz. You can modify this with setMonitorScanRate().

  But why is monitor scan rate higher than active scan rate? I don't know.

  @returns True if the data was read successfully, false on any failure.
*/
uint8_t CSE_FT6206:: getMonitorScanRate() {
  return readRegister8 (FT62XX_REG_PERIODMONITOR);
}

//============================================================================================//
/*!
  @brief  Sets the report rate or scan rate in the active mode.

  @param  rate The rate to set the active scan rate to. The default is 6 Hz.

  @returns True if the data was read successfully, false on any failure.
*/
bool CSE_FT6206:: setMonitorScanRate (uint8_t rate) {
  writeRegister8 (FT62XX_REG_PERIODMONITOR, rate);
  return true;
}

//============================================================================================//
/*!
  @brief  Sets the report rate or scan rate in the monitor/sleeping mode.

  @param  rate The rate to set the monitor scan rate to. The default is 6 Hz.

  @returns True if the data was read successfully, false on any failure.
*/
bool CSE_FT6206:: setActiveScanRate (uint8_t rate) {
  writeRegister8 (FT62XX_REG_PERIODACTIVE, rate);
  return true;
}

//============================================================================================//
/*!
  @brief  Reads the interrupt output mode. The default value is 0x1 (Trigger mode).

  @returns The interrupt mode; either 1 (trigger mode) or 0 (polling mode).
*/
uint8_t CSE_FT6206:: getInterruptMode() {
  return readRegister8 (FT62XX_REG_G_MODE);
}

//============================================================================================//
/*!
  @brief  Sets the interrupt output mode. The default value is 0x1 (Trigger mode).

  @param  mode The interrupt mode; either 1 (trigger mode) or 0 (polling mode).

  @returns True if the data was read successfully, false on any failure.
*/
bool CSE_FT6206:: setInterruptMode (uint8_t mode) {
  if (mode > 1) {
    mode = 1;
  }
  writeRegister8 (FT62XX_REG_G_MODE, mode);
  return true;
}

//============================================================================================//
/*!
  @brief  Reads the current gesture ID.

  @returns The current gesture ID.
*/
uint8_t CSE_FT6206:: getGestureID() {
  return readRegister8 (FT62XX_REG_GEST_ID);
}

//============================================================================================//
/*!
  @brief  Reads the current gesture ID and returns a string with the gesture name.

  @returns The current gesture name.
*/
String CSE_FT6206:: getGestureName() {
  uint8_t gestureID = getGestureID();
  
  switch (gestureID) {
    case FT62XX_GESTURE_MOVE_UP:
      return String ("Move Up");
    case FT62XX_GESTURE_MOVE_RIGHT:
      return String ("Move Right");
    case FT62XX_GESTURE_MOVE_DOWN:
      return String ("Move Down");
    case FT62XX_GESTURE_MOVE_LEFT:
      return String ("Move Left");
    case FT62XX_GESTURE_ZOOM_IN:
      return String ("Zoom In");
    case FT62XX_GESTURE_ZOOM_OUT:
      return String ("Zoom Out");
    default:
      return String ("None");
  } 
}

//============================================================================================//
/*!
  @brief  Reads the touch-related data from the FT6206 and saves them to the
  class variables. The reading is done in one go.
*/
void CSE_FT6206:: readData (void) {
  uint8_t i2cdat [16];  // Hols the first 16 bytes of data from the FT6206

  // Send the register address to start reading data from
  wireInstance->beginTransmission (FT62XX_I2C_ADDR);
  wireInstance->write (byte (FT62XX_REG_DEV_MODE));
  wireInstance->endTransmission();
  wireInstance->requestFrom (byte (FT62XX_I2C_ADDR), byte (16));

  // Read the first 16 bytes of data which have the information about the touches.
  for (uint8_t i = 0; i < 16; i++) {
    if (wireInstance->available()) {
      i2cdat [i] = wireInstance->read();
    }
  }

  #ifdef CSE_FT6206_DEBUG
    // Optionally print the register data
    for (int16_t i = 0; i < 16; i++) {
      DEBUG_SERIAL.print ("I2C Reg# ");
      DEBUG_SERIAL.print (i, HEX);
      DEBUG_SERIAL.print (" = 0x");
      DEBUG_SERIAL.println (i2cdat [i], HEX);
    }
  #endif

  // Save the touch count
  touches = i2cdat [FT62XX_REG_TD_STATUS];

  // The touch count can be 1-2 only as per the datasheet.
  // Values other than that are invalid.
  if ((touches > 2) || (touches == 0)) {
    touches = 0;
  }

  // Save the gesture ID
  gestureID = i2cdat [FT62XX_REG_GEST_ID];

  #ifdef CSE_FT6206_DEBUG
    DEBUG_SERIAL.print ("# Touches: ");
    DEBUG_SERIAL.println (touches);

    if (i2cdat [FT62XX_REG_GEST_ID] != 0x00) {
      DEBUG_SERIAL.print ("Gesture #");
      DEBUG_SERIAL.println (i2cdat [FT62XX_REG_GEST_ID]);
    }
  #endif

  // Extract the touch point data from the raw data.
  // Since we have identical set of registers for two touch points,
  // we can run them trhough a loop of two. Adding 6 to the register
  // is because the next identical set of registers is 6 bytes away.
  for (uint8_t i = 0; i < FT6206_MAX_TOUCH_POINTS; i++) {
    touchEvent [i] = i2cdat [FT62XX_REG_P1_XH + (i * 6)] >> 6;  // Shifting right by 6 gives us the 2 MSB bits [7:6]

    touchX [i] = i2cdat [FT62XX_REG_P1_XH + (i * 6)] & 0x0F;  // Only the last 4 bits are the X coordinate [11:8]
    touchX [i] <<= 8; // Shift the MSBs to place
    touchX [i] |= i2cdat [FT62XX_REG_P1_XL + (i * 6)];  // Now add the LSBs [7:0]

    touchY [i] = i2cdat [FT62XX_REG_P1_YH + (i * 6)] & 0x0F;  // Only the last 4 bits are the Y coordinate [11:8]
    touchY [i] <<= 8; // Shift the MSBs to place
    touchY [i] |= i2cdat [FT62XX_REG_P1_YL + (i * 6)];  // Now add the LSBs [7:0]

    touchID [i] = i2cdat [FT62XX_REG_P1_YH + (i * 6)] >> 4; // Only 4 bits are valid [7:4]

    touchWeight [i] = i2cdat [FT62XX_REG_P1_WEIGHT + (i * 6)];  // 8 bits of weight [7:0]
    touchArea [i] = i2cdat [FT62XX_REG_P1_MISC + (i * 6)] >> 4; // Only 4 bits are valid [7:4]
  }

  // Save first touch point data.
  touchPoints [0].state = touchEvent [0];
  touchPoints [0].x = touchX [0];
  touchPoints [0].y = touchY [0];
  touchPoints [0].z = touchWeight [0];
  touchPoints [0].id = touchID [0];

  // Save second touch point data.
  touchPoints [1].state = touchEvent [1];
  touchPoints [1].x = touchX [1];
  touchPoints [1].y = touchY [1];
  touchPoints [1].z = touchWeight [1];
  touchPoints [1].id = touchID [1];

  // Apply rotation if necessary
  for (uint8_t i = 0; i < FT6206_MAX_TOUCH_POINTS; i++) {
    CSE_TouchPoint point = touchPoints [i];
    
    switch (rotation) {
      case 0:
        touchPoints [i].x = point.x;
        touchPoints [i].y = point.y;
        break;
      case 1:
        touchPoints [i].x = point.y;
        touchPoints [i].y = height - point.x; // Mirror X
        break;
      case 2:
        touchPoints [i].x = width - point.x; // Mirror X
        touchPoints [i].y = height - point.y; // Mirror Y
        break;
      case 3:
        touchPoints [i].x = width - point.y; // Mirror Y
        touchPoints [i].y = point.x;
        break;
    }
  }

  #ifdef CSE_FT6206_DEBUG
    for (uint8_t i = 0; i < touches; i++) {
      DEBUG_SERIAL.print ("ID #");
      DEBUG_SERIAL.print (touchID [i]);
      DEBUG_SERIAL.print ("\t(");
      DEBUG_SERIAL.print (touchX [i]);
      DEBUG_SERIAL.print (", ");
      DEBUG_SERIAL.print (touchY [i]);
      DEBUG_SERIAL.print (", ");
      DEBUG_SERIAL.print (touchWeight [i]);
      DEBUG_SERIAL.print (", ");
      DEBUG_SERIAL.print (touchArea [i]);
      DEBUG_SERIAL.print (", ");
      DEBUG_SERIAL.print (touchEvent [i]);
      DEBUG_SERIAL.print (") ");
    }
    DEBUG_SERIAL.println();
  #endif
}

//============================================================================================//
/*!
  @brief  Reads the 8-bits from the specified register.

  @param  reg The register to read from.

  @returns The 8-bit value that was read from the register.
*/
uint8_t CSE_FT6206:: readRegister8 (uint8_t reg) {
  uint8_t value;

  wireInstance->beginTransmission (FT62XX_I2C_ADDR);
  wireInstance->write (byte (reg));
  wireInstance->endTransmission();

  wireInstance->requestFrom (byte (FT62XX_I2C_ADDR), byte (1));

  if (wireInstance->available()) {
    value = wireInstance->read();
  }

  #ifdef CSE_FT6206_I2C_DEBUG
    DEBUG_SERIAL.print ("$");
    DEBUG_SERIAL.print (reg, HEX);
    DEBUG_SERIAL.print (": 0x");
    DEBUG_SERIAL.println (x, HEX);
  #endif

  return value;
}

//============================================================================================//
/*!
  @brief  Writes an 8 bit value to the specified register location.

  @param  reg The register to write to.
  @param  val The 8-bit value to write to the register.
*/
void CSE_FT6206:: writeRegister8 (uint8_t reg, uint8_t val) {
  wireInstance->beginTransmission (FT62XX_I2C_ADDR);
  wireInstance->write (byte (reg));
  wireInstance->write (byte (val));
  wireInstance->endTransmission();
}

//============================================================================================//
