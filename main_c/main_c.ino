#include <Wire.h>

#include <GOFi2cOLED.h>

#include <EEPROM.h>

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
enum State {Initial=0, Neutral=1, Closed = 2, Opened = 3, Open = 4};


const int pushButtonPin = 2;     // the number of the pushbutton pin
const int closeButtonPin = 3;
const int relayPin1 = 12;
const int relayPin2 = 13;
// const int ledPin =  13;      // the number of the LED pin

// closeButtonState is closed at 0
// pushbuttonState is pushed at 1
volatile int pushButtonState = 0;         // variable for reading the pushbutton status
volatile int closeButtonState = 0;
volatile unsigned int count = 0;
volatile enum State curState = Initial;
GOFi2cOLED GOFoled;

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

  init_count();

  init_lcd();
  updateLCD();


  pinMode(closeButtonPin, INPUT);
  pinMode(pushButtonPin, INPUT);

  init_relay();

  // Attach an interrupt to the ISR vector
  attachInterrupt(0, pushButton_ISR, CHANGE);
  attachInterrupt(1, closeButton_ISR, CHANGE);
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
        magnetOff();
      }
      else{
        curState = Neutral;
      }
      break;

    case Closed:
      if(closeButtonState){
        curState = Opened;
        count++;
        updateLCD();
      }
      else if(!pushButtonState){
        curState = Neutral;
      }
      else {
        curState = Closed;
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
        magnetOn();
      }
      break;
  }
}

void closeButton_ISR() {
  static unsigned long last_interrupt_time1 = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time1 > 200)
  {
   closeButtonState = digitalRead(closeButtonPin);
   processStateMachine();
  }
  last_interrupt_time1 = interrupt_time;
 // Serial.print("cl\n");
}

void pushButton_ISR() {
  static unsigned long last_interrupt_time2 = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time2 > 200)
  {
    pushButtonState = digitalRead(pushButtonPin);
    processStateMachine();
  }
  last_interrupt_time2 = interrupt_time;
// Serial.print("op\n");
}
