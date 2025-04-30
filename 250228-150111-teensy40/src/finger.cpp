#include "finger.h"

Finger::Finger(int _servoPin, int _fsrPin, int _stretchPin, int _currentPin,
               float _vcc, int _adcResolution,
               int _minAngle, int _maxAngle,
               int _minPulse, int _maxPulse,
               float _relaxedLength,
               float _relaxedResistancePerInch) {
    
    // Assign pins
    servoPin = _servoPin;
    fsrPin = _fsrPin;
    stretchPin = _stretchPin;
    currentPin = _currentPin;
    
    // Set parameters
    vcc = _vcc;
    adcResolution = _adcResolution;
    minAngle = _minAngle;
    maxAngle = _maxAngle;
    minPulse = _minPulse;
    maxPulse = _maxPulse;
    relaxedLength = _relaxedLength;
    relaxedResistancePerInch = _relaxedResistancePerInch;
    
    // Initialize readings
    fsrReading = 0;
    fsrVoltage = 0;
    fsrForce = 0;
    stretchReading = 0;
    stretchVoltage = 0;
    stretchResistance = 0;
    currentLength = relaxedLength;
    currentReading = 0;
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

void Finger::update() {
    updateFSR();
    updateStretch();
    updateCurrent();
}

// TODO Update force math based on real FSR range and testing
void Finger::updateFSR() {
    fsrReading = analogRead(fsrPin);
    fsrVoltage = readVoltage(fsrPin);
    
    // Convert voltage to force in Newtons (approximate calculation)
    // Based on typical FSR behavior - this should be calibrated for accuracy
    // For a 1-100N FSR, we'll use a simplified model
    if (fsrVoltage < 0.1) {
        fsrForce = 0; // No pressure
    } else {
        // Simplified conversion: assumes a roughly linear relationship
        // across a voltage range (typically 0-3V for a 3.3V system)
        // This should be calibrated based on actual sensor characteristics
        fsrForce = (fsrVoltage / vcc) * 100; // Scale 0-VCC to 0-100N
    }
}

void Finger::updateStretch() {
    stretchReading = analogRead(stretchPin);
    stretchVoltage = readVoltage(stretchPin);
    
    // Calculate resistance using voltage divider formula
    // R_stretch = R_known * (Vcc/V_measured - 1)
    // This assumes a circuit where stretch sensor connects to VCC, then to analog pin, 
    // then through a 10K resistor to ground
    float knownResistor = 10000.0; // 10K resistor
    if (stretchVoltage > 0) {
        stretchResistance = knownResistor * (vcc / stretchVoltage - 1);
    } else {
        stretchResistance = 0;
    }
    
    // Estimate length based on resistance
    // Relaxed resistance = relaxedLength * relaxedResistancePerInch
    float relaxedResistance = relaxedLength * relaxedResistancePerInch;
    if (stretchResistance > 0 && relaxedResistance > 0) {
        currentLength = relaxedLength * (stretchResistance / relaxedResistance);
    } else {
        currentLength = relaxedLength;
    }
}

void Finger::updateCurrent() {
    currentReading = analogRead(currentPin);
    float voltage = readVoltage(currentPin);
    
    // Using formula from schematic:
    // A=20, R_sns = 50mOhms, VCC=3.3v
    currentValue = voltage / (20 * 0.05); // I = V/R = (voltage)/(20 * 50mOhms)
}

void Finger::setAngle(float angle) {
    // Constrain angle to valid range
    int constrainedAngle = constrain(angle, minAngle, maxAngle);
    currentAngle = constrainedAngle;
    
    // Convert to microseconds and write to servo
    servo.writeMicroseconds(angleToMicroseconds(constrainedAngle));
}

float Finger::getAngle() {
    return currentAngle;
}

int Finger::getRawAngle() {
    return currentAngle;
}

float Finger::getFSRVoltage() {
    return fsrVoltage;
}

float Finger::getFSRForce() {
    return fsrForce;
}

int Finger::getFSRRaw() {
    return fsrReading;
}

float Finger::getStretchVoltage() {
    return stretchVoltage;
}

float Finger::getStretchResistance() {
    return stretchResistance;
}

float Finger::getStretchLength() {
    return currentLength;
}

int Finger::getStretchRaw() {
    return stretchReading;
}

float Finger::getCurrentValue() {
    return currentValue;
}

int Finger::getCurrentRaw() {
    return currentReading;
} 