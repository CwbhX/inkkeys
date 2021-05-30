/*
 * E-Ink adaptive macro keyboard
 * See https://there.oughta.be/a/macro-keyboard
 * 
 * This is the main code which acts as a HID keyboard while
 * allowing to be controlled through a serial interface.
 */

#include "settings.h"           //Customize your settings in settings.h!

#include "eventsequence.h"      //Structure and constants to define the sequence of events assigned to different buttons

#include <HID-Project.h>        //HID (Keyboard/Mouse etc.)
#include <Encoder.h>            //Rotary Encoder
#include <GxEPD2_BW.h>          //E-Ink Display
#include <Adafruit_NeoPixel.h>  //Digital RGB LEDs

//Keys
const byte nSW = 11;
// const byte SW[] = {PIN_SW1, PIN_SW2, PIN_SW3, PIN_SW4, PIN_SW5, PIN_SW6, PIN_SW7, PIN_SW8, PIN_SW9}; //Array of switches for easy iteration
const byte ROWS[] = {ROW_1, ROW_2};
const byte ROW_COUNT = sizeof(ROWS)/sizeof(ROWS[0]);
const byte COLS[] = {COL_1, COL_2, COL_3, COL_4, COL_5};
const byte COL_COUNT = sizeof(COLS)/sizeof(COLS[0]);

bool pressed[] = {false, false, false, false, false, false, false, false, false}; //Last state of each key to track changes
uint32_t swDebounce[] = {0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L};

//Rotary encoder
Encoder rotary(PIN_ROTA, PIN_ROTB);
long rotaryPosition = 0;  //Last position to keep track of changes

//Display
GxEPD2_290 display(/*CS=*/ PIN_CS, /*DC=*/ PIN_DC, /*RST=*/ PIN_RST, /*BUSY=*/ PIN_BUSY);

void initDisplay() {
  display.init(0, true, 2, false);
  display.writeScreenBuffer();
  display.refresh();
  display.writeScreenBufferAgain();
  display.powerOff();
}

void setup() {
  initLEDs();
  //Initialize LEDs first, for some reason they initialize blue
  //  after a power cycle which is pretty annoying.
  
  Serial.begin(115200);

  defaultAssignment();

  //Greeting on serial
  Serial.println("= InkKeys =");
  
  //Set pin modes for keys
    for(int x = 0; x < ROW_COUNT; x++) {
        pinMode(rows[x], INPUT);
    }
    for (int x = 0; x < COL_COUNT; x++) {
      pinMode(cols[x], INPUT_PULLUP);
    }

  //Set rotary encoder to zero
  rotary.write(rotaryPosition);

  //Init e-ink screen, clear it and turn it off.
  initDisplay();

  //Init HID.
  //These three should provide most desired functions and as the 32u4
  //on the Pro Micro provides 6 endpoints (-1 for serial), we can
  //use the single report variants for compatibility.
  BootKeyboard.begin();
  BootMouse.begin();
  SingleConsumer.begin();

  //Show LED greeting to confirm completion
  ledGreeting(800);
  Serial.println("Ready.");
}

void loop() {
  checkKeysAndReportChanges();
  checkRotaryEncoderAndReportChanges();
  handleSerialInput();
}

//Called when state of key has changed. Checks debounce time
//and returns false if event should be filtered. Otherwise it
//returns true and resets the debounce timer.
bool debounce(byte i) {
  long t = millis();
  if (t - swDebounce[i] < DEBOUNCE_TIME)
    return false;
  else {
    swDebounce[i] = t;
    return true;
  }
}

//Check if keys have changed and report any changes
void checkKeysAndReportChanges() {
  for (int i = 0; i < nSW; i++) {
    int state = digitalRead(SW[i]);
    if (state == LOW && !pressed[i]) {
      if (debounce(i)) {
        pressed[i] = true;
        Serial.print(i+1);
        Serial.println("p");
        executeEvents(assignments[i][0]);
      }
    } else if (state == HIGH && pressed[i]) {
      if (debounce(i)) {
        pressed[i] = false;
        Serial.print(i+1);
        Serial.println("r");
        executeEvents(assignments[i][1]);
      }
    }
  }
}

//Check if rotary encoder position changed and report any changes
void checkRotaryEncoderAndReportChanges() {
  long rotaryNew = rotary.read();
  if (abs(rotaryNew - rotaryPosition) >= ROT_FACTOR) {
    int report = (rotaryNew-rotaryPosition)/ROT_FACTOR;
    Serial.print("R");
    Serial.println(report);
    rotaryPosition += report*ROT_FACTOR;
    for (int i = 0; i < report; i++)
      executeEvents(assignments[9][0]);
    for (int i = 0; i < -report; i++)
      executeEvents(assignments[9][1]);
  }
}

void readMatrix() {
    // iterate the columns
    for (int colIndex=0; colIndex < COL_COUNT; colIndex++) {
        // col: set to output to low
        byte curCol = COLS[colIndex];
        pinMode(curCol, OUTPUT);
        digitalWrite(curCol, LOW);
 
        // row: interate through the rows
        for (int rowIndex=0; rowIndex < ROW_COUNT; rowIndex++) {
            byte rowCol = ROWS[rowIndex];
            pinMode(rowCol, INPUT_PULLUP);
            keys[colIndex][rowIndex] = digitalRead(rowCol);
            pinMode(rowCol, INPUT);
        }
        // disable the column
        pinMode(curCol, INPUT);
    }
}

// CURRENT STATUS: 
// I am trying to add the read matrix code i, I think I need the keys array, and then we'll need to add the "ok we got the right keys pressed, now activate the right events" bit in