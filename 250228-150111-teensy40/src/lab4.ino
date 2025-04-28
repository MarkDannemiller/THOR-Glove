#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include <ACS712.h>

// calibration constants
#define VCC 3.3         // 3.3V Logic
#define SERVO_MAX_ANGLE 270
#define SERVO_MIN_ANGLE 0
#define SERVO_MIN_PULSE 500
#define SERVO_MAX_PULSE 2500
#define SERVO_DEFAULT   90
#define NUM_FINGERS 5
#define NUM_BUTTONS 3
#define NUM_LEDS    2
#define MAX_CURRENT 2300 // 2.3 A to mA

// test for these values
#define FSR_THRESH 2
#define CURRENT_THRESH 3

uint16_t adc_read;              // reading from ADC pin
float adc_voltage;              // adc_read converted to voltage value
float  adc_current;             // adc_read converted to current value
float servo_angle;              // angle of targeted servo
Servo servoList [NUM_FINGERS];  // analog servos run at ~60 Hz updates
int counter;


// Pin Assignments

// pinky, ring, middle, index, thumb
int FSR_PINS [NUM_FINGERS] =      {14,16,18,20,22};
int CURRENT_PINS [NUM_FINGERS] =  {15,17,19,21,23};
int SERVO_PINS [NUM_FINGERS] =    {2, 3, 4, 5, 7};

int BUTTON_PINS [NUM_BUTTONS] = {8,9,10}; // max, min, start
int LED_PINS [NUM_LEDS] = {12,13};

// Map servo angle to pulse width
double angleToMicroseconds(int angle) {
  return map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
}

// Write pwm to servo
void writeToServo(Servo servo, float angle) {
  servo.writeMicroseconds(angleToMicroseconds((int)angle+3)); // +3 to increase tension
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);  // 12-bit ADC
  counter = 0;

  // setup pins and servos
  for (int i = 8; i <= 10; i++) { pinMode(i,INPUT_PULLDOWN); }
  for (int i = 0; i < NUM_FINGERS; i++) { servoList[i].attach(SERVO_PINS[i]); }

  // set LED status
  pinMode(LED_PINS[2], OUTPUT);
  digitalWrite(LED_PINS[2], HIGH);

  delay(2000);
}

void loop() {

  // calculate voltage from resistance reading
  adc_read = analogRead(FSR_PINS[counter]);
  adc_voltage = (adc_read / 4095.0) * VCC;  // 12-bit ADC (0-4095)

  // get current reading
  adc_current = ACS712(CURRENT_PINS[counter], VCC, 4095, 100).mA_AC(); //  ACS712 20A uses 100 mV per A

  // button settings
  if(digitalRead(BUTTON_PINS[0])) {        writeToServo(servoList[counter], SERVO_MAX_ANGLE);
  } else if(digitalRead(BUTTON_PINS[1])) { writeToServo(servoList[counter], SERVO_MIN_ANGLE);
  } else if(digitalRead(BUTTON_PINS[2])) { writeToServo(servoList[counter], SERVO_DEFAULT);

  // general control logic
  } else if (adc_voltage > FSR_THRESH && adc_current < CURRENT_THRESH) {

    // Map voltage (0-3V) to servo angle (0° to upper limit)
    servo_angle = (adc_voltage / 2.75) * SERVO_MAX_ANGLE; 

    // cap servo angle within bounds
    if (servo_angle < SERVO_MIN_ANGLE) servo_angle = SERVO_MIN_ANGLE;
    if (servo_angle > SERVO_MAX_ANGLE) servo_angle = SERVO_MAX_ANGLE;

    // write pwm to servo
    writeToServo(servoList[counter], servo_angle);
  }

  writeToServo(servoList[counter], 180);

  // increment counter
  counter = (counter+1)%NUM_FINGERS;

  delay(200);
}