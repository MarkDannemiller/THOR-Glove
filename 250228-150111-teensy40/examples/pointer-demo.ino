#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>

// Control constants
#define VCC 3.3         // 3.3V Logic
#define SERVO_MAX_ANGLE 130
#define SERVO_MIN_ANGLE 0
#define SERVO_MIN_PULSE 500
#define SERVO_MAX_PULSE 2500
#define MAX_CURRENT 2300 // 2.3 A to mA
#define NUM_PORTS 5  // Number of finger ports
#define LED_PIN 13   // Built-in LED pin

// test for these values
#define FSR_THRESH 2
#define CURRENT_THRESH 3

uint16_t adc_read;    // reading from ADC pin
float fsr_voltage;    // adc_read converted to voltage value
float  adc_current;  // adc_read converted to current value
float servo_angle;    // angle of targeted servo
int counter;
Servo servoSelect;  // Analog servos run at ~60 Hz updates

uint32_t timer;


//Parameters
int FSR_PINS [NUM_PORTS] = {A0, A1, A2, A3, A4};
int CURRENT_PINS [NUM_PORTS] = {A5, A6, A7, A8, A9};
int SERVO_PINS [NUM_PORTS] = {3, 5, 2, 7, 4};
Servo actuators [NUM_PORTS] = {Servo()};

double angleToMicroseconds(int angle) {
  return map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
}

void setAngle(Servo servo, int angle) {
  servo.writeMicroseconds(angleToMicroseconds((int)angle));
  Serial.println("Servo " + String(servo.attached()) + " angle: " + String(angle));
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);  // 12-bit ADC
  pinMode(LED_PIN, OUTPUT);  // Set LED pin as output
  digitalWrite(LED_PIN, HIGH);  // Turn on the LED

  counter = 0;

  for (int i = 0; i < NUM_PORTS; i++) {
    actuators[i].attach(SERVO_PINS[i]);
  }

  

  // delay(5000);

  // // Test index finger servo
  // setAngle(actuators[0], 0);
  // delay(5000);
  // setAngle(actuators[0], 90);
  // delay(5000);
  // setAngle(actuators[0], 0);
  // delay(10000);

  timer = millis();
}

void loop() {
  // calculate FSR voltage from resistance reading
  adc_read = analogRead(FSR_PINS[counter]);
  fsr_voltage = (adc_read / 4095.0) * VCC;  // 12-bit ADC (0-4095)

  Serial.println("FSR Voltage: " + String(fsr_voltage));

  // get current reading
  // A=20, R_sns = 50mOhms, VCC=3.3v
  adc_read = analogRead(CURRENT_PINS[counter]);
  adc_current = ( adc_read / 4095.0) * VCC / (20 * 0.05); // I = V/R = (VCC/20)/(50mOhms)

  Serial.println("Servo Current: " + String(adc_current) + " raw: " + String(adc_read));

  // if (fsr_voltage > FSR_THRESH || adc_current < CURRENT_THRESH) {
  //   servo_angle = (fsr_voltage / 2.75) * SERVO_MAX_ANGLE; // Map voltage (0-3V) to servo angle (0° to upper limit)

  //   // cap servo angle within bounds
  //   if (servo_angle < SERVO_MIN_ANGLE) servo_angle = SERVO_MIN_ANGLE;
  //   if (servo_angle > SERVO_MAX_ANGLE) servo_angle = SERVO_MAX_ANGLE;

  //   setAngle(actuators[0], servo_angle);
  // }

  Serial.println("Timer: " + String(millis() - timer));

  for(int i=0; i<NUM_PORTS; i++) {
    // Swtich from 30->0 every 2.5 seconds
    if(millis() - timer > 10000) {
      timer = millis();
    }
    else if(millis() - timer > 5000) {
      setAngle(actuators[i], 30);
    }
    else {
      setAngle(actuators[i], 0);
    }
  }

  // increment counter
  counter = (counter + 1) % NUM_PORTS;

  delay(200);
}