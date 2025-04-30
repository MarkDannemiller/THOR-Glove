#ifndef FINGER_H
#define FINGER_H

#include <Arduino.h>
#include <Servo.h>

class Finger {
  private:
    // Pin assignments
    int servoPin;
    int fsrPin;
    int stretchPin;
    int currentPin;
    
    // Servo parameters
    Servo servo;
    int minAngle;
    int maxAngle;
    int minPulse;
    int maxPulse;
    float currentAngle;
    
    // FSR parameters
    float fsrReading;
    float fsrVoltage;
    float fsrForce;
    float vcc;
    int adcResolution;
    
    // Stretch sensor parameters
    float stretchReading;
    float stretchVoltage;
    float stretchResistance;
    float relaxedLength; // in inches
    float currentLength; // in inches
    float relaxedResistancePerInch; // typically ~350 ohms per inch
    
    // Current sensor parameters
    float currentReading;
    float currentValue; // in Amperes
    
    // Helper functions
    float readVoltage(int pin);
    int angleToMicroseconds(int angle);
    
  public:
    // Constructor
    Finger(int _servoPin, int _fsrPin, int _stretchPin, int _currentPin, 
           float _vcc = 3.3, int _adcResolution = 12, 
           int _minAngle = 0, int _maxAngle = 180,
           int _minPulse = 500, int _maxPulse = 2500,
           float _relaxedLength = 6.0, 
           float _relaxedResistancePerInch = 350.0);
    
    // Initialization
    void begin();
    
    // Update functions
    void update();
    void updateFSR();
    void updateStretch();
    void updateCurrent();
    
    // Servo control
    void setAngle(float angle);
    float getAngle();
    int getRawAngle();
    
    // FSR functions
    float getFSRVoltage();
    float getFSRForce(); // in Newtons
    int getFSRRaw();
    
    // Stretch functions
    float getStretchVoltage();
    float getStretchResistance();
    float getStretchLength(); // estimated length in inches
    int getStretchRaw();
    
    // Current functions
    float getCurrentValue(); // in Amperes
    int getCurrentRaw();
};

#endif // FINGER_H
