
//============================================================================================//
/*
  Filename: Read-Touch-Polling.ino
  Description: Example Arduino sketch from the CSE_FT6206 Arduino library.
  Reads the touch sensor through polling method and prints the data to the serial monitor.
  This code was written for and tested with FireBeetle-ESP32E board.
  
  Framework: Arduino, PlatformIO
  Author: Vishnu Mohanan (@vishnumaiea, @vizmohanan)
  Maintainer: CIRCUITSTATE Electronics (@circuitstate)
  Version: 0.0.3
  License: MIT
  Source: https://github.com/CIRCUITSTATE/CSE_FT6206
  Last Modified: +05:30 08:55:15 AM 28-03-2025, Friday
 */
//============================================================================================//

#include <Wire.h>
#include <CSE_FT6206.h>

#define FT6206_PIN_RST  4
#define FT6206_PIN_INT  -1
#define FT6206_PIN_SDA  21
#define FT6206_PIN_SCL  22

//============================================================================================//

// Parameters: Width, Height, &Wire, Reset pin, Interrupt pin
CSE_FT6206 tsPanel = CSE_FT6206 (240, 320, &Wire, FT6206_PIN_RST, FT6206_PIN_INT);

//============================================================================================//

void setup() {
  Serial.begin (115200);
  delay (100);

  Serial.println();
  Serial.println ("== CSE_FT6206: Read-Touch-Polling ==");

  // // Initialize the I2C interface (for ESP32).
  // Wire.begin (FT6206_PIN_SDA, FT6206_PIN_SCL);

  Wire.begin();

  tsPanel.begin();
  tsPanel.setActiveScanRate (60);
  tsPanel.setMonitorScanRate (60);
  
  delay (100);
}

//============================================================================================//

void loop() {
  readTouch();
  delay (100);
}

//============================================================================================//
/**
 * @brief Reads the touches from the panel and print their info to the serial monitor.
 * 
 */
void readTouch() {
  uint8_t touches = tsPanel.getTouches(); // Get the number of touches currently detected.

  if (touches > 0) { // If any touches are detected.
    tsPanel.readData();
    
    for (uint8_t i = 0; i < touches; i++) {
      Serial.print ("ID: ");
      Serial.print (i);
      Serial.print (", X: ");
      Serial.print (tsPanel.getPoint (i).x);
      Serial.print (", Y: ");
      Serial.print (tsPanel.getPoint (i).y);
      Serial.print (", Z: ");
      Serial.print (tsPanel.getPoint (i).z);
      Serial.print (", State: ");
      Serial.print (tsPanel.getPoint (i).state);
      if (tsPanel.getPoint (i).state == 0) Serial.println (" (Press Down)");
      else if (tsPanel.getPoint (i).state == 1) Serial.println (" (Lift Up)");
      else if (tsPanel.getPoint (i).state == 2) Serial.println (" (Contact)");
      else if (tsPanel.getPoint (i).state == 3) Serial.println (" (No Event)");
    }
  }
  else {
    Serial.println ("No touches detected");
  }
}

//============================================================================================//