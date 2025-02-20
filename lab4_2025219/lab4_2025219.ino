#include <Arduino.h>
// #include <Servo.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// calibration constants
#define VCC 3.3         // 3.3V Logic
// test/get these values
#define PDR 5100        // pull down resistor
#define OPEN_THRESH 1   // threshold force to open finger
#define CLOSE_THRESH 5  // threshold force to close finger
#define A 1.0           // calibration constant for force
#define B 0.5           // calibration constant for force
#define ANGLE_INC 5     // increment for servo angle

//Constants
#define PCA_NUM_PORTS 16
#define SERVO1_PORT 0
#define SERVO2_PORT 1

uint16_t adc_read;  // reading from ADC pin
float adc_voltage;  // adc_read converted to voltage value
float resistance;   // resistance calcuation
float force;        // Force calculation
int fsrPin;         // targeted ADC input pin
int servoPin;       // targeted servo pin
float servoAngle;   // angle of targeted servo

//Parameters
int MIN_PULSE [PCA_NUM_PORTS] = {500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500};
int MAX_PULSE [PCA_NUM_PORTS] = {2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500};
int MIN_ANG [PCA_NUM_PORTS] =   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int MAX_ANG [PCA_NUM_PORTS] =   {270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270};

int SERVO_UL_DEG = 90; 


//Objects
Adafruit_PWMServoDriver pca= Adafruit_PWMServoDriver(0x40);

double angleToMicroseconds(int angle, int servoPort) {
    return map(angle, MIN_ANG[servoPort], MAX_ANG[servoPort], MIN_PULSE[servoPort], MAX_PULSE[servoPort]);
}


void setup() {
    Serial.begin(115200);
    analogReadResolution(12);  // 12-bit ADC (0-4095)

    fsrPin = A0;

    //Wire.begin();

    pca.begin();
    pca.setPWMFreq(60);  // Analog servos run at ~60 Hz updates

    Serial.println("Zeroing Servos...");
    pca.writeMicroseconds(SERVO1_PORT, angleToMicroseconds(0, SERVO1_PORT));
    pca.writeMicroseconds(SERVO2_PORT, angleToMicroseconds(0, SERVO2_PORT));
    delay(2000);
}

void loop() {
    adc_read = analogRead(fsrPin);
    adc_voltage = (adc_read / 4095.0) * VCC;
    resistance = (VCC - adc_voltage) * PDR / adc_voltage;
    force = pow(resistance, -B);

    Serial.printf("ADC: %u, Voltage: %.2fV, Resistance: %.2fΩ, Force: %.2fN\n",
                  adc_read, adc_voltage, resistance, force);

    // Map voltage (0V to 3V) to a servo angle (0° to upper limit)
    servoAngle = (adc_voltage / 2.75) * SERVO_UL_DEG;
    if (servoAngle < 0) servoAngle = 0;
    if (servoAngle > SERVO_UL_DEG) servoAngle = SERVO_UL_DEG;

    pca.writeMicroseconds(SERVO1_PORT, angleToMicroseconds((int)servoAngle, SERVO1_PORT));
    pca.writeMicroseconds(SERVO2_PORT, angleToMicroseconds((int)(SERVO_UL_DEG-servoAngle), SERVO2_PORT));

    delay(100);
}