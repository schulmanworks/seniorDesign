#include <Wire.h>

#include <GOFi2cOLED.h>

#include <EEPROM.h>


#define COUNT_ADDR 0
#define EEPROM_CANARY 10

const int pushButtonPin = 11;     // the number of the pushbutton pin
const int closeButtonPin = 12;
const int ledPin =  13;      // the number of the LED pin

// variables will change:
volatile int pushButtonState = 0;         // variable for reading the pushbutton status
volatile int closeButtonState = 0;
volatile unsigned int count = 0;



void setup() {
  uint8_t value;
  uint8_t canary;
  unsigned int tmp_count;
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(pushButtonPin, INPUT);
  // Attach an interrupt to the ISR vector
  attachInterrupt(0, pushButton_ISR, CHANGE);
  attachInterrupt(0, closeButton_ISR, CHANGE);


  //If the EEPROM has been initialized, grab the value
  canary = EEPROM.read(EEPROM_CANARY);
  if (!canary) {
    for(int i = 0; i < 4; i++) {
      value = EEPROM.read(COUNT_ADDR + i);
      count = value << i*sizeof(byte);
    }
  }

  // If the EEPROM has not been touched, clear it
  else {
    for(int i = 0; i < 4; i++) {
      EEPROM.write(COUNT_ADDR + i, 0);
    }
    EEPROM.write(EEPROM_CANARY, 0);
    count = 0;
  }
}

void loop() {
  // Nothing here!
}

void closeButton_ISR() {
 closeButtonState = digitalRead(pushButtonPin);
}

void pushButton_ISR() {
  pushButtonState = digitalRead(pushButtonPin);

}
