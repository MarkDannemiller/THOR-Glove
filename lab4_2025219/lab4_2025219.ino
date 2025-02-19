#include <Arduino.h>
#include <Servo.h>

// calibration constants
#define VCC 3.3         // 3.3V Logic
// test/get these values
#define PDR 5100        // pull down resistor
#define OPEN_THRESH 1   // threshold force to open finger
#define CLOSE_THRESH 5  // threshold force to close finger
#define A 1.0           // calibration constant for force
#define B 0.5           // calibration constant for force
#define ANGLE_INC 5     // increment for servo angle

uint16_t adc_read;  // reading from ADC pin
float adc_voltage;  // adc_read converted to voltage value
float resistance;   // resistance calcuation
float force;        // Force calculation
int fsrPin;         // targeted ADC input pin
int servoPin;       // targeted servo pin
float servoAngle;   // angle of targeted servo

Servo servo;

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);  // 12-bit ADC (0-4095)

    fsrPin = A0;
    servoPin = 0;
}

void loop() {
    // combine after testing
    adc_read = analogRead(fsrPin);
    adc_voltage = (adc_read/4095.0)*VCC;
    resistance = (VCC-adc_voltage)*PDR/adc_voltage;

    // force equation F=A(R^-B)
    force = pow(resistance, -B); 

    Serial.printf("ADC: %u, Voltage: %.2fV, Resistance: %.2fΩ, Force: %.2fN\n",
                  adc_read, adc_voltage, resistance, force);

    // adjust servo angle if force is outside threshold
    servo.attach(servoPin);    // alternate: (pin,min,max)
    servoAngle = servo.read();
    if (force < CLOSE_THRESH) { servo.write(servoAngle+ANGLE_INC); } 
    else if (force > OPEN_THRESH) { servo.write(servoAngle-ANGLE_INC); }
    servo.detach();

    delay(100);
}