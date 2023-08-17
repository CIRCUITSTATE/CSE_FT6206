/*!
 * @file CSE_FT6206.cpp
 *
 * @mainpage Adafruit FT2606 Library
 *
 * @section intro_sec Introduction
 *
 * This is a library for the Adafruit Capacitive Touch Screens
 *
 * ----> http://www.adafruit.com/products/1947
 *
 * Check out the links above for our tutorials and wiring diagrams
 * This chipset uses I2C to communicate
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section author Author
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries.
 *
 * @section license License

 * MIT license, all text above must be included in any redistribution
 */

#include <Arduino.h>
#include <CSE_FT6206.h>
#include <Wire.h>

//==============================================================================//
/*!
  @brief  Instantiates a new FT6206 class.
*/
CSE_FT6206:: CSE_FT6206 (uint16_t width, uint16_t height, TwoWire *i2c, int8_t pinRst) {
  wireInstance = i2c;
  pinReset = pinRst;
  touches = 0;
  this->width = width;
  defWidth = width;
  this->height = height;
  defHeight = height;
}

//==============================================================================//
/*!
  @brief  Setups the I2C interface and hardware, identifies if chip is found.

  @returns True if an FT6206 is found, false on any failure.
*/
bool CSE_FT6206:: begin () {
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

  #ifdef CSE_FT6206_DEBUG
    debugSerial.print ("Vendor ID: 0x");
    debugSerial.println (readRegister8 (FT62XX_REG_FOCALTECH_ID), HEX);
    debugSerial.print ("Chip ID: 0x");
    debugSerial.println (readRegister8 (FT62XX_REG_CIPHER), HEX);
    debugSerial.print ("Firmware Version: ");
    debugSerial.println (readRegister8 (FT62XX_REG_FIRMID));
    debugSerial.print ("Sample Rate Hz: ");
    debugSerial.println (readRegister8 (FT62XX_REG_PERIODACTIVE));
    debugSerial.print ("Threshold: ");
    debugSerial.println (readRegister8 (FT62XX_REG_TH_GROUP));

    // dump all registers
    for (int16_t i = 0; i < 0x10; i++) {
      debugSerial.print ("I2C $");
      debugSerial.print (i, HEX);
      debugSerial.print (" = 0x");
      debugSerial.println (readRegister8 (i), HEX);
    }

    if (readRegister8 (FT62XX_REG_CIPHER) != FT62XX_VALUE_CHIP_ID) {
      debugSerial.print ("Returned Chip ID 0x");
      debugSerial.print (readRegister8 (FT62XX_REG_CIPHER), HEX);
      debugSerial.print (" is different from expected 0x");
      debugSerial.println (FT62XX_VALUE_CHIP_ID, HEX);
      // return false;
    }

    if (readRegister8 (FT62XX_REG_FOCALTECH_ID) != FT62XX_VALUE_PANEL_ID) {
      debugSerial.print ("Returned Vendor ID 0x");
      debugSerial.print (readRegister8 (FT62XX_REG_FOCALTECH_ID), HEX);
      debugSerial.print (" is different from expected 0x");
      debugSerial.println (FT62XX_VALUE_PANEL_ID, HEX);
      // return false;
    }
  #endif

  return setThreshold();
}

//==============================================================================//

uint8_t CSE_FT6206:: setRotation (uint8_t rotation) {
  uint8_t currentRotation = this->rotation;
  this->rotation = rotation;

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
  return currentRotation;
}

//==============================================================================//

uint8_t CSE_FT6206:: getRotation () {
  return rotation;
}

//==============================================================================//

uint16_t CSE_FT6206:: getWidth() {
  return width;
}

//==============================================================================//

uint16_t CSE_FT6206:: getHeight() {
  return height;
}

//==============================================================================//
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

//==============================================================================//
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

//==============================================================================//

bool CSE_FT6206:: isTouched (void) {
  return (getTouches() != 0);
}

//==============================================================================//
/*!
  @brief  Queries the chip and retrieves a point data.

  @param  n The # index (0 or 1) to the points we can detect. In theory we can
  detect 2 points but we've found that you should only use this for
  single-touch since the two points cant share the same half of the screen.

  @returns TS_Point object that has the x and y coordinets set. If the
  z coordinate is 0 it means the point is not touched. If z is 1, it is
  currently touched.
*/
TS_Point CSE_FT6206:: getPoint (uint8_t n) {
  readData();
  TS_Point point;

  if ((touches == 0) || (n > 1)) {
    return TS_Point (0, 0, 0);
  }
  else {
    point.x = touchX [n];
    point.y = touchY [n];
    point.z = 1;
    // switch (rotation) {
    //   case 0:
    //     point.x = touchX [n];
    //     point.y = touchY [n];
    //     break;
    //   case 1:
    //     point.x = touchY [n];
    //     point.y = defWidth - touchX [n]; // Mirror Y
    //     break;
    //   case 2:
    //     point.x = defWidth - touchX [n]; // Mirror X
    //     point.y = defHeight - touchY [n]; // Mirror Y
    //     break;
    //   case 3:
    //     point.x = defHeight - touchY [n]; // Mirror Y
    //     point.y = touchX [n];
    //     break;
    // }
  }

  return point;
}

