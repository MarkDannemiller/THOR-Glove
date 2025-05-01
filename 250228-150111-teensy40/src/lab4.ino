#include <Arduino.h>
#include "../src/finger.h"

// calibration constants
#define VCC 3.3         // 3.3V Logic
#define ADC_BITS 12
#define SERVO_DEFAULT   90 // needs adjusting because of flipped servos
#define NUM_FINGERS 5
#define NUM_BUTTONS 3
#define NUM_LEDS    2
#define MAX_CURRENT 2300 // 2.3 A to mA

// test for these values
#define FSR_THRESH 2
#define CURRENT_THRESH 3

float fsr_voltage;              // adc_read converted to voltage value
float current;                  // adc_read converted to current value
int counter;

int BUTTON_PINS [NUM_BUTTONS] = {8,9,10}; // max, min, start
int LED_PINS [NUM_LEDS] = {12,13};

// angle, pulse
int SERVO_MIN [2] = {0, 500};
int SERVO_MAX [2] = {270, 2500};

// finger definitions
Finger PINKY = Finger(2, 14, 15, VCC, ADC_BITS, 
  SERVO_MIN[0], SERVO_MAX[0], SERVO_MIN[1], SERVO_MAX[1]);

Finger RING = Finger(3, 16, 17, VCC, ADC_BITS, 
  SERVO_MIN[0], SERVO_MAX[0], SERVO_MIN[1], SERVO_MAX[1]);

Finger MIDDLE = Finger(4, 18, 19, VCC, ADC_BITS, 
  SERVO_MIN[0], SERVO_MAX[0], SERVO_MIN[1], SERVO_MAX[1]);

Finger INDEX = Finger(5, 20, 21, VCC, ADC_BITS, 
  SERVO_MIN[0], SERVO_MAX[0], SERVO_MIN[1], SERVO_MAX[1]);

Finger THUMB = Finger(7, 22, 23, VCC, ADC_BITS, 
  SERVO_MIN[0], SERVO_MAX[0], SERVO_MIN[1], SERVO_MAX[1]);

// finger list
Finger fingers[NUM_FINGERS] = {PINKY, RING, MIDDLE, INDEX, THUMB};


void setup() {
  Serial.begin(115200);
  analogReadResolution(12);  // 12-bit ADC
  counter = 0;

  // setup button pins
  for (int i = 8; i <= 10; i++) { pinMode(i,INPUT_PULLDOWN); }

  // set LED status
  pinMode(LED_PINS[2], OUTPUT);
  digitalWrite(LED_PINS[2], HIGH);

  delay(2000);
}

void loop() {

  // get voltage and current readings
  fsr_voltage = fingers[counter].getFSRVoltage();
  current = fingers[counter].getCurrentValue();

  // button settings
  if(digitalRead(BUTTON_PINS[0])) {        fingers[counter].setAngle(SERVO_MAX[0]);
  } else if(digitalRead(BUTTON_PINS[1])) { fingers[counter].setAngle(SERVO_MIN[0]);
  } else if(digitalRead(BUTTON_PINS[2])) { fingers[counter].setAngle(SERVO_DEFAULT);

  // general control logic
  } else if (fsr_voltage > FSR_THRESH && current < CURRENT_THRESH) {
    fingers[counter].setAngle(fingers[counter].calcAngle());
  }

  // test angle
  fingers[counter].setAngle(180);

  // increment counter
  counter = (counter+1)%NUM_FINGERS;

  delay(200);
}