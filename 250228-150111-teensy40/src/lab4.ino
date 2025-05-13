//lab4.ino
#include <Arduino.h>
#include "../src/finger.h"
// Teensy 4.0 is the microcontroller

#define VCC           3.3   // 3.3V Logic
#define ADC_BITS      12
#define NUM_BUTTONS   3
#define NUM_LEDS      2

// Servo Calibration
#define SERVO_DEFAULT 90    // in degrees
#define NUM_FINGERS   5
#define MAX_CURRENT_PER_SERVO   2500  // 2.5 A to mA
#define MAX_CURRENT_TOTAL       12000 // 12 A to mA
#define INCREMENT     2

#define UPPER_LIMIT   190
#define LOWER_LIMIT   5

#define UPPER_LIMIT_PINKY 250
#define LOWER_LIMIT_PINKY 15
#define UPPER_LIMIT_RING 190
#define LOWER_LIMIT_RING 5
#define UPPER_LIMIT_MIDDLE 190
#define LOWER_LIMIT_MIDDLE 5
#define UPPER_LIMIT_THUMB 250
#define LOWER_LIMIT_THUMB 5
#define UPPER_LIMIT_INDEX 250
#define LOWER_LIMIT_INDEX 5

// reusable finger variables
float fsr_voltage;  // fsr voltage value
float current;      // current sensor value
int counter;

// pin numbers
int BUTTON_PINS [NUM_BUTTONS] = {8,9,10}; // max, min, start
int LED_PINS [NUM_LEDS] = {12,13};

// angle, pulse
int SERVO_MIN [2] = {0, 500};
int SERVO_MAX [2] = {270, 2500};

#define PINKY 0
#define RING 1
#define MIDDLE 2
#define INDEX 3
#define THUMB 4

// Add these constants at the top with other defines
#define MOTION_STEPS 50    // Number of steps for smooth motion
#define STEP_DELAY 20     // Delay between steps in milliseconds

// System state variables
enum SystemMode {
  ACTIVE_ASSIST,
  RELAX,
  FAULT
};

SystemMode currentMode = ACTIVE_ASSIST;
float holdCurrent = 1.5;  // Initial hold current in Amperes
float neutralFSR[NUM_FINGERS];  // Neutral FSR values for each finger
unsigned long lastFaultTime = 0;
const unsigned long FAULT_TIMEOUT = 150;  // ms

// Control parameters
const float K_FSR = 2000.0;  // deg/s/V
const float K_I = 100.0;    // deg/s/A
const float FSR_DEADBAND = 0.2;  // V, FSR above this value is considered user intention to open/extend finger



// finger list
Finger FINGERS[NUM_FINGERS] = {
  // servoPin, fsrPin, currentPin, VCC, num ADC bits, 
  // servo min angle, servo max angle, servo min pulse, servo max pulse, invert
  Finger(3, 14, 17, VCC, ADC_BITS, SERVO_MIN[0], SERVO_MAX[0], 
    SERVO_MIN[1], SERVO_MAX[1], LOWER_LIMIT_PINKY, UPPER_LIMIT_PINKY, false), // pinky
  Finger(5, 16, 21, VCC, ADC_BITS, SERVO_MIN[0], SERVO_MAX[0],
    SERVO_MIN[1], SERVO_MAX[1], LOWER_LIMIT_RING, UPPER_LIMIT_RING, true), // ring
  Finger(2, 18, 15, VCC, ADC_BITS, SERVO_MIN[0], SERVO_MAX[0], 
    SERVO_MIN[1], SERVO_MAX[1], LOWER_LIMIT_MIDDLE, UPPER_LIMIT_MIDDLE, false), // middle
  Finger(7, 20, 23, VCC, ADC_BITS, SERVO_MIN[0], SERVO_MAX[0], 
    SERVO_MIN[1], SERVO_MAX[1], LOWER_LIMIT_INDEX, UPPER_LIMIT_INDEX, true), // index
  Finger(4, 22, 19, VCC, ADC_BITS, SERVO_MIN[0], SERVO_MAX[0], 
    SERVO_MIN[1], SERVO_MAX[1], LOWER_LIMIT_THUMB, UPPER_LIMIT_THUMB, false)  // thumb
};

