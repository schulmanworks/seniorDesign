#include <Wire.h>

#include <GOFi2cOLED.h>

#include <EEPROM.h>

// #include <PinChangeInt.h>

// #include <Serial.h>

#define COUNT_ADDR 0
#define EEPROM_CANARY 10
#define OPEN = 1
#define CLOSE = 0

void init_lcd(void);
void init_relay(void);
void magnetOn(void);
void  magnetOff(void);
void updateLCD(void);
bool tryLock(char *m);
enum State {Initial=0, Neutral=1, Closed = 2, Opened = 3, Open = 4};


const int pushButtonPin = 2;     // the number of the pushbutton pin
const int closeButtonPin = 3;
const int relayPin1 = 12;
const int relayPin2 = 13;
int processStateLock = 0;
// const int ledPin =  13;      // the number of the LED pin

// closeButtonState is closed at 0
// pushbuttonState is pushed at 1
volatile int pushButtonState = 0;         // variable for reading the pushbutton status
volatile int closeButtonState = 1;
volatile unsigned int count = 0;
volatile enum State curState = Initial;
GOFi2cOLED GOFoled;
bool tryLock(int *m) {
  if (*m) {
    return false;
  }
  else {
    *m = 1;
    return true;
  }
}
bool unlock(int *m) {
  *m = 0;
  return true;
}
void init_lcd(void) {
  GOFoled.init(0x3C);  //initialze  OLED display

  GOFoled.display(); // show splashscreen
  delay(2000);
  GOFoled.clearDisplay();
  GOFoled.setCursor(0,0);
  GOFoled.setTextSize(2);
  GOFoled.setTextColor(WHITE);

  // GOFoled.setTextSize(1);
  // GOFoled.setTextColor(WHITE);
  // GOFoled.setCursor(0,0);
  // GOFoled.println("Hello, world!");
  // GOFoled.println(-1234);
  // GOFoled.println(3.14159);
  // GOFoled.setTextColor(BLACK, WHITE); // 'inverted' text
  // GOFoled.println(3.14159,5);
  // GOFoled.setTextSize(2);
  // GOFoled.setTextColor(WHITE);
  // GOFoled.print("0x"); GOFoled.println(0xDEADBEEF, HEX);
  // GOFoled.display();
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

void init_count(void) {
  uint8_t value;
  uint8_t canary;
  unsigned int tmp_count;

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

void setup() {
  // initialize the LED pin as an output:
  // pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  Serial.begin(9600);

  // init_count();
  count = 0;

  init_lcd();
  updateLCD();


  pinMode(closeButtonPin, INPUT);
  pinMode(pushButtonPin, INPUT);

  init_relay();

  // Attach an interrupt to the ISR vector
  // attachInterrupt(0, pushButton_ISR, CHANGE);
  // attachInterrupt(1, closeButton_ISR, CHANGE);
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
    // closeButtonState = digitalRead(closeButtonPin);
    // pushButtonState = digitalRead(pushButtonPin);
    // processStateMachine();
  // Serial.print("push: ");
  // Serial.print(pushButtonState);
  // Serial.print(" door: ");
  // Serial.print(closeButtonState);
  // Serial.print(" state:");
  // Serial.print(curState);
  // Serial.print("\n");
  closeButtonState = digitalRead(closeButtonPin);
  pushButtonState = digitalRead(pushButtonPin);
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
void processStateMachine() {
  // while (tryLock(&processStateLock)) ;
  // Serial.print("push: ");
  // Serial.print(pushButtonState);
  // Serial.print("door: ");
  // Serial.print(closeButtonState);
  // Serial.print("\n");
  switch (curState) {
    case Initial:
      //load data from EEPROM
      if(!pushButtonState && !closeButtonState){
        curState = Neutral;
        // printLCDStr("neutral");
      }
      else
      {
        curState = Initial;
        // printLCDStr("initial");
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
          // printLCDStr("closed");
      }
      else{
        curState = Neutral;
          // printLCDStr("neutral");
      }
      break;

    case Closed:
      if (closeButtonState) {
        curState = Opened;
        // printLCDStr("opened");
      }
      else if(!pushButtonState){
        curState = Neutral;
        magnetOn();
          // printLCDStr("neutral");
      }
      else {
        curState = Closed;
          // printLCDStr("closed");
      }
      break;

    case Opened:
      //Magnet
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
  // unlock(&processStateLock);
}

void closeButton_ISR() {
  //Serial.print("In close ISR\n");
  static unsigned long last_interrupt_time1 = 0;
  unsigned long interrupt_time1 = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time1 - last_interrupt_time1 > 200)
  {
   closeButtonState = digitalRead(closeButtonPin);
  //  processStateMachine();
  }
  last_interrupt_time1 = interrupt_time1;
 // Serial.print("cl\n");
}

void pushButton_ISR() {
  //Serial.print("In button ISR\n");
  static unsigned long last_interrupt_time2 = 0;
  unsigned long interrupt_time2 = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time2 - last_interrupt_time2 > 200)
  {
    pushButtonState = digitalRead(pushButtonPin);
    // processStateMachine();
  }
  last_interrupt_time2 = interrupt_time2;
// Serial.print("op\n");
}
