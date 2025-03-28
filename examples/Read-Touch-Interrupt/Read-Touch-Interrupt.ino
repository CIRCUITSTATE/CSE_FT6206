
//============================================================================================//
/*
  Filename: Read-Touch-Interrupt.ino
  Description: Example Arduino sketch from the CSE_FT6206 Arduino library.
  Reads the touch sensor through interrupt method and prints the data to the serial monitor.
  This code was written for and tested with FireBeetle-ESP32E board.
  
  Framework: Arduino, PlatformIO
  Author: Vishnu Mohanan (@vishnumaiea, @vizmohanan)
  Maintainer: CIRCUITSTATE Electronics (@circuitstate)
  Version: 0.0.3
  License: MIT
  Source: https://github.com/CIRCUITSTATE/CSE_FT6206
  Last Modified: +05:30 08:55:18 AM 28-03-2025, Friday
 */
//============================================================================================//

#include <Wire.h>
#include <CSE_FT6206.h>

#define FT6206_PIN_RST  4
#define FT6206_PIN_INT  25
#define FT6206_PIN_SDA  21
#define FT6206_PIN_SCL  22

//============================================================================================//

// Parameters: Width, Height, &Wire, Reset pin, Interrupt pin
CSE_FT6206 tsPanel = CSE_FT6206 (240, 320, &Wire, FT6206_PIN_RST, FT6206_PIN_INT);

bool intReceived = false; // Flag to indicate that an interrupt has been received

//============================================================================================//

void setup() {
  Serial.begin (115200);
  delay (100);

  Serial.println();
  Serial.println ("== CSE_FT6206: Read-Touch-Interrupt ==");

  // // Initialize the I2C interface (for ESP32).
  // Wire.begin (FT6206_PIN_SDA, FT6206_PIN_SCL);

  Wire.begin();

  tsPanel.begin();
  tsPanel.setActiveScanRate (60);
  tsPanel.setMonitorScanRate (60);

  // Change the interrupt mode to trigger or polling and observe the difference.
  tsPanel.setInterruptMode (FT62XX_INTERRUPT_TRIGGER); // Continous triggering
  // tsPanel.setInterruptMode (FT62XX_INTERRUPT_POLLING); // One trigger at a time

  // Either use internal pullup or external pullup. Line is active low.
  pinMode (FT6206_PIN_INT, INPUT_PULLUP);

  // Attach the interrupt function.
  attachInterrupt (digitalPinToInterrupt (FT6206_PIN_INT), touchISR, FALLING);
  delay (1000);

}

//============================================================================================//

void loop() {
  if (intReceived) {
    readTouch();
    intReceived = false;
    attachInterrupt (digitalPinToInterrupt (FT6206_PIN_INT), touchISR, FALLING);
  }
}

//============================================================================================//
/**
 * @brief Reads all touch points from the panel and print their info to the serial monitor.
 * 
 */
void readTouch() {
  // Read all touch data.
  // tsPanel.readData();

  // Print the touch point data.
  for (uint8_t i = 0; i < 2; i++) {
    if (tsPanel.isTouched (i)) {
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
}

//============================================================================================//

void touchISR() {
  // Detach the interrupt to prevent multiple interrupts
  detachInterrupt (digitalPinToInterrupt (FT6206_PIN_INT));
  intReceived = true;
}

//============================================================================================//