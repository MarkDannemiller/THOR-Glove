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
    float minAngle;
    float maxAngle;
    int minPulse;
    int maxPulse;
    float currentAngle;
    bool invert;
    
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
           float _minAngle = 0, float _maxAngle = 180,
           int _minPulse = 500, int _maxPulse = 2500,
           float _lowerLimit = 0, float _upperLimit = 180,
           bool _invert = false);
    
    // Initialization
    void begin();
    
    // Servo control
    float calcAngle();
    void setAngle(float angle);
    float getAngle();
    void release();  // Detach servo to release

    // Servo control (without reversed)
    void setTrueAngle(float angle);
    float getTrueAngle();
    
    // FSR functions
    float getFSRVoltage();
    
    // Current functions
    float getCurrentValue(); // in Amperes

    float upperLimit;
    float lowerLimit;
};

#endif // FINGER_H
