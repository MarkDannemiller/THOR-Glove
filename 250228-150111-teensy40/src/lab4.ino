#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include <ACS712.h>

// calibration constants
#define VCC 3.3         // 3.3V Logic
#define PCA_NUM_PORTS 16
#define SERVO_MAX_ANGLE 270
#define SERVO_MIN_ANGLE 0
#define SERVO_MIN_PULSE 500
#define SERVO_MAX_PULSE 2500
#define NUM_PORTS 2
#define MAX_CURRENT 2300 // 2.3 A to mA

// test for these values
#define FSR_THRESH 2
#define CURRENT_THRESH 3

uint16_t adc_read;    // reading from ADC pin
float adc_voltage;    // adc_read converted to voltage value
float  adc_current;  // adc_read converted to current value
float servo_angle;    // angle of targeted servo
int counter;
Servo servoSelect;  // Analog servos run at ~60 Hz updates


//Parameters
int FSR_PINS [NUM_PORTS] = {A0};
int CURRENT_PINS [NUM_PORTS] = {1};
int SERVO_PINS [NUM_PORTS] = {0};

double angleToMicroseconds(int angle) {
  return map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);  // 12-bit ADC

  counter = 0;

  delay(2000);
}

void loop() {
  // calculate voltage from resistance reading
  adc_read = analogRead(FSR_PINS[counter]);
  adc_voltage = (adc_read / 4095.0) * VCC;  // 12-bit ADC (0-4095)

  // get current reading
  adc_current = ACS712(CURRENT_PINS[counter], VCC, 4095, 100).mA_AC(); //  ACS712 20A uses 100 mV per A

  if (adc_voltage > FSR_THRESH || adc_current < CURRENT_THRESH) {
    servo_angle = (adc_voltage / 2.75) * SERVO_MAX_ANGLE; // Map voltage (0-3V) to servo angle (0° to upper limit)

    // cap servo angle within bounds
    if (servo_angle < SERVO_MIN_ANGLE) servo_angle = SERVO_MIN_ANGLE;
    if (servo_angle > SERVO_MAX_ANGLE) servo_angle = SERVO_MAX_ANGLE;

    // write pwm to servo
    servoSelect.attach(SERVO_PINS[counter]);
    servoSelect.writeMicroseconds(angleToMicroseconds((int)servo_angle+3)); // +3 to increase tension
    servoSelect.detach();
  }

  // increment counter
  counter++;

  delay(200);
}