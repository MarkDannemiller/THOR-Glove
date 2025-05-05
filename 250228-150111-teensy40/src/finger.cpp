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
    currentAngle = 0;
}

void Finger::begin() {
    // Attach servo to pin
    servo.attach(servoPin, minPulse, maxPulse);
    
    // Set initial position
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

// current logic doesnt use this function
float Finger::calcAngle() {
    // Map voltage (0-3V) to servo angle (0° to upper limit)
    return (fsrVoltage / 2.75) * maxAngle;
}

void Finger::setAngle(float angle) {

    if(!servo.attached()) {
        servo.attach(servoPin, minPulse, maxPulse);
    }

    // Constrain angle to valid range
    if (invert) angle = upperLimit - angle;
    float constrainedAngle = constrain(angle, lowerLimit, upperLimit);
    constrainedAngle = constrain(constrainedAngle, minAngle, maxAngle);
    currentAngle = constrainedAngle;
    
    // Convert to microseconds and write to servo
    servo.writeMicroseconds(angleToMicroseconds(constrainedAngle));

    Serial.println("Servo " + String(servo.attached()) + "(pin " + servoPin + ") angle: " + String(constrainedAngle) + "\t");
}

float Finger::getAngle() {
    return currentAngle;
}

// Detach from servo (used in overcurrent condition)
void Finger::release()
{
    servo.detach();
}

// Sets true angle of the servo, without any reverse logic
void Finger::setTrueAngle(float angle)
{
    if(!servo.attached()) {
        servo.attach(servoPin, minPulse, maxPulse);
    }

    // Constrain angle to valid range, first within param limits and then within device limits
    float constrainedAngle = constrain(angle, lowerLimit, upperLimit);
    constrainedAngle = constrain(constrainedAngle, minAngle, maxAngle);
    currentAngle = constrainedAngle;
    
    // Convert to microseconds and write to servo
    servo.writeMicroseconds(angleToMicroseconds(constrainedAngle));

    Serial.println("Servo " + String(servo.attached()) + "(pin " + servoPin + ") real angle: " + String(constrainedAngle) + "\t");
}

// Returns true angle that the servo has been set to, regardless of reversed value
float Finger::getTrueAngle()
{
    return invert ? maxAngle - currentAngle : currentAngle;
}
