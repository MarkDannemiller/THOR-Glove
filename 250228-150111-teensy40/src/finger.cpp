#include "finger.h"

Finger::Finger(int _servoPin, int _fsrPin,
               int _currentPin,
               float _vcc, int _adcResolution,
               int _minAngle, int _maxAngle,
               int _minPulse, int _maxPulse) {
    
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
    
    // Initialize readings
    fsrVoltage = 0;
    currentValue = 0;
    currentAngle = 0;
}

void Finger::begin() {
    // Attach servo to pin
    servo.attach(servoPin, minPulse, maxPulse);
    
    // Set initial position
    setAngle(minAngle);
}

float Finger::readVoltage(int pin) {
    int maxAdcValue = (1 << adcResolution) - 1; // 4095 for 12-bit ADC
    int reading = analogRead(pin);
    return (reading / (float)maxAdcValue) * vcc;
}

int Finger::angleToMicroseconds(int angle) {
    return map(angle, minAngle, maxAngle, minPulse, maxPulse);
}

float Finger::getFSRVoltage() { return fsrVoltage = readVoltage(fsrPin); }

float Finger::getCurrentValue() { 
    // Using formula from schematic:
    // A=20, R_sns = 50mOhms, VCC=3.3v
    return currentValue = readVoltage(currentPin) / (20 * 0.05); // I = V/R = (voltage)/(20 * 50mOhms)
} 

float Finger::calcAngle() {
    // Map voltage (0-3V) to servo angle (0° to upper limit)
    return (fsrVoltage / 2.75) * maxAngle;
}

void Finger::setAngle(float angle) {
    // Constrain angle to valid range
    int constrainedAngle = constrain(angle, minAngle, maxAngle);
    currentAngle = constrainedAngle;
    
    // Convert to microseconds and write to servo
    servo.writeMicroseconds(angleToMicroseconds(constrainedAngle));

    Serial.print("set angle ");
    Serial.print(angle);
}

float Finger::getAngle() {
    return currentAngle;
}