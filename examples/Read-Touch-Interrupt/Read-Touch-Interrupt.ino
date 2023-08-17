

//===================================================================================//

#include <Wire.h>
#include <CSE_FT6206.h>

#define FT6206_PIN_RST  2
#define FT6206_PIN_INT  4
#define FT6206_PIN_SDA  12
#define FT6206_PIN_SCL  13

//===================================================================================//

// Create a new instance of the FT6206 class and send the Wire instance,
// and the reset pin to the constructor.
// You can leave all of these parameters if you want to use the default values.
// The default values are: Wire, -1.
CSE_FT6206 tsPanel = CSE_FT6206 (&Wire, FT6206_PIN_RST);

bool intReceived = false; // Flag to indicate that an interrupt has been received

//===================================================================================//

void setup() {
  Serial.begin (115200);
  delay (2000);
  Serial.println ("FT6206 Touch Controller Test");

  // // Set the I2C pins if your board allows it
  // Wire.setSDA (FT6206_PIN_SDA);
  // Wire.setSCL (FT6206_PIN_SCL);
  Wire.begin();

  tsPanel.begin();
  tsPanel.setActiveScanRate (60);
  tsPanel.setMonitorScanRate (60);

  // Change the interrupt mode to trigger or polling and observe the difference
  tsPanel.setInterruptMode (FT62XX_INTERRUPT_TRIGGER);
  // tsPanel.setInterruptMode (FT62XX_INTERRUPT_POLLING);

  // Either use internal pullup or external pullup. Line is active low.
  pinMode (FT6206_PIN_INT, INPUT_PULLUP);

  attachInterrupt (digitalPinToInterrupt (FT6206_PIN_INT), readTouch, FALLING);
  delay (1000);
}

//===================================================================================//

void loop() {
  if (intReceived) {
    tsPanel.readData();

    for (uint8_t i = 0; i < tsPanel.touches; i++) {
      Serial.println();
      Serial.print ("ID #");
      Serial.print (tsPanel.touchID [i]);
      Serial.print ("\t(");
      Serial.print (tsPanel.touchX [i]);
      Serial.print (", ");
      Serial.print (tsPanel.touchY [i]);
      Serial.print (", ");
      Serial.print (tsPanel.touchWeight [i]);
      Serial.print (", ");
      Serial.print (tsPanel.touchArea [i]);
      Serial.print (", ");
      Serial.print (tsPanel.touchEvent [i]);
      Serial.print (") ");
    }
    Serial.println();
    // delay (50);  // Set a delay here if you want to wait for some time before reading again.
    intReceived = false;
    attachInterrupt (digitalPinToInterrupt (FT6206_PIN_INT), readTouch, FALLING);
  }
}

//===================================================================================//

void readTouch() {
  // Detach the interrupt to prevent multiple interrupts
  detachInterrupt (digitalPinToInterrupt (FT6206_PIN_INT));
  intReceived = true;
}

//===================================================================================//