#include <Arduino.h>
#include <Servo.h>

// calibration constants
#define VCC 3.3         // 3.3V Logic
// test/get these values
#define PDR 5100        // pull down resistor
#define ANGLE_INC 5     // increment for servo angle

//Constants
#define SERVO1_PIN 2
#define SERVO2_PIN 3
#define FSR_PIN A0

uint16_t adc_read;  // reading from ADC pin
float adc_voltage;  // adc_read converted to voltage value
float servoAngle;   // angle of targeted servo

const int MIN_PULSE = 500;
const int MAX_PULSE = 2500;
const int SERVO_MIN_ANG = 0;
const int SERVO_MAX_ANG = 270;
const int SERVO_UL_DEG = 90;

Servo servo1;
Servo servo2;

double angleToMicroseconds(int angle) {
    return map(angle, SERVO_MIN_ANG, SERVO_MAX_ANG, MIN_PULSE, MAX_PULSE);
}


void setup() {
    Serial.begin(115200);
    analogReadResolution(12);  // 12-bit ADC (0-4095)

    servo1.attach(SERVO1_PIN);
    servo2.attach(SERVO2_PIN);


    Serial.println("Zeroing Servos...");
    servo1.writeMicroseconds(angleToMicroseconds(0));
    servo2.writeMicroseconds(angleToMicroseconds(0));
    delay(2000);
}

void loop() {
    adc_read = analogRead(FSR_PIN);
    adc_voltage = (adc_read / 4095.0) * VCC;

    Serial.printf("ADC: %u, Voltage: %.2fV\n",
                  adc_read, adc_voltage);

    // Map voltage (0V to 3V) to a servo angle (0° to upper limit)
    servoAngle = (adc_voltage / 2.75) * SERVO_UL_DEG;
    if (servoAngle < 0) servoAngle = 0;
    if (servoAngle > SERVO_UL_DEG) servoAngle = SERVO_UL_DEG;

    servo1.writeMicroseconds(angleToMicroseconds((int)servoAngle));
    servo2.writeMicroseconds(angleToMicroseconds((int)(SERVO_UL_DEG-servoAngle)));

    delay(100);
}