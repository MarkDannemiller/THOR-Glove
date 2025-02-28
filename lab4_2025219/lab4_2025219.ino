#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// calibration constants
#define VCC 3.3         // 3.3V Logic
#define PCA_NUM_PORTS 16
#define SERVO1_PORT 0
#define SERVO2_PORT 1
#define SERVO_MAX_ANGLE 90
#define SERVO_MIN_ANGLE 0

uint16_t adc_read;  // reading from ADC pin
float adc_voltage;  // adc_read converted to voltage value
int fsrPin;         // targeted ADC input pin
int servoPin;       // targeted servo pin
float servoAngle;   // angle of targeted servo

//Parameters
int MIN_PULSE [PCA_NUM_PORTS] = {500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500};
int MAX_PULSE [PCA_NUM_PORTS] = {2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500};
int MIN_ANG [PCA_NUM_PORTS] =   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int MAX_ANG [PCA_NUM_PORTS] =   {270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270};

//Objects
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(0x40);

double angleToMicroseconds(int angle, int servoPort) {
  return map(angle, MIN_ANG[servoPort], MAX_ANG[servoPort], MIN_PULSE[servoPort], MAX_PULSE[servoPort]);
}


void setup() {
  Serial.begin(115200);
  analogReadResolution(12);  // 12-bit ADC

  fsrPin = A0;

  pca.begin();
  pca.setPWMFreq(60);  // Analog servos run at ~60 Hz updates

  Serial.println("Zeroing Servos...");
  pca.writeMicroseconds(SERVO1_PORT, angleToMicroseconds(0, SERVO1_PORT));
  pca.writeMicroseconds(SERVO2_PORT, angleToMicroseconds(0, SERVO2_PORT));
  delay(2000);
}

void loop() {
  // calculate voltage from resistance reading
  adc_read = analogRead(fsrPin);
  adc_voltage = (adc_read / 4095.0) * VCC;  // 12-bit ADC (0-4095)
  servoAngle = (adc_voltage / 2.75) * SERVO_MAX_ANGLE; // Map voltage (0-3V) to servo angle (0° to upper limit)

  // cap servo angle within bounds
  if (servoAngle < SERVO_MIN_ANGLE) servoAngle = SERVO_MIN_ANGLE;
  if (servoAngle > SERVO_MAX_ANGLE) servoAngle = SERVO_MAX_ANGLE;

  // write pwm to servo
  pca.writeMicroseconds(SERVO1_PORT, angleToMicroseconds((int)servoAngle+3, SERVO1_PORT));  // +3 to increase tension
  pca.writeMicroseconds(SERVO2_PORT, angleToMicroseconds((int)(SERVO_MAX_ANGLE-servoAngle), SERVO2_PORT));

  delay(100);
}