//==============================================================================//
/*!
  @brief  Reads the report rate or scan rate in the active mode. The default value
  is usually 6 Hz and max is 14 hz. You can modify this with setMonitorScanRate().

  @returns True if the data was read successfully, false on any failure.
*/
uint8_t CSE_FT6206:: getActiveScanRate() {
  return readRegister8 (FT62XX_REG_PERIODACTIVE);
}

//==============================================================================//
/*!
  @brief  Reads the report rate or scan rate in the monitor/sleeping mode. The default value
  is usually 40 Hz. You can modify this with setMonitorScanRate().

  But why is monitor scan rate higher than active scan rate? I don't know.

  @returns True if the data was read successfully, false on any failure.
*/
uint8_t CSE_FT6206:: getMonitorScanRate() {
  return readRegister8 (FT62XX_REG_PERIODMONITOR);
}

//==============================================================================//
/*!
  @brief  Sets the report rate or scan rate in the active mode.

  @param  rate The rate to set the active scan rate to. The default is 6 Hz.

  @returns True if the data was read successfully, false on any failure.
*/
bool CSE_FT6206:: setMonitorScanRate (uint8_t rate) {
  writeRegister8 (FT62XX_REG_PERIODMONITOR, rate);
  return true;
}

//==============================================================================//
/*!
  @brief  Sets the report rate or scan rate in the monitor/sleeping mode.

  @param  rate The rate to set the monitor scan rate to. The default is 6 Hz.

  @returns True if the data was read successfully, false on any failure.
*/
bool CSE_FT6206:: setActiveScanRate (uint8_t rate) {
  writeRegister8 (FT62XX_REG_PERIODACTIVE, rate);
  return true;
}

//==============================================================================//
/*!
  @brief  Reads the interrupt output mode. The default value is 0x1 (Trigger mode).

  @returns The interrupt mode; either 1 (trigger mode) or 0 (polling mode).
*/
uint8_t CSE_FT6206:: getInterruptMode() {
  return readRegister8 (FT62XX_REG_G_MODE);
}

//==============================================================================//
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

//==============================================================================//
/*!
  @brief  Reads the current gesture ID.

  @returns The current gesture ID.
*/
uint8_t CSE_FT6206:: getGestureID() {
  return readRegister8 (FT62XX_REG_GEST_ID);
}

//==============================================================================//
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

//==============================================================================//
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
      debugSerial.print ("I2C Reg# ");
      debugSerial.print (i, HEX);
      debugSerial.print (" = 0x");
      debugSerial.println (i2cdat [i], HEX);
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
    debugSerial.print ("# Touches: ");
    debugSerial.println (touches);

    if (i2cdat [FT62XX_REG_GEST_ID] != 0x00) {
      debugSerial.print ("Gesture #");
      debugSerial.println (i2cdat [FT62XX_REG_GEST_ID]);
    }
  #endif

  // Extract the touch point data from the raw data.
  // Since we have identical set of registers for two touch points,
  // we can run them trhough a loop of two. Adding 6 to the register
  // is because the next identical set of registers is 6 bytes away.
  for (uint8_t i = 0; i < 2; i++) {
    touchEvent [i] = i2cdat [FT62XX_REG_P1_XH + (i * 6)] >> 6;  // Shfting right by 6 gives us the 2 MSB bits [7:6]

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

  #ifdef CSE_FT6206_DEBUG
    for (uint8_t i = 0; i < touches; i++) {
      debugSerial.print ("ID #");
      debugSerial.print (touchID [i]);
      debugSerial.print ("\t(");
      debugSerial.print (touchX [i]);
      debugSerial.print (", ");
      debugSerial.print (touchY [i]);
      debugSerial.print (", ");
      debugSerial.print (touchWeight [i]);
      debugSerial.print (", ");
      debugSerial.print (touchArea [i]);
      debugSerial.print (", ");
      debugSerial.print (touchEvent [i]);
      debugSerial.print (") ");
    }
    debugSerial.println();
  #endif
}

//==============================================================================//
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
    debugSerial.print ("$");
    debugSerial.print (reg, HEX);
    debugSerial.print (": 0x");
    debugSerial.println (x, HEX);
  #endif

  return value;
}

//==============================================================================//
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

//==============================================================================//
/*!
  @brief  Instantiates a new touch point object with x, y and z set to 0 by default.
*/
TS_Point:: TS_Point (void) {
  x = y = z = 0;
}

//==============================================================================//
/*!
  @brief  Instantiates a new touch point object with x, y and z set by params.

  @param  _x The X coordinate
  @param  _y The Y coordinate
  @param  _z The Z coordinate
*/
TS_Point:: TS_Point (int16_t _x, int16_t _y, int16_t _z) {
  x = _x;
  y = _y;
  z = _z;
}

//==============================================================================//
/*!
  @brief  Simple == comparator for two TS_Point objects.

  @returns True if x, y and z are the same for both points, False otherwise.
*/
bool TS_Point:: operator== (TS_Point p1) {
  return ((p1.x == x) && (p1.y == y) && (p1.z == z));
}

//==============================================================================//
/*!
  @brief  Simple != comparator for two TS_Point objects.
  @returns False if x, y and z are the same for both points, True otherwise.
*/
bool TS_Point:: operator!= (TS_Point p1) {
  return ((p1.x != x) || (p1.y != y) || (p1.z != z));
}

//==============================================================================//
