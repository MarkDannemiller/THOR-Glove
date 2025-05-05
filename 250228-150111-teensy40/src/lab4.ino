#include <Arduino.h>
#include "../src/finger.h"
// Teensy 4.0 is the microcontroller

// calibration constants
#define VCC           3.3   // 3.3V Logic
#define ADC_BITS      12
#define SERVO_DEFAULT 90    // in degrees
#define NUM_FINGERS   5
#define NUM_BUTTONS   3
#define NUM_LEDS      2
#define MAX_CURRENT   2300  // 2.3 A to mA
#define INCREMENT     2

#define UPPER_LIMIT   190
#define LOWER_LIMIT   15

#define UPPER_LIMIT_PINKY 250
#define LOWER_LIMIT_PINKY 15
#define UPPER_LIMIT_RING 190
#define LOWER_LIMIT_RING 15
#define UPPER_LIMIT_MIDDLE 190
#define LOWER_LIMIT_MIDDLE 15
#define UPPER_LIMIT_THUMB 250
#define LOWER_LIMIT_THUMB 15
#define UPPER_LIMIT_INDEX 250
#define LOWER_LIMIT_INDEX 15

// test for these values
#define FSR_THRESH_HIGH 2
#define FSR_THRESH_LOW  1
#define CURRENT_THRESH  3

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

// finger list
Finger FINGERS[NUM_FINGERS] = {
  // servoPin, fsrPin, currentPin, VCC, num ADC bits, 
  // servo min angle, servo max angle, servo min pulse, servo max pulse, invert
  Finger(3, 14, 15, VCC, ADC_BITS, SERVO_MIN[0], SERVO_MAX[0], 
    SERVO_MIN[1], SERVO_MAX[1], LOWER_LIMIT_PINKY, UPPER_LIMIT_PINKY, false), // pinky
  Finger(5, 16, 17, VCC, ADC_BITS, SERVO_MIN[0], SERVO_MAX[0],
    SERVO_MIN[1], SERVO_MAX[1], LOWER_LIMIT_RING, UPPER_LIMIT_RING, true), // ring
  Finger(2, 18, 19, VCC, ADC_BITS, SERVO_MIN[0], SERVO_MAX[0], 
    SERVO_MIN[1], SERVO_MAX[1], LOWER_LIMIT_MIDDLE, UPPER_LIMIT_MIDDLE, false), // middle
  Finger(7, 20, 21, VCC, ADC_BITS, SERVO_MIN[0], SERVO_MAX[0], 
    SERVO_MIN[1], SERVO_MAX[1], LOWER_LIMIT_INDEX, UPPER_LIMIT_INDEX, true), // index
  Finger(4, 22, 23, VCC, ADC_BITS, SERVO_MIN[0], SERVO_MAX[0], 
    SERVO_MIN[1], SERVO_MAX[1], LOWER_LIMIT_THUMB, UPPER_LIMIT_THUMB, false)  // thumb
};

// Set fingers to virtual 0 point, not physical servo 0
void zeroFingers() {
  for (int i = 0; i < NUM_FINGERS; i++) { FINGERS[i].setAngle(0); }
}

void setAngleAll(int angle) {
  for (int i = 0; i < NUM_FINGERS; i++) { FINGERS[i].setAngle(angle); }
}

// Add this function before setup()
void smoothMotion(int fingerIndex, int startAngle, int endAngle) {
  float angleStep = (float)(endAngle - startAngle) / MOTION_STEPS;
  
  for(int step = 0; step <= MOTION_STEPS; step++) {
    int currentAngle = startAngle + (angleStep * step);
    FINGERS[fingerIndex].setAngle(currentAngle);
    delay(STEP_DELAY);
  }
}

