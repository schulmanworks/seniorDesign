#include <Wire.h>

#include <GOFi2cOLED.h>

#include <EEPROM.h>

// #include <PinChangeInt.h>

// #include <Serial.h>

void init_lcd(void);
void init_relay(void);
void magnetOn(void);
void magnetOff(void);
void updateLCD(void);

// States the door can be in for processStateMachine
enum State {Initial=0, Neutral=1, Closed = 2, Opened = 3, Open = 4};

// define pins
const int pushButtonPin = 2;
const int closeButtonPin = 3;
const int relayPin1 = 12;
const int relayPin2 = 13;
int processStateLock = 0;

// closeButtonState is 0 when door is closed
// pushbuttonState is 1 when button is pushed
volatile int pushButtonState = 0;         // variable for reading the pushbutton status
volatile int closeButtonState = 1;
volatile unsigned int count = 0;
volatile enum State curState = Initial;

// LCD Driver object
GOFi2cOLED GOFoled;

void init_lcd(void) {
  GOFoled.init(0x3C);  //initialze  OLED display

  GOFoled.display(); // show splashscreen
  delay(2000);
  GOFoled.clearDisplay();
  GOFoled.setCursor(0,0);
  GOFoled.setTextSize(2);
  GOFoled.setTextColor(WHITE);
}

void updateLCD(void) {
  char s[11];
  sprintf(s, "%u", count);
  GOFoled.clearDisplay();
  GOFoled.setCursor(0,0);
  GOFoled.println("Count:");
  GOFoled.println(s);
  GOFoled.display();
}
void printLCDStr(char* c) {
  // GOFoled.setCursor(0,0);
  GOFoled.println(c);
  GOFoled.display();
}

void init_relay(void) {
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin1, OUTPUT);
  magnetOn();
}

void setup() {
  Serial.begin(9600);

  count = 0;

  init_lcd();
  updateLCD();


  pinMode(closeButtonPin, INPUT);
  pinMode(pushButtonPin, INPUT);

  init_relay();

  processStateMachine();
}
int oldPush = -1;
int oldDoor = -1;
void loop() {
  // Nothing here!]
  // curState = Neutral;
  int oldCount = -1;
  enum State oldState = Neutral;

  while(1){
    closeButtonState = digitalRead(closeButtonPin);
    pushButtonState = digitalRead(pushButtonPin);
    // Only process on change
    if (oldPush != pushButtonState || oldDoor != closeButtonState) {
      processStateMachine();
      oldPush = pushButtonState;
      oldDoor = closeButtonState;
    }
    if (oldState != curState || oldCount != count) {
      Serial.print("push: ");
      Serial.print(pushButtonState);
      Serial.print(" door: ");
      Serial.print(closeButtonState);
      Serial.print("\n");
      Serial.print("state:");
      Serial.print(curState);
      Serial.print(" count: ");
      Serial.print(count);
      Serial.print("\n");
      Serial.println("lock: ");
      Serial.println(processStateLock);
      Serial.print("\n");
      oldState = curState;
      oldCount = count;
    }
  }
}
void magnetOn(void) {
  //turn magnet on
  digitalWrite(relayPin1, 1);
  digitalWrite(relayPin2, 0);
}
void magnetOff(void) {
  // turn magnet off
  digitalWrite(relayPin1, 0);
  digitalWrite(relayPin2, 1);
}

// Keep in mind, you need to act during the transition from one state to another.
// Don't do closing actions in the "Closed" block, do it on the transition from
// opened to closed
void processStateMachine() {
  switch (curState) {
    case Initial:
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
        magnetOff();
        delay(4000);
        closeButtonState = digitalRead(closeButtonPin);
        pushButtonState = digitalRead(pushButtonPin);
        processStateMachine();
      }
      else{
        curState = Neutral;
      }
      break;

    case Closed:
      if (closeButtonState) {
        curState = Opened;
      }
      else if(!pushButtonState){
        curState = Neutral;
        magnetOn();
      }
      else {
        curState = Closed;
      }
      break;

    case Opened:
      if(closeButtonState && !pushButtonState){
        curState = Opened;
      }
      else if(!closeButtonState && !pushButtonState) {
        curState = Neutral;
        count++;
        updateLCD();
        magnetOn();
      }
      else if(closeButtonState && pushButtonState){
        curState = Closed;
      } else {
          curState = Opened;
      }
      break;
  }
}
