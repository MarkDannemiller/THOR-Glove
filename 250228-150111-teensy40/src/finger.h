#ifndef FINGER_H
#define FINGER_H

#include <Arduino.h>
#include <Servo.h>

class Finger {
  private:
    // Pin assignments
    int servoPin;
    int fsrPin;
    int currentPin;
    
    // Servo parameters
    Servo servo;
    int minAngle;
    int maxAngle;
    int minPulse;
    int maxPulse;
    float currentAngle;
    bool invert;
    int upperLimit;
    int lowerLimit;
    
    // FSR parameters
    float fsrVoltage;
    float vcc;
    int adcResolution;

    // Current sensor parameters
    float currentValue; // in Amperes
    
    // Helper functions
    float readVoltage(int pin);
    int angleToMicroseconds(int angle);
    
  public:
    // Constructor
    Finger(int _servoPin, int _fsrPin,
          int _currentPin, 
           float _vcc = 3.3, int _adcResolution = 12, 
           int _minAngle = 0, int _maxAngle = 180,
           int _minPulse = 500, int _maxPulse = 2500,
           int _lowerLimit = 0, int _upperLimit = 180,
           bool _invert = false);
    
    // Initialization
    void begin();
    
    // Servo control
    float calcAngle();
    void setAngle(float angle);
    float getAngle();
    
    // FSR functions
    float getFSRVoltage();
    
    // Current functions
    float getCurrentValue(); // in Amperes
};

#endif // FINGER_H