void setup() {
  Serial.begin(9600);
  analogReadResolution(12);  // 12-bit ADC
  counter = 0;

  // setup button pins
  for (int i = 8; i <= 10; i++) { pinMode(i,INPUT_PULLDOWN); }

  // initialize each finger
  for (int i = 0; i < NUM_FINGERS; i++) { FINGERS[i].begin(); }

  // set LED status
  pinMode(LED_PINS[1], OUTPUT);
  digitalWrite(LED_PINS[1], HIGH);

  zeroFingers();

  delay(5000);

   FINGERS[RING].setAngle(250);

  // for(int i=0; i<10; i++) {
  //   for(int j=0; j<NUM_FINGERS; j++) {
  //     FINGERS[j].setAngle(FINGERS[j].upperLimit);
  //   } 
  //   delay(2000);
  //   for(int j=0; j<NUM_FINGERS; j++) {
  //     FINGERS[j].setAngle(FINGERS[j].lowerLimit);
  //   }
  //   delay(4000);
  // }

  // for(int i=0; i<10; i++) {
  //   FINGERS[PINKY].setTrueAngle(180);
  //   delay(2000);
  //   FINGERS[PINKY].setTrueAngle(0);
  //   delay(2000);
  //   }

  // for(int i=0; i<10; i++) {
  //   // zeroFingers();
  //   FINGERS[PINKY].setTrueAngle(0);
  //   FINGERS[RING].setTrueAngle(0);
  //   FINGERS[MIDDLE].setTrueAngle(0);
  //   FINGERS[INDEX].setTrueAngle(0);
  //   FINGERS[THUMB].setTrueAngle(0);
  //   //setAngleAll(0); 
  //   delay(3000);
  //   FINGERS[PINKY].setTrueAngle(UPPER_LIMIT);
  //   FINGERS[RING].setTrueAngle(UPPER_LIMIT);
  //   FINGERS[MIDDLE].setTrueAngle(UPPER_LIMIT);
  //   FINGERS[INDEX].setTrueAngle(UPPER_LIMIT);
  //   FINGERS[THUMB].setTrueAngle(UPPER_LIMIT);
  //   //setAngleAll(30);
  //   delay(3000);
  //   FINGERS[PINKY].setTrueAngle(0);
  //   FINGERS[RING].setTrueAngle(0);
  //   FINGERS[MIDDLE].setTrueAngle(0);
  //   FINGERS[INDEX].setTrueAngle(0);
  //   FINGERS[THUMB].setTrueAngle(0);
  //   //setAngleAll(0);
  //   delay(3000);
  // }


  // // Test smooth motion for finger
  // for(int i = 0; i < 100; i++) {
  //   smoothMotion(INDEX, 0, 250);    // Move from 0 to 180 degrees
  //   delay(2000);                     // Pause at top
  //   smoothMotion(INDEX, 250, 0);    // Move from 180 to 0 degrees
  //   delay(2000);                     // Pause at bottom
  // }
}

void loop() {

//   // get voltage and current readings
//   fsr_voltage = FINGERS[counter].getFSRVoltage();
//   current = FINGERS[counter].getCurrentValue();

//   // test data
//   Serial.print(counter);
//   Serial.print("\tfsr voltage: ");
//   Serial.print(fsr_voltage);
//   Serial.print("\tcurrent: ");
//   Serial.print(current);

//   // software overcurrent 
//   if(current > CURRENT_THRESH) { 
//     FINGERS[counter].release();
//     return;
//   }

// /*

//   // button settings
//   } else if(digitalRead(BUTTON_PINS[0])) { FINGERS[counter].setAngle(SERVO_MAX[0]);
//   } else if(digitalRead(BUTTON_PINS[1])) { FINGERS[counter].setAngle(SERVO_MIN[0]);
//   } else if(digitalRead(BUTTON_PINS[2])) { FINGERS[counter].setAngle(SERVO_DEFAULT);

//   // general control logic
//   // no need for angle limits because setAngle has them built in
//   } else if (fsr_voltage > FSR_THRESH_HIGH) { FINGERS[counter].setAngle(FINGERS[counter].getAngle()+INCREMENT); 
//   } else if (fsr_voltage < FSR_THRESH_LOW) { FINGERS[counter].setAngle(FINGERS[counter].getAngle()-INCREMENT); }
//   */
//   // test angle
//   FINGERS[3].setAngle(20*(counter%2));

//   // increment counter
//   counter = (counter + 1) % NUM_FINGERS;

//   delay(200);
}