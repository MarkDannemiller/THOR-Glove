#include "finger.h"

Finger::Finger(int _servoPin, int _fsrPin,
               int _currentPin,
               float _vcc, int _adcResolution,
               float _minAngle, float _maxAngle,
               int _minPulse, int _maxPulse,
               float _lowerLimit, float _upperLimit,
               bool _invert) {
    
    // Assign pins
    servoPin = _servoPin;
    fsrPin = _fsrPin;
    currentPin = _currentPin;
    
    // Set parameters
    vcc = _vcc;
    adcResolution = _adcResolution;
    minAngle = _minAngle;
    maxAngle = _maxAngle;
    minPulse = _minPulse;
    maxPulse = _maxPulse;
    invert = _invert;
    upperLimit = _upperLimit;
    lowerLimit = _lowerLimit;
    
    // Initialize readings
    fsrVoltage = 0;
    currentValue = 0;
    userAngle = lowerLimit;           // Initialize to relaxed position (15°)
    servoAngle = invert ? (upperLimit - lowerLimit) : lowerLimit;  // Map to raw servo angle
}

void Finger::begin() {
    // Attach servo to pin
    servo.attach(servoPin, minPulse, maxPulse);
    
    // Set initial position in user space
    setAngle(lowerLimit);
}

float Finger::readVoltage(int pin) {
    int maxAdcValue = (1 << adcResolution) - 1; // 4095 for 12-bit ADC
    int reading = analogRead(pin);
    return (reading / (float)maxAdcValue) * vcc;
}

int Finger::angleToMicroseconds(int angle) {
    return map(angle, minAngle, maxAngle, minPulse, maxPulse);
}

//FSR is a 1 ~ 100 Newton Force - Alpha MF01A-N-221-A05
float Finger::getFSRVoltage() { return fsrVoltage = readVoltage(fsrPin); }

float Finger::getCurrentValue() { 
    // Using formula from schematic:
    // A=20, R_sns = 50mOhms, VCC=3.3v
    return currentValue = readVoltage(currentPin) / (20 * 0.05); // I = V/R = (voltage)/(20 * 50mOhms)
} 

float Finger::getCurrentVoltage() {
    return readVoltage(currentPin);
}

// current logic doesnt use this function
float Finger::calcAngle() {
    // Map voltage (0-3V) to servo angle (0° to upper limit)
    return (fsrVoltage / 2.75) * maxAngle;
}

void Finger::setAngle(float angle) {
    if (!servo.attached()) {
        servo.attach(servoPin, minPulse, maxPulse);
    }

    // Clamp user request first
    angle = constrain(angle, lowerLimit, upperLimit);   // 15° to 290° (logical)

    // Map to raw servo space if this finger is physically reversed
    float raw = invert ? (upperLimit - angle) : angle;
    raw = constrain(raw, minAngle, maxAngle);           // 0° to 270° absolute

    userAngle = angle;   // Remember logical position
    servoAngle = raw;    // Remember raw servo position

    servo.writeMicroseconds(angleToMicroseconds((int)raw));
}

float Finger::getAngle() {
    return userAngle;  // Return logical position
}

// Detach from servo (used in overcurrent condition)
void Finger::release() {
    servo.detach();
}

// Sets true angle of the servo, without any reverse logic
void Finger::setTrueAngle(float raw) {
    if (!servo.attached()) {
        servo.attach(servoPin, minPulse, maxPulse);
    }

    // Constrain to valid servo range
    raw = constrain(raw, minAngle, maxAngle);

    servoAngle = raw;
    userAngle = invert ? (upperLimit - raw) : raw;   // Keep logical shadow in sync

    servo.writeMicroseconds(angleToMicroseconds((int)raw));
}

// Returns true angle that the servo has been set to, regardless of reversed value
float Finger::getTrueAngle() {
    return servoAngle;  // Return raw servo position
}