void calibrateNeutralFSR() {
  Serial.println("Calibrating neutral FSR values...");
  for (int i = 0; i < NUM_FINGERS; i++) {
    float sum = 0;
    for (int j = 0; j < 200; j++) {  // 2 seconds at 100Hz
      sum += FINGERS[i].getFSRVoltage();
      delay(10);
    }
    neutralFSR[i] = sum / 200;
    Serial.print("Finger ");
    Serial.print(i);
    Serial.print(" neutral FSR: ");
    Serial.println(neutralFSR[i]);
  }
}

void updateSystemMode() {
  // Check for relax button press
  if (digitalRead(BUTTON_PINS[2])) {  // RELAX button
    if (currentMode == ACTIVE_ASSIST) {
      currentMode = RELAX;
    } else {
      currentMode = ACTIVE_ASSIST;
    }
    delay(200);  // Debounce
  }

  // Update LED based on mode
  digitalWrite(LED_PINS[1], (currentMode != ACTIVE_ASSIST));
}

void updateHoldCurrent() {
  if (digitalRead(BUTTON_PINS[0])) {  // GRIP -
    holdCurrent = max(0.3, holdCurrent - 0.1);
    delay(200);  // Debounce
  }
  if (digitalRead(BUTTON_PINS[1])) {  // GRIP +
    holdCurrent = min(1.5, holdCurrent + 0.1);
    delay(200);  // Debounce
  }
}

float velocity[NUM_FINGERS] = {0};

void controlFinger(int fingerIndex) {
  if (currentMode == FAULT) {
    FINGERS[fingerIndex].release();
    return;
  }
  if (currentMode == RELAX) {
    FINGERS[fingerIndex].setAngle(FINGERS[fingerIndex].lowerLimit);
    return;
  }

  // Get sensor readings
  float fsrVoltage = FINGERS[fingerIndex].getFSRVoltage();
  float current = FINGERS[fingerIndex].getCurrentValue();
  float currentAngle = FINGERS[fingerIndex].getAngle();

  // Check for overcurrent fault
  if (current > MAX_CURRENT_PER_SERVO) {
    if (millis() - lastFaultTime > FAULT_TIMEOUT) {
      currentMode = FAULT;
      return;
    }
  } else {
    lastFaultTime = millis();
  }

  // Calculate FSR-based velocity
  float deltaFSR = fsrVoltage - neutralFSR[fingerIndex];

  // User intention to open/extend finger (is fighting the servo tendon pull)
  if (deltaFSR > FSR_DEADBAND) {
    velocity[fingerIndex] = -K_FSR * (deltaFSR - FSR_DEADBAND); // negative velocity to extend finger
  }
  // User intention to flex finger (is not fighting the servo tendon pull)
  else if (deltaFSR < FSR_DEADBAND) {
    velocity[fingerIndex] = K_FSR * (FSR_DEADBAND - fsrVoltage); // positive velocity to flex finger
  }

  // Current-based override
  if (current > holdCurrent) {
    velocity[fingerIndex] = -K_I * (current - holdCurrent);
  }

  // Update position
  float newAngle = currentAngle + (velocity[fingerIndex] * 0.01);  // 10ms period
  FINGERS[fingerIndex].setAngle(newAngle);
}

/* ------------------------------------------------------------------------
 * Pretty printer for the THOR-glove debug table
 * --------------------------------------------------------------------- */
constexpr uint8_t CELL_W = 10;           // characters between the “|” pipes

/* helper: print one cell and right-pad it to CELL_W */
void printCell(const char *txt)
{
  Serial.print("| ");
  size_t len = strlen(txt);
  Serial.print(txt);
  /* pad on the right so that “| txt…… ” is CELL_W long           */
  for (size_t i = len + 1; i < CELL_W; ++i) Serial.print(' ');
}

/* helper: print the +----------+ separator row */
void printRow(char fill)
{
  for (uint8_t c = 0; c <= NUM_FINGERS; ++c) {   // 5 fingers + “Value” col
    Serial.print('+');
    for (uint8_t i = 0; i < CELL_W; ++i) Serial.print(fill);
  }
  Serial.println('+');
}

