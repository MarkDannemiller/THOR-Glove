#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>

/*
  FSR 0 = bottom
  FSR 1 = top
*/

// calibration constants
#define VCC 3.3         // 3.3V Logic
#define PCA_NUM_PORTS 16
#define SERVO_MAX_ANGLE 270
#define SERVO_MIN_ANGLE 0
#define SERVO_MIN_PULSE 500
#define SERVO_MAX_PULSE 2500
#define FSR_NUM_PORTS 2
#define SERVO_NUM_PORTS 1

uint16_t adc_read;  // reading from ADC pin
float adc_voltage;  // adc_read converted to voltage value
float servoAngle;   // angle of targeted servo
int fsr_counter;
int servo_counter;
Servo servoSelect;  // Analog servos run at ~60 Hz updates

//Parameters
int FSR_PINS [FSR_NUM_PORTS] = {A0,A1};
int SERVO_PINS [SERVO_NUM_PORTS] = {0};

double angleToMicroseconds(int angle) {
  return map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
}


void setup() {
  Serial.begin(115200);
  analogReadResolution(12);  // 12-bit ADC

  delay(2000);
}

void loop() {
  // calculate voltage from resistance reading
  adc_read = analogRead(FSR_PINS[fsr_counter]);
  adc_voltage = (adc_read / 4095.0) * VCC;  // 12-bit ADC (0-4095)
  servoAngle = (adc_voltage / 2.75) * SERVO_MAX_ANGLE; // Map voltage (0-3V) to servo angle (0° to upper limit)

  // cap servo angle within bounds
  if (servoAngle < SERVO_MIN_ANGLE) servoAngle = SERVO_MIN_ANGLE;
  if (servoAngle > SERVO_MAX_ANGLE) servoAngle = SERVO_MAX_ANGLE;

  // write pwm to servo
  servoSelect.attach(SERVO_PINS[servo_counter]);
  servoSelect.writeMicroseconds(angleToMicroseconds((int)servoAngle+3)); // +3 to increase tension
  servoSelect.detach();

  // increment counters
  fsr_counter += 2;
  servo_counter++;

  delay(50);
}