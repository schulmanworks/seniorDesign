#include <Wire.h>

#include <GOFi2cOLED.h>

#include <EEPROM.h>

// #include <Serial.h>


#define COUNT_ADDR 0
#define EEPROM_CANARY 10
#define OPEN = 1
#define CLOSE = 0


enum State {Initial=0, Neutral=1, Closed = 2, Opened = 3, Open = 4};


const int pushButtonPin = 2;     // the number of the pushbutton pin
const int closeButtonPin = 3;
const int relayPin1 = 12;
const int relayPin2 = 13;
// const int ledPin =  13;      // the number of the LED pin

// variables will change:
volatile int pushButtonState = 0;         // variable for reading the pushbutton status
volatile int closeButtonState = 0;
volatile unsigned int count = 0;
enum State curState = Initial;


void setup() {
  uint8_t value;
  uint8_t canary;
  unsigned int tmp_count;
  // initialize the LED pin as an output:
  // pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  Serial.begin(9600);

  pinMode(closeButtonPin, INPUT);
  pinMode(pushButtonPin, INPUT);

  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin1, OUTPUT);
  // Attach an interrupt to the ISR vector
  attachInterrupt(0, pushButton_ISR, CHANGE);
  attachInterrupt(1, closeButton_ISR, CHANGE);


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
  // Nothing here!]
  curState = Neutral;
  int oldCount = -1;
  enum State oldState = Initial;

  while(1){
    if (oldCount != count) {
      Serial.print("Count:");
      Serial.print(count);
      Serial.print("\n");
      oldCount = count;
    }
    if (oldState != curState) {
      Serial.print("state:");
      Serial.print(curState);
      Serial.print(" count: ");
      Serial.print(count);
      Serial.print("\n");
      oldState = curState;
    }
  }
}

void processStateMachine() {
  switch (curState) {
    case Initial:
      //load data from EEPROM
      if(!pushButtonState && !closeButtonState){
        curState = Neutral;
      }
      else
      {
        curState = Initial;
      }
      break;

    case Neutral:
      if(pushButtonState){
        curState = Closed;
        // turn magnet off
        digitalWrite(relayPin1, 0);
        digitalWrite(relayPin2, 1)
      }
      else{
        curState = Neutral;
      }
      break;

    case Closed:
      if(pushButtonState && !closeButtonState){
        curState = Closed;
      }
      if(closeButtonState){
        curState = Opened;
        count++;
      }
      else{
        curState = Neutral;
      }
      break;

    // case Opened:
    //     count++;
    //     //magnet should already be off
    //     curState = Open;
    //     break;

    case Opened:
      //Magnet
      if(!closeButtonState && pushButtonState){
        curState = Closed;
      }
      else{
        curState = Neutral;
        //turn magnet on
        digitalWrite(relayPin1, 1);
        digitalWrite(relayPin2, 0)
      }
      break;

  }
}

void closeButton_ISR() {
 closeButtonState = digitalRead(closeButtonPin);
 processStateMachine();
 // Serial.print("cl\n");
}

void pushButton_ISR() {
  pushButtonState = digitalRead(pushButtonPin);
  processStateMachine();
  // Serial.print("op\n");
}