/* ------------------------------ call this inside loop() -------------- */
void printDebug()
{
  /* clear terminal & move cursor to top-left */
  Serial.print("\x1B[2J");   // ESC[2J  → clear entire screen
  Serial.print("\x1B[H");    // ESC[H   → cursor to home (row 1, col 1)

  /* header ------------------------------------------------------------ */
  Serial.println(F("\n=== THOR Glove Status ==="));
  Serial.print  (F("Mode: "));
  switch (currentMode) {
    case ACTIVE_ASSIST: Serial.print(F("ACTIVE")); break;
    case RELAX:         Serial.print(F("RELAX"));  break;
    case FAULT:         Serial.print(F("FAULT"));  break;
  }
  Serial.print  (F(" | Hold Current: "));
  Serial.print  (holdCurrent, 2);
  Serial.println(F(" A"));

  /* top border + title row ------------------------------------------- */
  printRow('-');
  printCell("Value");   printCell("Pinky"); printCell("Ring");
  printCell("Middle");  printCell("Index"); printCell("Thumb");
  Serial.println('|');
  printRow('-');

  /* FSR row ----------------------------------------------------------- */
  char buf[12];
  float totalFSR = 0;
  printCell("FSR[V]");
  for (int i = 0; i < NUM_FINGERS; ++i) {
    totalFSR += FINGERS[i].getFSRVoltage();
    snprintf(buf, sizeof(buf), "%6.3fV", FINGERS[i].getFSRVoltage());
    printCell(buf);
  }
  Serial.println('|');

  /* Current row ------------------------------------------------------- */
  float totalCur = 0;
  printCell("Curr[A]");
  for (int i = 0; i < NUM_FINGERS; ++i) {
    totalCur += FINGERS[i].getCurrentValue();
    snprintf(buf, sizeof(buf), "%6.2fA", FINGERS[i].getCurrentValue());
    printCell(buf);
  }
  Serial.println('|');

  /* Angle row --------------------------------------------------------- */
  float totalAng = 0;
  printCell("Angle°");
  for (int i = 0; i < NUM_FINGERS; ++i) {
    totalAng += FINGERS[i].getAngle();
    snprintf(buf, sizeof(buf), "%6.1fd", FINGERS[i].getAngle());
    printCell(buf);
  }
  Serial.println('|');

  /* velocity row --------------------------------------------------------- */
  printCell("Velocity");
  for (int i = 0; i < NUM_FINGERS; ++i) {
    snprintf(buf, sizeof(buf), "%6.1fd", velocity[i]);
    printCell(buf);
  }
  Serial.println('|');

  /* totals row -------------------------------------------------------- */
  printRow('=');
  printCell("Total");
  snprintf(buf, sizeof(buf), "%6.3fV", totalFSR);  printCell(buf);
  snprintf(buf, sizeof(buf), "%6.2fA", totalCur);  printCell(buf);
  snprintf(buf, sizeof(buf), "%6.1fd", totalAng);  printCell(buf);
  /* two blank cells so table width stays constant */
  printCell(""); printCell("");
  Serial.println('|');
  printRow('-');
}


/* ------------------------------------------------------------------------
 * Main
 * --------------------------------------------------------------------- */

void setup() {
  Serial.begin(9600);
  analogReadResolution(12);  // 12-bit ADC
  counter = 0;

  // setup button pins
  for (int i = 8; i <= 10; i++) { pinMode(i,INPUT_PULLDOWN); }

  // Initialize each finger
  for (int i = 0; i < NUM_FINGERS; i++) { 
    FINGERS[i].begin(); 
  }

  // set LED status
  pinMode(LED_PINS[1], OUTPUT);
  digitalWrite(LED_PINS[1], HIGH);

  delay(5000);

  //  FINGERS[RING].setAngle(250);

  // Calibrate neutral FSR values
  calibrateNeutralFSR();

  // Initial system state
  currentMode = ACTIVE_ASSIST;
}

void loop() {
  static unsigned long lastControlTime = 0;
  unsigned long currentTime = millis();

  // Maintain 100Hz control loop
  if (currentTime - lastControlTime >= 10) {  // 10ms = 100Hz
    lastControlTime = currentTime;

    // Update system mode and hold current
    updateSystemMode();
    updateHoldCurrent();

    // Control each finger
    for (int i = 0; i < NUM_FINGERS; i++) {
      controlFinger(i);
    }

    // Check for overcurrent fault
    float totalCurrent = 0;
    for (int i = 0; i < NUM_FINGERS; i++) {
      totalCurrent += FINGERS[i].getCurrentValue();
    }
    if (totalCurrent > MAX_CURRENT_TOTAL) {
      Serial.println("Overcurrent fault detected (total current: " + String(totalCurrent) + " mA)");
      currentMode = FAULT;
    }

    printDebug();  // Debugs the device status
  }
}